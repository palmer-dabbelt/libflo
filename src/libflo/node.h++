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

#ifndef LIBFLO__NODE_HXX
#define LIBFLO__NODE_HXX

#include "opcode.h++"
#include "unknown.h++"
#include "sizet_printf.h++"
#include <map>
#include <memory>
#include <string>
#include <string.h>
#include <vector>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

namespace libflo {
    /* Everything in the dataflow machine that can have a value is
     * considered a node. */
    class node {
    private:
        const std::string _name;
        unknown<size_t> _width;
        unknown<size_t> _depth;
        const bool _is_mem;
        const bool _is_const;
        unknown<size_t> _dfdepth;
        unknown<size_t> x, y;

    protected:
        node(const std::string name,
             const unknown<size_t>& width,
             const unknown<size_t>& depth,
             bool is_mem,
             bool is_const,
             unknown<size_t> dfdepth);

    public:
        /* Accessor functions. */
        const std::string& name(void) const { return _name; }

        size_t width(void) const { return _width.value(); }
        bool known_width(void) const { return _width.known(); }

        size_t depth(void) const { return _depth.value(); }
        bool known_depth(void) const { return _depth.known(); }

        bool is_mem(void) const { return _is_mem; }
        bool is_reg(void) const { return !_is_mem; }
        bool is_const(void) const { return _is_const; }

        size_t dfdepth(void) const { return _dfdepth.value(); }
        bool known_dfdepth(void) const { return _dfdepth.known(); }

        /* These access the unknown-type values, which are useful if
         * you want to copy them somewhere else. */
        const unknown<size_t>& width_u(void) const { return _width; }
        const unknown<size_t>& depth_u(void) const { return _depth; }
        const unknown<size_t>& dfdepth_u(void) const { return _dfdepth; }

        /* Updates a node with a new width -- this will fail if both
         * widths are known and they don't match. */
        void update_width(const unknown<size_t>& width);

        /* Updates a node with a new dfdepth, failing on mismatch. */
        void update_dfdepth(const unknown<size_t>& dfdepth);

        /* Returns the value of this constant, parsed as an integer. */
        int const_int(void) const;

    public:
        /* Parses an input string, which contains the combined
         * name+width (an optional width) for a node.  str2name
         * returns the node's name, while str2width returns the width
         * (or unknown). */
        static std::string str2name(const std::string n);
        static unknown<size_t> str2width(const std::string,
                                         const unknown<size_t>& w);
        static unknown<size_t> str2width(const std::string);

        /* Looks up a node in a node map, potientally mangling its
         * name.  If the node can be built as a constant then then
         * constant will be returned, otherwise NULL will be. */
        template <class node_t>
        static std::shared_ptr<node_t>
        lookup(const std::map<std::string, std::shared_ptr<node_t> >& map,
               const std::string str)
            {
                auto l = map.find(str);
                if (l != map.end())
                    return l->second;

                /* If there was no node with this name then we
                 * must have ended up with a constant --
                 * verify that and add one to the list. */
                char buffer[LINE_MAX];
                snprintf(buffer, LINE_MAX, "%s", str2name(str).c_str());
                for (size_t i = 0; i < strlen(buffer); ++i) {
                    if (i == 0 && buffer[i] == '-')
                        continue;

                    if (buffer[i] == '\'' || buffer[i] == '/')
                        break;

                    if (!isdigit(buffer[i])) {
                        return NULL;
                    }
                }

                return node::constant<node_t>(str);
            }

        /* Parses a node to determine exactly what sort of node it
         * is.  You probably don't want to bother with this unless
         * you're inside the parser... */
        template <class node_t>
        static std::shared_ptr<node_t>
        parse(const std::string dw,
              const opcode& op_in,
              const unknown<size_t>& width,
              const std::vector<std::string>& s,
              int array_width, int array_height, bool dims_valid)
            {
                std::string d = dw;
                unknown<size_t> x, y;

                /* If this node has been placed in continuous
                 * coodinates then we need to convert those to proper
                 * coordinates by multiplying by the array size. */
                float dx, dy;
                char buf[LINE_MAX];
                if (sscanf(dw.c_str(), "%[^$]$%f,%f", buf, &dx, &dy) == 3) {
                    if (dims_valid == false) {
                        fprintf(stderr, "Requested dims but invalid\n");
                        abort();
                    }

                    d = buf;
                    x = ((dx + 1) / 2) * array_width;
                    y = ((dy + 1) / 2) * array_height;
                }

                /* FIXME: Jonathan's placement tool outputs NOPs
                 * sometimes.  It appears these always come from IN
                 * nodes, but I'm not really sure about that... */
                opcode op = op_in;
                if (op == opcode::NOP)
                    op = opcode::IN;

                switch (op) {
                    /* These operations produce a fixed output width
                     * and are always availiable -- essentially theh
                     * easiest one. */
                case opcode::RST:
                    return reg<node_t>(d,
                                       unknown<size_t>(1),
                                       unknown<size_t>(0)
                        );

                    /* These operations simply produce a single bit as
                     * an output regardless of their input widths. */
                case opcode::EQ:
                case opcode::GTE:
                case opcode::LT:
                case opcode::NEQ:
                    return reg<node_t>(d,
                                       unknown<size_t>(1),
                                       unknown<size_t>());

                    /* These operations produce a variable output
                     * width but are always ready. */
                case opcode::IN:
                case opcode::REG:
                    return reg<node_t>(d,
                                       width,
                                       unknown<size_t>(0));

                    /* These are "normal" nodes, which means their
                     * output width is specified directly by the
                     * operation. */
                case opcode::ADD:
                case opcode::AND:
                case opcode::ARSH:
                case opcode::LD:
                case opcode::LIT:
                case opcode::LOG2:
                case opcode::LSH:
                case opcode::MOV:
                case opcode::MSK:
                case opcode::MUL:
                case opcode::MUX:
                case opcode::NEG:
                case opcode::NOT:
                case opcode::OR:
                case opcode::OUT:
                case opcode::RD:
                case opcode::RND:
                case opcode::RSH:
                case opcode::RSHD:
                case opcode::ST:
                case opcode::SUB:
                case opcode::WR:
                case opcode::XOR:
                    return reg<node_t>(d,
                                       width,
                                       unknown<size_t>());
                    break;

                    /* Some operations don't have their output width
                     * specified by the Flo file but instead must have
                     * this width somehow inferred. */
                case opcode::CAT:
                case opcode::CATD:
                    return reg<node_t>(d,
                                       unknown<size_t>(),
                                       unknown<size_t>());

                    /* Memories are special: they have a depth
                     * paramater as well as a width parameter.  It's
                     * possible to specify the depth explicitly, but
                     * there's also some notion of being able to
                     * automatically detect the width. */
                case opcode::MEM:
                {
                    if (s.size() == 0)
                        return mem<node_t>(d, width, unknown<size_t>());

                    long long depth = atoll(s[0].c_str());
                    if (depth == -1)
                        return mem<node_t>(d, width, unknown<size_t>());

                    size_t sd = (size_t)depth;
                    auto m = mem<node_t>(d, width, sd);
                    m->update_dfdepth(0);
                    return m;

                    break;
                }

                case opcode::INIT:
                    return reg<node_t>(d,
                                       unknown<size_t>(1),
                                       unknown<size_t>(0));

                /* These operations don't actually produce a node. */
                case opcode::EAT:
                case opcode::NOP:
                    return NULL;
                    break;
                }

                fprintf(stderr, "Unhandled switch case\n");
                abort();
            }

        /* Generates a new constant-valued node. */
        template<class node_t>
        static std::shared_ptr<node_t> constant(const std::string name)
            {
                auto value = str2name(name);
                auto width = str2width(name);
                auto posval = str2pos(value, width);

                return std::shared_ptr<node_t>(new node_t(posval,
                                                          width,
                                                          unknown<size_t>(0),
                                                          false,
                                                          true,
                                                          unknown<size_t>(0)
                                                   ));
            }


        template<class node_t>
        static std::shared_ptr<node_t> reg(const std::string name,
                                           const unknown<size_t>& width,
                                           const unknown<size_t>& dfdepth)
            {
                return std::shared_ptr<node_t>(new node_t(name,
                                                          width,
                                                          unknown<size_t>(0),
                                                          false,
                                                          false,
                                                          dfdepth
                                                   ));
            }

        template<class node_t>
        static std::shared_ptr<node_t> mem(const std::string name,
                                           const unknown<size_t>& width,
                                           const unknown<size_t>& depth)
            {
                return std::shared_ptr<node_t>(new node_t(name,
                                                          width,
                                                          depth,
                                                          true,
                                                          false,
                                                          unknown<size_t>()
                                                   ));
            }

    private:
        static std::string str2pos(const std::string& value,
                                   const unknown<size_t>& width)
            {
                if (!width.known())
                    return value;
                if (value.c_str()[0] != '-')
                    return value;

                if (width.value() > 63) {
                    fprintf(stderr, "Unable to parse large negative\n");
                    fprintf(stderr, "  value: '%s'\n",
                            value.c_str());
                    fprintf(stderr, "  width: " SIZET_FORMAT "\n",
                            width.value());
                    abort();
                }

                int64_t ll = atoll(value.c_str());
                uint64_t ull = ll;
                ull &= ((1 << width.value()) - 1);
                return std::to_string(ull);
            }
    };
}

#endif
