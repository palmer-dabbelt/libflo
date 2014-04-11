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

#ifndef LIBFLO__OPERATION_HXX
#define LIBFLO__OPERATION_HXX

#include "sizet_printf.h++"
#include "unknown.h++"
#include <map>
#include <math.h>
#include <memory>
#include <string.h>
#include <vector>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

namespace libflo {
    /* Operations represent the computation of a node -- this means
     * they take as their input the values of some nodes and produce
     * as an output the values of other nodes. */
    template<class node_t> class operation {
    public:
        class node_iter {
        private:
            const typename std::vector<std::shared_ptr<node_t>> _nodes;
            typename std::vector<std::shared_ptr<node_t>>::const_iterator _it;
        public:
            node_iter(const std::vector<std::shared_ptr<node_t>>& nodes)
                : _nodes(nodes),
                  _it(_nodes.begin())
                {
                }
            std::shared_ptr<node_t> operator*(void) const { return *_it; }
            bool done(void) const { return _it == _nodes.end(); }
            void operator++(void) { ++_it; }
        };

    private:
        std::shared_ptr<node_t> _d;
        unknown<size_t> _width;
        const opcode _op;
        const std::vector<std::shared_ptr<node_t>> _s;

    protected:
        operation(std::shared_ptr<node_t>& dest,
                  const unknown<size_t>& width,
                  const opcode& op,
                  const std::vector<std::shared_ptr<node_t>>& s)
            : _d(dest),
              _width(width),
              _op(op),
              _s(s)
            {
                /* FIXME: Check that an array with the correct number
                 * of sources has been provided.  The fact that I'm
                 * not doing this can cause SEGVs, which is
                 * yucky... */
                switch (op) {
                case opcode::ADD:
                case opcode::AND:
                case opcode::ARSH:
                case opcode::CAT:
                case opcode::CATD:
                case opcode::EAT:
                case opcode::EQ:
                case opcode::GTE:
                case opcode::IN:
                case opcode::INIT:
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
                case opcode::RSHD:
                case opcode::RST:
                case opcode::ST:
                case opcode::SUB:
                case opcode::WR:
                case opcode::XOR:
                    break;
                }

                /* Here's a sanity check: ensure that the sources
                 * aren't the same as the destination. */
                for (const auto& s : _s) {
                    if (strcmp(s->name().c_str(), _d->name().c_str()) == 0) {
                        fprintf(stderr, "Trivial dest <= source loop: ");
                        writeln_debug(stderr);
                        abort();
                    }
                }

                /* Zero-width operations aren't valid. */
                if (_width.known() && (_width.value() == 0)) {
                    fprintf(stderr, "Created 0-width operation: ");
                    writeln_debug(stderr);
                    abort();
                }
            }

    public:
        /* Accessor functions. */
        const unknown<size_t>& width_u(void) const { return _width; }
        size_t width(void) const { return _width.value(); }
        bool known_width(void) const { return _width.known(); }

        const unknown<size_t>& cycle_u(void) const { return _d->cycle_u(); }
        size_t cycle(void) const { return _d->cycle(); }
        bool known_cycle(void) const { return _d->known_cycle(); }

        opcode op(void) const { return _op; }

        const std::vector<std::shared_ptr<node_t>> svec(void) const
            { return _s; }

        /* Allows access to the destination, the source array, or the
         * operand array -- this just contains two indexing schemes
         * for the nodes this operation can see. */
        const std::shared_ptr<node_t> d(void) const { return _d; }
        const std::shared_ptr<node_t> s(size_t i) const { return _s[i]; }
        const std::shared_ptr<node_t> o(size_t i) const
            { return (i == 0) ? d() : s(i-1); }
        const std::shared_ptr<node_t> s(void) const { return s(0); }
        const std::shared_ptr<node_t> t(void) const { return s(1); }
        const std::shared_ptr<node_t> u(void) const { return s(2); }
        const std::shared_ptr<node_t> v(void) const { return s(3); }

        /* Iterates through all the operands (both source and
         * destination) of this node. */
        node_iter operands(void) const
            {
                std::vector<std::shared_ptr<node_t>> o;
                o.push_back(_d);
                for (auto it = _s.begin(); it != _s.end(); ++it)
                    o.push_back(*it);
                return node_iter(o);
            }

        /* Iterates through all the sources. */
        node_iter sources(void) const
            {
                return node_iter(_s);
            }

        /* Attempts to perform local width inference.  In other words,
         * infers the width of this node by only looking at the nodes
         * directly used by this operation. */
        void try_infer_width(void)
            {
                switch (_op) {
                    /* 0-source operations. */
                case opcode::IN:
                case opcode::RND:
                case opcode::RST:
                    must_match(std::vector<size_t>({0}));
                    if (o(0)->known_width())
                        _width = o(0)->width();
                    break;

                    /* 1-source ALU operations (sources match dest) */
                case opcode::LIT:
                case opcode::NEG:
                case opcode::MOV:
                case opcode::NOT:
                case opcode::OUT:
                    must_match(std::vector<size_t>({0, 1}));
                    if (o(0)->known_width())
                        _width = o(0)->width();
                    break;

                    /* 2-source ALU operations (sources match dest) */
                case opcode::ADD:
                case opcode::AND:
                case opcode::OR:
                case opcode::REG:
                case opcode::SUB:
                case opcode::XOR:
                    must_match(std::vector<size_t>({0, 1, 2}));
                    if (o(0)->known_width())
                        _width = o(0)->width();
                    break;

                    /* Comparison operations -- note that here we will
                     * have already set the destination width because
                     * it's always staticly known. */
                case opcode::EQ:
                case opcode::GTE:
                case opcode::LT:
                case opcode::NEQ:
                    must_match(std::vector<size_t>({1, 2}));
                    if (o(1)->known_width())
                        _width = o(1)->width();
                    break;

                    /* For MUXes, everything must match excpet for the first
                     * source, which is always a boolean. */
                case opcode::MUX:
                    must_match(std::vector<size_t>({0, 2, 3}));
                    must_be(1, 1);
                    if (o(0)->known_width())
                        _width = o(0)->width();
                    break;

                    /* For multiplication the output width is twice
                     * the input width.  We've encoded this with a bit
                     * of a trick... */
                case opcode::MUL:
                    must_sum(0, std::vector<size_t>({1, 2}));
                    if (o(0)->known_width())
                        _width = o(0)->width();
                    break;

                    /* CAT is a special case: the output widtht is the
                     * sum of the input widths. */
                case opcode::CAT:
                case opcode::CATD:
                    must_sum(0, std::vector<size_t>({1, 2}));
                    if (o(2)->known_width())
                        _width = o(2)->width();

                    break;

                    /* Memory operations are pretty much all special
                     * cases. */
                case opcode::RD:
                    /* As far as I can tell, the RD instruction looks
                     * like the following: "VALUE = rd/WIDTH 1 MEMORY
                     * ADDRESS". */
                    must_be(1, 1);
                    break;

                case opcode::WR:
                    /* As far as I can tell, the WR instruction looks
                     * like the following: "UNUSED = wr/WIDTH WEN
                     * MEMORY ADDR DATA". */
                    must_match(std::vector<size_t>({0, 4}));
                    break;

                case opcode::LD:
                case opcode::ST:
                    fprintf(stderr, "FIXME: infer widths for memory ops\n");
                    writeln_debug(stderr);
                    abort();
                    break;

                    /* FIXME: I don't think there's anything we can do
                     * about shift operations because of the
                     * mask/extened that happens.  Note that we _do_
                     * know the output width, so I think it's somewhat
                     * fair... */
                case opcode::LOG2:
                    break;

                    /* LSH's width is defined to never drop any bits
                     * off the end.  Unfortunately this is a pretty
                     * wacky special case, as there appears to be a
                     * whole bunch of width inference rules that
                     * Chisel attempts to use... */
                case opcode::LSH:
                {
                    if (s()->known_width() && t()->is_const()) {
                        /* If the shift amount is a constant then we
                         * know exactly how much to shift.*/
                        _width = s()->width() + t()->const_int();
                    }
                    else if (s()->known_width() && t()->known_width()) {
                        /* If we just know the width of both sides
                         * then generate a pessimsticly-large shift. */
                        _width = s()->width() + (1 << t()->width()) - 1;
                    }

                    if (_width.known())
                        must_be(0, _width.value());

                    break;
                }

                case opcode::ARSH:
                case opcode::RSH:
                case opcode::RSHD:
                {
                    /* Here a special hack for RSH: essentially the
                     * idea is that if the offset is a constant then
                     * it doesn't really matter what the width is (as
                     * long as it can fit the value without
                     * truncation) because nothing else is going to
                     * see it. */
                    if (t()->is_const() && !t()->known_width()) {
                        if (t()->const_int() == 0)
                            t()->update_width(1);
                        else
                            t()->update_width(ceil(log2(t()->const_int())) + 1);
                    }

                    break;
                }

                case opcode::MSK:
                    break;

                    /* Essentially here we just enforce that the
                     * widths of the constants inside an init node
                     * match the memory's width.  Note that this isn't
                     * technically correct (as the address width is
                     * not the same as the data width, it's
                     * "log2(depth)"), but it's good enough for now...
                     * Note that nobody should notice because
                     * constants are almost always passed raw and only
                     * constans can be in init nodes. */
                case opcode::INIT:
                    must_match(std::vector<size_t>({1, 2, 3}));
                    break;

                    /* These operations just don't do anything, so it
                     * doesn't make any sense to infer widths at
                     * all. */
                case opcode::EAT:
                case opcode::MEM:
                case opcode::NOP:
                    break;
                }
            }

        /* Attempts to perform local scheduling -- note that this is
         * just a simple schedule, it produces the first cycle at
         * which this operation could possibly run if dataflow order
         * is observed.. */
        void try_schedule(void)
            {
                switch (_op) {
                    /* These operations are just always ready to
                     * issue. */
                case opcode::IN:
                case opcode::REG:
                case opcode::RND:
                case opcode::RST:
                    _d->update_cycle(0);
                    break;

                    /* Most operations can issue after all sources
                     * have issued, they've just got a different
                     * number of sources. */
                case opcode::LIT:
                case opcode::LOG2:
                case opcode::NEG:
                case opcode::MOV:
                case opcode::NOT:
                case opcode::OUT:
                    after(std::vector<size_t>({1}));
                    break;

                case opcode::ADD:
                case opcode::AND:
                case opcode::OR:
                case opcode::SUB:
                case opcode::XOR:
                case opcode::EQ:
                case opcode::GTE:
                case opcode::LT:
                case opcode::NEQ:
                case opcode::MUL:
                case opcode::CAT:
                case opcode::CATD:
                case opcode::ARSH:
                case opcode::LSH:
                case opcode::MSK:
                case opcode::RSH:
                case opcode::RSHD:
                    after(std::vector<size_t>({1, 2}));
                    break;

                case opcode::MUX:
                    after(std::vector<size_t>({1, 2, 3}));
                    break;

                    /* Memory operations are pretty much all special
                     * cases... */
                case opcode::WR:
                    after(std::vector<size_t>({1, 2, 3, 4}));
                    break;

                case opcode::RD:
                    after(std::vector<size_t>({1, 2, 3}));
                    break;

                    /* FIXME: I have no idea what memory operations
                     * should do... */
                case opcode::LD:
                case opcode::ST:
                    fprintf(stderr, "FIXME: schedule memory ops\n");
                    writeln_debug(stderr);
                    abort();
                    break;

                    /* These operations just don't do anything, so it
                     * doesn't make any sense to infer widths at
                     * all. */
                case opcode::EAT:
                case opcode::MEM:
                case opcode::NOP:
                case opcode::INIT:
                    break;
                }
            }

        /* Writes this operation out to standard out as a string. */
        void writeln(FILE *f) const
            {
                if (_op != opcode::INIT)
                    fprintf(f, "%s = ", _d->name().c_str());

                fprintf(f, "%s",
                        opcode_to_string(_op).c_str());

                if (_width.known() && _op != opcode::INIT)
                    fprintf(f, "'" SIZET_FORMAT, _width.value());
                
                for (auto it = _s.begin(); it != _s.end(); ++it) {
                    fprintf(f, " %s", (*it)->name().c_str());
                    if ((*it)->known_width() && (*it)->is_const())
                        fprintf(f, "'" SIZET_FORMAT, (*it)->width());
                }
                
                fprintf(f, "\n");
            }

        /* Writes a more verbose form on this operation out to a file.
         * This may not be parseable by libflo, but it should be
         * readable by a human and look a lot like a Flo file. */
        void writeln_debug(FILE *f) const
            {
                fprintf(f, "%s",
                        _d->name().c_str());
                if (_d->known_width())
                    fprintf(f, "'" SIZET_FORMAT, _d->width());

                fprintf(f, " = %s",
                        opcode_to_string(_op).c_str());
                if (_width.known())
                    fprintf(f, "'" SIZET_FORMAT, _width.value());

                for (auto it = _s.begin(); it != _s.end(); ++it) {
                    fprintf(f, " %s", (*it)->name().c_str());
                    if ((*it)->known_cycle())
                        fprintf(f, "@" SIZET_FORMAT, (*it)->cycle());
                    if ((*it)->known_width())
                        fprintf(f, "'" SIZET_FORMAT, (*it)->width());
                }

                fprintf(f, "\n");
            }

        /* Produces a string that contains the contents of this
         * operation. */
        const std::string to_string(void) const
            {
                char opstr[LINE_MAX];
                if (_width.known()) {
                    snprintf(opstr, LINE_MAX, "%s'" SIZET_FORMAT,
                             opcode_to_string(_op).c_str(),
                             _width.value()
                        );
                } else {
                    snprintf(opstr, LINE_MAX, "%s",
                             opcode_to_string(_op).c_str()
                        );
                }

                char buffer[LINE_MAX];
                switch (_s.size()) {
                case 0:
                    snprintf(buffer, LINE_MAX, "%s = %s",
                             d()->name().c_str(),
                             opstr
                        );
                    break;

                case 1:
                    snprintf(buffer, LINE_MAX, "%s = %s %s",
                             d()->name().c_str(),
                             opstr,
                             s()->name().c_str()
                        );
                    break;

                case 2:
                    snprintf(buffer, LINE_MAX, "%s = %s %s %s",
                             d()->name().c_str(),
                             opstr,
                             s()->name().c_str(),
                             t()->name().c_str()
                        );
                    break;

                case 3:
                    snprintf(buffer, LINE_MAX, "%s = %s %s %s %s",
                             d()->name().c_str(),
                             opstr,
                             s()->name().c_str(),
                             t()->name().c_str(),
                             u()->name().c_str()
                        );
                    break;

                case 4:
                    snprintf(buffer, LINE_MAX, "%s = %s %s %s %s %s",
                             d()->name().c_str(),
                             opstr,
                             s()->name().c_str(),
                             t()->name().c_str(),
                             u()->name().c_str(),
                             v()->name().c_str()
                        );
                    break;

                default:
                    fprintf(stderr, "Wrong number of sources\n");
                    abort();
                }
                return buffer;
            }

    private:
        /* Here's some helper functions for width inference.  They
         * shouldn't be useful to anyone else. */
        void must_match(const std::vector<size_t>& o)
            {
                /* Attempt to find a single known width operand so we
                 * can infer the widths of everything else.  Check to
                 * make sure any widths actually match while doing
                 * this. */
                std::shared_ptr<node_t> known = NULL;
                for (auto it = o.begin(); it != o.end(); ++it) {
                    auto node = this->o(*it);
                    if (node->known_width()) {
                        if (known != NULL) {
                            if (known->width() != node->width()) {
                                fprintf(stderr, "Mismatched widths\n");
                                fprintf(stderr, "  %s: " SIZET_FORMAT "\n",
                                        known->name().c_str(),
                                        known->width());
                                fprintf(stderr, "  %s: " SIZET_FORMAT "\n",
                                        node->name().c_str(),
                                        node->width());
                                writeln_debug(stderr);
                                abort();
                            }
                        }

                        known = node;
                    }
                }

                if (known == NULL)
                    return;

                /* Now that we know one of the widths, infer the width
                 * of every other node in this operation. */
                for (auto it = o.begin(); it != o.end(); ++it) {
                    auto node = this->o(*it);
                    node->update_width(known->width());

#ifdef DEBUG_WIDTH_INFERENCE
                    fprintf(stderr, "infer %s: ", node->name().c_str());
                    writeln_debug(stderr);
#endif
                }
            }

        void must_sum(size_t o, const std::vector<size_t>& i)
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

                /* If there were too many unknows, then there's
                 * nothing we can do to infer the widths right now. */
                if (unknows > 1)
                    return;

                /* If everything is known, then just make sure the sum
                 * matches. */
                if (unknows == 0 && i_sum != o_sum) {
                    fprintf(stderr, "Mismatches width sum\n");
                    writeln_debug(stderr);
                    abort();
                }

                /* If the one unknown width is the LHS then just set it. */
                if (this->o(o)->known_width() == false)
                    this->o(o)->update_width(unknown<size_t>(i_sum));

                /* Find the one unknown width on the RHS and update that. */
                for (auto it = i.begin(); it != i.end(); ++it)
                    if (this->o(*it)->known_width() == false)
                        this->o(*it)->update_width(o_sum - i_sum);
            }

        void must_be(size_t o, size_t w)
            {
                this->o(o)->update_width(unknown<size_t>(w));
            }

        /* Used for scheduling. */
        void after(const std::vector<size_t>& o)
            {
                size_t min_cycle = 0;
                for (auto it = o.begin(); it != o.end(); ++it) {
                    if (this->o(*it)->known_cycle() == false)
                        return;

                    if (this->o(*it)->cycle() > min_cycle)
                        min_cycle = this->o(*it)->cycle();
                }
                _d->update_cycle(min_cycle + 1);
            }

    public:
        /* Sorts operations based on their clock cycle. */
        static bool cmp_sched(const std::shared_ptr<operation<node_t>> a,
                              const std::shared_ptr<operation<node_t>> b)
            {
                return a->cycle() < b->cycle();
            }

        /* Parses an operation, looking up the sources and
         * destinations by string in the provided map. */
        template<class operation_t>
        static std::shared_ptr<operation_t>
        parse(const std::map<std::string, std::shared_ptr<node_t>>& n,
              const std::string d,
              const opcode& op,
              const unknown<size_t>& width,
              const std::vector<std::string>& s)
            {
                /* Here we look up the actual node data for everything
                 * this operation touches. */
                auto dl = n.find(d);
                if (dl == n.end()) {
                    fprintf(stderr, "Tried to read operation without dest\n");
                    abort();
                }
                auto dp = dl->second;

                std::vector<std::shared_ptr<node_t>> sp;
                for (auto it = s.begin(); it != s.end(); ++it) {
                    /* Try and find this source in the list of nodes
                     * that we know about. */
                    auto s = node::lookup(n, *it);
                    if (s == NULL) {
                        fprintf(stderr, "Unable to parse node: '%s'\n",
                                (*it).c_str());
                        abort();
                    }
                    sp.push_back(s);
                }

                /* Now that we've got pointer to all the nodes we're
                 * pretty much done -- just construct the operation
                 * structure that cooresponds to it.  Note that some
                 * operations don't actually have any sor of operation
                 * directly attached to them. */
                switch (op) {
                case opcode::ADD:
                case opcode::AND:
                case opcode::ARSH:
                case opcode::CAT:
                case opcode::CATD:
                case opcode::EQ:
                case opcode::GTE:
                case opcode::IN:
                case opcode::INIT:
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
                case opcode::RSHD:
                case opcode::RST:
                case opcode::ST:
                case opcode::SUB:
                case opcode::WR:
                case opcode::XOR:
                    return std::shared_ptr<operation_t>(new operation_t(dp,
                                                                        width,
                                                                        op,
                                                                        sp)
                        );
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

        /* This is really just a constructor, but I want to avoid
         * anyone from passing around a raw pointer. */
        static std::shared_ptr<operation<node_t>>
        create(std::shared_ptr<node_t>& dest,
                  const unknown<size_t>& width,
                  const opcode& op,
                  const std::vector<std::shared_ptr<node_t>>& s)
            {
                auto ptr = new operation<node_t>(dest,
                                                 width,
                                                 op,
                                                 s);
                return std::shared_ptr<operation<node_t>>(ptr);
            }
    };
}

#endif
