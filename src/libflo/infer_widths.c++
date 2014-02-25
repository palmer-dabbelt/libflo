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

#include "infer_widths.h++"
#include <map>
#include <queue>
using namespace libflo;

/* Here I define a whole bunch of short names for the hugely long
 * templated types that keep showing up in C++.  They're all probably
 * specific to this algorithm, so I'm just leaving them in this
 * file. */
typedef std::vector<node_ptr> known_vector;
typedef std::vector<node_ptr> unknown_vector;
typedef std::queue<node_ptr> unknown_queue;
typedef std::pair<std::string, std::string> string_pair;
typedef std::multimap<std::string, std::string> string_mmap;
typedef std::pair<std::string, node_ptr> known_pair;
typedef std::map<std::string, node_ptr> known_map;
typedef std::pair<std::string, node_ptr> unknown_pair;
typedef std::map<std::string, node_ptr> unknown_map;

/* These determine if the destination width of an operation is known,
 * and if it is known determine what it is. */
static bool know_d_width(const node_ptr o);
static unsigned get_d_width(const node_ptr o);

/* These determine if the source width of an operation is known
 * implicitly as the result of just the opcode. */
static bool know_s_width(const node_ptr o, int i);
static unsigned get_s_width(const node_ptr o, int i);

/* Returns TRUE if the two operand indicies (d is 0, s starts at 1)
 * need to match. */
static bool need_o_match(const node_ptr o, int i, int j);

/* Upgrades an operation to the version with a known width. */
static node_ptr upgrade(node_ptr u);

/* This is used for the final pass: once all the node widths are known
 * we need to re-map them to produce actual operation widths.  For
 * example, the first phase will always produce "gte/1" because the
 * output of gte is always a boolean: this instead converts gte to the
 * width of its inputs. */
static node_ptr remap(node_ptr o, const known_map &map);

const node_list libflo::infer_widths(const node_list &ops_in)
{
    node_list out;
    unknown_queue ops;

    /* This allows us to look up all the consumers of a given node by
     * name.  These are all listed by name because we'll have to look
     * up widths in two places: the list of known widths and the list
     * of unknown widths. */
    string_mmap consumers;

    /* Here we map the name of a node to the operation that writes to
     * that node. */
    unknown_map unknown_dest;

    /* This provides a simple way to look up the width of some nodes
     * that have known widths. */
    known_map known;

    /* Here we build the required data structures... */
    for (auto it = ops_in.nodes(); !it.done(); ++it) {
        /* Every done must be pushed eventually. */
        ops.push(*it);

        /* Allows for lookups of the unknown operation using a node's
         * name. */
        unknown_dest.insert(unknown_pair((*it)->d(), *it));

        /* Builds a quick lookup so we can figure out all the
         * consumers of a given operation. */
        for (auto sit = (*it)->s_begin(); sit != (*it)->s_end(); ++sit)
            consumers.insert(string_pair(*sit, (*it)->d()));
    }

    /* Eventually the width of every item should be inferred -- note
     * that some of them may be pushed back on the queue if they're
     * unable to be reasoned about, so this can't just be an
     * iteration. */
    size_t skipped_count = 0;
    while (ops.empty() == false) {
        bool did_work = false;

        /* Here's the termination condition: we've skipped the entire
         * queue without actually doing anything.  That means that the
         * algorithm has reached a steady state and is unable to
         * finish. */
        if (skipped_count > (ops.size() * 2)) {
            fprintf(stderr, "Unable to infer the widths of some nodes\n");
            while (ops.empty() == false) {
                auto o = ops.front(); ops.pop();
                fprintf(stderr, "  node '%s' remains\n", o->d().c_str());
            }
            fprintf(stderr, "Aborting due uninferrable widths\n");
            abort();
        }

        auto o = ops.front(); ops.pop();

        /* The simplest option is that we end up with some sort of
         * operation in which the width is known directly from the
         * input file. */
        if (know_d_width(o) == true) {
            /* Here we manually specify a width and create a new node
             * (of the known width type) that cooresponds to the old
             * node. */
            auto wid = o->with_width(get_d_width(o));
            auto kw = upgrade(wid);

            /* Add this to the list of already-known data. */
            if (known.find(kw->d()) == known.end()) {
                out.add(kw);
                known.insert(known_pair(kw->d(), kw));
                did_work = true;
            }

#ifdef DEBUG_WIDTH_INFERENCE
            fprintf(stderr, "known D width: '%s' of %u\n",
                    wid->d().c_str(), wid->width());
#endif
        }

        /* If we somehow already managed to infer the width of this
         * node (via coorelation, as in below) then we can throw it
         * away after performing the necessary checks. */
        if (known.find(o->d()) != known.end())
            did_work = true;

        /* Checks if we automatically know any of the source operand
         * widths.  Some operations have implicit widths for some
         * sources (MUX, for example). */
        for (size_t i = 0; i < o->s().size(); ++i) {
            /* Check if the source width can be staticly determined
             * from the operation that's being performed. */
            if (know_s_width(o, i) == true) {
                auto sname = o->s(i);
                auto ucopl = unknown_dest.find(sname);

                /* It's possible that we can't find an operation that
                 * creates this node -- this is actually OK beacuse
                 * this just means that we discovered an integer
                 * constant. */
                if (ucopl == unknown_dest.end())
                    continue;

                auto ucop = ucopl->second;

                /* Check if the width has already been set by
                 * something else.  If it hasn't then we can go ahead
                 * and store it here because we know it (and in an
                 * extreme case, this might be the only case in which
                 * we can be sure we know the width). */
                auto kcopl = known.find(sname);
                if (kcopl == known.end()) {
                    auto wid = ucop->with_width(get_s_width(o, i));
                    auto kw = upgrade(wid);

                    /* Add this to the list of already-known data. */
                    out.add(kw);
                    known.insert(known_pair(kw->d(), kw));
                    did_work = true;

                    kcopl = known.find(sname);
                }

                /* This can't happen because we either found the node
                 * or just stored it. */
                if (kcopl == known.end()) {
                    fprintf(stderr, "op wasn't stored\n");
                    abort();
                }

                /* We need to ensure that the width we just inferred
                 * matches the width that was previously inferred --
                 * otherwise there is some sort of bug somewhere... */
                auto kcop = kcopl->second;
                if (know_s_width(o, i)) {
                    if (kcop->width() != get_s_width(o, i)) {
                        fprintf(stderr, "Mismatched width\n");
                        fprintf(stderr, "'%s' had inferred width of %d\n",
                                kcop->d().c_str(), kcop->width());
                        fprintf(stderr, "'%s' requires width of %d\n",
                                o->d().c_str(), get_s_width(o, i));
                        abort();
                    }
                }

#ifdef DEBUG_WIDTH_INFERENCE
                fprintf(stderr, "known S width: '%s' of %u in ",
                        kcop->d().c_str(), kcop->width());
                o->writeln(stderr);
#endif
            }
        }

        /* Try and do some cross-coorelation between the different
         * operands that this operation knows about. */
        for (size_t i = 0; i <= o->s().size(); ++i) {
            for (size_t j = 0; j <= o->s().size(); ++j) {
                if (i == j)
                    continue;

                /* There's really nothing that can be done if this
                 * operand pair doesn't match. */
                if (need_o_match(o, i, j) == false)
                    continue;

                auto in = o->o(i);
                auto jn = o->o(j);

                auto kil = known.find(in);
                auto kjl = known.find(jn);

                if (kil == known.end() && kjl == known.end()) {
                    /* Neither of the sources has a known width, which
                     * means there's nothing left to do on this pass
                     * -- try again later. */
                } else if (kil == known.end() || kjl == known.end()) {
                    /* One of the operations has a known width, figure
                     * out that width... */
                    auto kl = (kil == known.end()) ? kjl : kil;
                    auto k  = kl->second;

                    /* Obtain a pointer to the other operation. */
                    auto un = (kil == known.end()) ? in : jn;
                    auto ul = unknown_dest.find(un);
                    if (ul == unknown_dest.end()) {
                        /* This is OK, it just means that we found a
                         * constant.  Since constants can have any
                         * width then there's nothing to try and
                         * map. */
                        continue;
                    }
                    auto u  = ul->second;

                    /* Stick that other operation in the big list of
                     * known operations. */
                    auto wid = u->with_width(k->width());
                    auto kw = upgrade(wid);
                    out.add(kw);
                    known.insert(known_pair(kw->d(), kw));
                    did_work = true;

#ifdef DEBUG_WIDTH_INFERENCE
                    fprintf(stderr,
                            "inferred O width: '%s' of %u (matches '%s') in ",
                            o->o(i).c_str(), kw->width(),
                            o->o(j).c_str());
                    o->writeln(stderr);
#endif
                } else {
                }
            }
        }

        if (did_work == false) {
            /* If we've managed to make it to here then that means
             * that we couldn't do any work at this step.  Add it back
             * to the queue for processing later (where hopefully
             * we'll know more), but make sure to keep track so we
             * always eventually terminate. */
            ops.push(o);
            skipped_count++;
        }
    }

    if (ops_in.size() != out.size()) {
        fprintf(stderr, "infer_width(): input and output sizes differ\n");
        abort();
    }

    node_list remapped;
    for (auto it = out.nodes(); !it.done(); ++it)
        remapped.add(remap(*it, known));

    return remapped;
}

bool know_d_width(const node_ptr o)
{
    switch (o->opcode()) {
        /* These are all boolean-valued operations, so the output
         * width is always known exactly. */
    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    case opcode::RST:
        return true;

        /* These operations produce their output width if it was
         * exactly specified anywhere. */
    case opcode::ADD:
    case opcode::AND:
    case opcode::IN:
    case opcode::MOV:
    case opcode::MUX:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::REG:
    case opcode::RSH:
    case opcode::SUB:
    case opcode::XOR:
        return o->op().has_width();

        /* I haven't figured out what these ones do yet... */
    case opcode::EAT:
    case opcode::RND:
    case opcode::LIT:
    case opcode::CAT:
    case opcode::MSK:
    case opcode::LD:
    case opcode::NEQ:
    case opcode::ARSH:
    case opcode::LSH:
    case opcode::ST:
    case opcode::MEM:
    case opcode::NOP:
    case opcode::MUL:
    case opcode::LOG2:
    case opcode::NEG:
    case opcode::RD:
    case opcode::WR:
        fprintf(stderr, "Opcode '%s' not handled in know_d_width()\n",
                opcode_to_string(o->opcode()).c_str());
        abort();
        break;
    }

    fprintf(stderr, "Opcode '%s' not handled in know_d_width()\n",
            opcode_to_string(o->opcode()).c_str());
    abort();
    return false;
}

unsigned get_d_width(const node_ptr o)
{
    switch (o->opcode()) {
    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    case opcode::RST:
        return 1;

    case opcode::ADD:
    case opcode::AND:
    case opcode::IN:
    case opcode::MOV:
    case opcode::MUX:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::REG:
    case opcode::RSH:
    case opcode::SUB:
    case opcode::XOR:
        return o->op().width();

    case opcode::EAT:
    case opcode::RND:
    case opcode::LIT:
    case opcode::CAT:
    case opcode::MSK:
    case opcode::LD:
    case opcode::NEQ:
    case opcode::ARSH:
    case opcode::LSH:
    case opcode::ST:
    case opcode::MEM:
    case opcode::NOP:
    case opcode::MUL:
    case opcode::LOG2:
    case opcode::NEG:
    case opcode::RD:
    case opcode::WR:
        fprintf(stderr, "Opcode '%s' not handled in get_d_width()\n",
                opcode_to_string(o->opcode()).c_str());
        abort();
        break;
    }

    fprintf(stderr, "Opcode '%s' not handled in get_d_width()\n",
            opcode_to_string(o->opcode()).c_str());
    abort();
    return -1;
}

bool know_s_width(const node_ptr o, int i)
{
    switch (o->opcode()) {
        /* The first input to a MUX is a special case: it's always a
         * boolean. */
    case opcode::MUX:
        if (i == 0) return true;

        /* These operations have only a single bit width throughout
         * their entire operation.  Thus, if we know the output width
         * then we implicitly know the input widths. */
    case opcode::ADD:
    case opcode::AND:
    case opcode::IN:
    case opcode::MOV:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::REG:
    case opcode::SUB:
    case opcode::XOR:
        return o->op().has_width();

        /* These operations have a single bit width through all their
         * sources, but their destination doesn't match. */
    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    case opcode::RST:
        return o->op().has_width();

        /* For this operation, nothing at all is known about the width
         * of the sources! */
    case opcode::RSH:
        return false;

    case opcode::EAT:
    case opcode::RND:
    case opcode::LIT:
    case opcode::CAT:
    case opcode::MSK:
    case opcode::LD:
    case opcode::NEQ:
    case opcode::ARSH:
    case opcode::LSH:
    case opcode::ST:
    case opcode::MEM:
    case opcode::NOP:
    case opcode::MUL:
    case opcode::LOG2:
    case opcode::NEG:
    case opcode::RD:
    case opcode::WR:
        fprintf(stderr, "Opcode '%s' not handled in know_s_width()\n",
                opcode_to_string(o->opcode()).c_str());
        abort();
        break;
    }

    fprintf(stderr, "Opcode '%s' not handled in know_s_width()\n",
            opcode_to_string(o->opcode()).c_str());
    abort();
    return false;
}

unsigned get_s_width(const node_ptr o, int i)
{
    switch (o->opcode()) {
    case opcode::MUX:
        if (i == 0) return 1;

    case opcode::ADD:
    case opcode::AND:
    case opcode::IN:
    case opcode::MOV:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::REG:
    case opcode::SUB:
    case opcode::XOR:
        return o->op().width();

    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    case opcode::RST:
        return o->op().width();

    case opcode::RSH:
        fprintf(stderr, "The bit-width of a source of this type unknowable\n");
        abort();
        return -1;

    case opcode::EAT:
    case opcode::RND:
    case opcode::LIT:
    case opcode::CAT:
    case opcode::MSK:
    case opcode::LD:
    case opcode::NEQ:
    case opcode::ARSH:
    case opcode::LSH:
    case opcode::ST:
    case opcode::MEM:
    case opcode::NOP:
    case opcode::MUL:
    case opcode::LOG2:
    case opcode::NEG:
    case opcode::RD:
    case opcode::WR:
        fprintf(stderr, "Opcode '%s' not handled in get_s_width()\n",
                opcode_to_string(o->opcode()).c_str());
        abort();
        break;
    }

    fprintf(stderr, "Opcode '%s' not handled in get_s_width()\n",
            opcode_to_string(o->opcode()).c_str());
    abort();
    return -1;
}

bool need_o_match(const node_ptr o, int i, int j)
{
    switch (o->opcode()) {
    case opcode::MUX:
        /* Index 1 (the first source) is boolean, all others must
         * match. */
        return (i != 1) && (j != 1);

        /* All indicies must match. */
    case opcode::ADD:
    case opcode::AND:
    case opcode::IN:
    case opcode::MOV:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::REG:
    case opcode::SUB:
    case opcode::XOR:
        return true;

        /* The destination is boolean, but all other operations must
         * match.  Note that we don't have to worry about the case
         * where "i == j" because this isn't called that way. */
    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    case opcode::RST:
        return (i >= 1) && (j >= 1);

        /* FIXME: Is it true that the widths don't have to match for
         * shifts?  It certainly feels that way... */
    case opcode::RSH:
        return false;

    case opcode::EAT:
    case opcode::RND:
    case opcode::LIT:
    case opcode::CAT:
    case opcode::MSK:
    case opcode::LD:
    case opcode::NEQ:
    case opcode::ARSH:
    case opcode::LSH:
    case opcode::ST:
    case opcode::MEM:
    case opcode::NOP:
    case opcode::MUL:
    case opcode::LOG2:
    case opcode::NEG:
    case opcode::RD:
    case opcode::WR:
        fprintf(stderr, "Opcode '%s' not handled in need_o_match()\n",
                opcode_to_string(o->opcode()).c_str());
        abort();
        break;
    }

    fprintf(stderr, "Unknown match needed\n");
    abort();
    return false;
}

node_ptr upgrade(node_ptr u)
{
    return u;
}

node_ptr remap(node_ptr o, const known_map &map)
{
    switch (o->opcode()) {
        /* These operations match the input width fine and don't need
         * modification. */
    case opcode::ADD:
    case opcode::AND:
    case opcode::IN:
    case opcode::MOV:
    case opcode::MUX:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::REG:
    case opcode::RSH:
    case opcode::RST:
    case opcode::SUB:
    case opcode::XOR:
        return o;

        /* The destination is boolean, but the operation still has a
         * width.  This means we need to look up a source (all are the
         * same). */
    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    {
        auto l = map.find(o->s(0));
        if (l == map.end()) {
            fprintf(stderr, "Attempted to remap unmapped node '%s'\n",
                    o->s(0).c_str());
            abort();
        }
        return o->with_width(l->second->width());
    }

    case opcode::EAT:
    case opcode::RND:
    case opcode::LIT:
    case opcode::CAT:
    case opcode::MSK:
    case opcode::LD:
    case opcode::NEQ:
    case opcode::ARSH:
    case opcode::LSH:
    case opcode::ST:
    case opcode::MEM:
    case opcode::NOP:
    case opcode::MUL:
    case opcode::LOG2:
    case opcode::NEG:
    case opcode::RD:
    case opcode::WR:
        fprintf(stderr, "Unknown opcode '%s' in remap()\n",
                opcode_to_string(o->opcode()).c_str());
        abort();
        break;
    }

    fprintf(stderr, "unhandled switch case\n");
    abort();
}
