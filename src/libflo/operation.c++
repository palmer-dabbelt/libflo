/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer.dabbelt@eecs.berkeley.edu>
 *
 * This file is part of libflo.
 *
 * libflo is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libflo is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libflo.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "operation.h++"
#include <string.h>
using namespace libflo;

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

operation::operation(node_ptr& dest,
                     const unknown<size_t>& width,
                     const opcode& op,
                     const std::vector<node_ptr>& s)
    : _d(dest),
      _width(width),
      _op(op),
      _s(s)
{
    /* FIXME: Check that an array with the correct number of sources
     * has been provided. */
    switch (op) {
    case opcode::ADD:
    case opcode::AND:
    case opcode::ARSH:
    case opcode::CAT:
    case opcode::EAT:
    case opcode::EQ:
    case opcode::GTE:
    case opcode::IN:
    case opcode::LD:
    case opcode::LIT:
    case opcode::LOG2:
    case opcode::LSH:
    case opcode::LT:
    case opcode::MEM:
    case opcode::MOV:
    case opcode::MSK:
    case opcode::MUL:
    case opcode::MUX:
    case opcode::NEG:
    case opcode::NEQ:
    case opcode::NOP:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::RD:
    case opcode::REG:
    case opcode::RND:
    case opcode::RSH:
    case opcode::RST:
    case opcode::ST:
    case opcode::SUB:
    case opcode::WR:
    case opcode::XOR:
        break;
    }
}

void operation::try_infer_width(void)
{
    switch (_op) {
        /* 0-source operations. */
    case opcode::RND:
    case opcode::RST:
        must_match(std::vector<size_t>({0}));
        _width = o(0)->width();
        break;

        /* 1-source ALU operations (sources match dest) */
    case opcode::IN:
    case opcode::LIT:
    case opcode::NEG:
    case opcode::MOV:
    case opcode::NOT:
    case opcode::OUT:
        must_match(std::vector<size_t>({0, 1}));
        _width = o(0)->width();
        break;

        /* 2-source ALU operations (sources match dest) */
    case opcode::ADD:
    case opcode::AND:
    case opcode::OR:
    case opcode::REG:
    case opcode::RSH:
    case opcode::SUB:
    case opcode::XOR:
        must_match(std::vector<size_t>({0, 1, 2}));
        _width = o(0)->width();
        break;

        /* Comparison operations -- note that here we will have
         * already set the destination width because it's always
         * staticly known. */
    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    case opcode::NEQ:
        must_match(std::vector<size_t>({1, 2}));
        _width = o(1)->width();
        break;

    case opcode::ARSH:
    case opcode::CAT:
    case opcode::LOG2:
    case opcode::LSH:
    case opcode::MSK:
        break;

        /* For MUXes, everything must match excpet for the first
         * source, which is always a boolean. */
    case opcode::MUX:
        must_match(std::vector<size_t>({0, 2, 3}));
        must_be(1, 1);
        _width = o(0)->width();
        break;

    case opcode::LD:
    case opcode::RD:
    case opcode::ST:
    case opcode::WR:
        break;

    case opcode::MUL:
        must_match(std::vector<size_t>({1, 2}));
        must_sum(0, std::vector<size_t>({1, 2}));
        break;

        /* These operations just don't do anything, so it doesn't make
         * any sense to infer widths at all. */
    case opcode::EAT:
    case opcode::MEM:
    case opcode::NOP:
        break;
    }
}

void operation::writeln(FILE *f) const
{
    fprintf(f, "%s = %s", _d->name().c_str(), opcode_to_string(_op).c_str());

    if (_width.known())
        fprintf(f, "/%lu", _width.value());

    for (auto it = _s.begin(); it != _s.end(); ++it)
        fprintf(f, " %s", (*it)->name().c_str());

    fprintf(f, "\n");
    
}

void operation::must_match(const std::vector<size_t>& o)
{
    /* Attempt to find a single known width operand so we can infer
     * the widths of everything else.  Check to make sure any widths
     * actually match while doing this. */
    node_ptr known = NULL;
    for (auto it = o.begin(); it != o.end(); ++it) {
        auto node = this->o(*it);
        if (node->known_width()) {
            if (known != NULL) {
                if (known->width() != node->width()) {
                    fprintf(stderr, "Mismatched widths\n");
                    abort();
                }
            }

            known = node;
        }
    }

    if (known == NULL)
        return;

    /* Now that we know one of the widths, infer the width of every
     * other node in this operation. */
    for (auto it = o.begin(); it != o.end(); ++it) {
        auto node = this->o(*it);
        node->update_width(known->width());
    }
    
}

void operation::must_sum(size_t o, const std::vector<size_t>& i)
{
    size_t unknows = 0;

    /* Compute the LHS side of the sum. */
    size_t o_sum = 0;
    if (this->o(o)->known_width() == false)
        unknows++;
    else
        o_sum += this->o(o)->width();

    /* Comupute the RHS side of the sum. */
    size_t i_sum = 0;
    for (auto it = i.begin(); it != i.end(); ++it) {
        if (this->o(*it)->known_width() == false)
            unknows++;
        else
            i_sum += this->o(*it)->width();
    }

    /* If there were too many unknows, then there's nothing we can do
     * to infer the widths right now. */
    if (unknows > 1)
        return;

    /* If everything is known, then just make sure the sum matches. */
    if (unknows == 0 && i_sum != o_sum) {
        fprintf(stderr, "Mismatches width sum\n");
        abort();
    }

    /* If the one unknown width is the LHS then just set it. */
    if (this->o(o)->known_width() == false)
        this->o(o)->update_width(unknown<size_t>(i_sum));

    /* Find the one unknown width on the RHS and update that. */
    for (auto it = i.begin(); it != i.end(); ++it)
        if (this->o(*it)->known_width() == false)
            this->o(*it)->update_width(unknown<size_t>(o_sum - i_sum));
    
}

void operation::must_be(size_t o, size_t w)
{
    this->o(o)->update_width(unknown<size_t>(w));
}

operation_ptr operation::parse(const std::map<std::string, node_ptr>& n,
                               const std::string d,
                               const opcode& op,
                               const unknown<size_t>& width,
                               const std::vector<std::string>& s)
{
    /* Here we look up the actual node data for everything this
     * operation touches. */
    auto dl = n.find(d);
    if (dl == n.end()) {
        fprintf(stderr, "Tried to read operation without dest\n");
        abort();
    }
    auto dp = dl->second;

    std::vector<node_ptr> sp;
    for (auto it = s.begin(); it != s.end(); ++it) {
        /* Try and find this source in the list of nodes that we know
         * about. */
        auto sl = n.find(*it);
        if (sl == n.end()) {
            /* If there was no node with this name then we must have
             * ended up with a constant -- verify that and add one to
             * the list. */
            char buffer[LINE_MAX];
            snprintf(buffer, LINE_MAX, "%s", (*it).c_str());
            for (size_t i = 0; i < strlen(buffer); ++i) {
                if (i == 0 && buffer[i] == '-')
                    continue;
                if (!isdigit(buffer[i])) {
                    fprintf(stderr, "Found non-numeric constant\n");
                    abort();
                }
            }

            auto val = atoll(buffer);
            sp.push_back(node::constant(val));
        } else {
            /* The lookup didn't fail, so just directly add that node
             * to the list of sources. */
            sp.push_back(sl->second);
        }
    }

    /* Now that we've got pointer to all the nodes we're pretty much
     * done -- just construct the operation structure that cooresponds
     * to it.  Note that some operations don't actually have any sor
     * of operation directly attached to them. */
    switch (op) {
    case opcode::ADD:
    case opcode::AND:
    case opcode::ARSH:
    case opcode::CAT:
    case opcode::EQ:
    case opcode::GTE:
    case opcode::IN:
    case opcode::LD:
    case opcode::LIT:
    case opcode::LOG2:
    case opcode::LSH:
    case opcode::LT:
    case opcode::MOV:
    case opcode::MSK:
    case opcode::MUL:
    case opcode::MUX:
    case opcode::NEG:
    case opcode::NEQ:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::RD:
    case opcode::REG:
    case opcode::RND:
    case opcode::RSH:
    case opcode::RST:
    case opcode::ST:
    case opcode::SUB:
    case opcode::WR:
    case opcode::XOR:
        return operation_ptr(new operation(dp, width, op, sp));
        break;

    case opcode::EAT:
    case opcode::MEM:
    case opcode::NOP:
        return NULL;
        break;
    }

    fprintf(stderr, "Unhandled switch case\n");
    abort();
}
