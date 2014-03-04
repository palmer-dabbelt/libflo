/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer.dabbelt@eecs.berkeley.edu>
 *
 * This file is part of DREAMER.
 *
 * DREAMER is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * DREAMER is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with DREAMER.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "opcode.h++"
#include <stdio.h>
#include <string.h>
using namespace libflo;

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/* Searches for a short string at the start of a long string. */
static bool str_starts(const std::string haystack, const std::string needle);

enum opcode libflo::string_to_opcode(const std::string op)
{
    if (str_starts(op, "eat")) {
        return opcode::EAT;
    } else if (str_starts(op, "rnd")) {
        return opcode::RND;
    } else if (str_starts(op, "mux")) {
        return opcode::MUX;
    } else if (str_starts(op, "sub")) {
        return opcode::SUB;
    } else if (str_starts(op, "and")) {
        return opcode::AND;
    } else if (str_starts(op, "lt")) {
        return opcode::LT;
    } else if (str_starts(op, "not")) {
        return opcode::NOT;
    } else if (str_starts(op, "reg")) {
        return opcode::REG;
    } else if (str_starts(op, "or")) {
        return opcode::OR;
    } else if (str_starts(op, "eq")) {
        return opcode::EQ;
    } else if (str_starts(op, "add")) {
        return opcode::ADD;
    } else if (str_starts(op, "rst")) {
        return opcode::RST;
    } else if (str_starts(op, "in")) {
        return opcode::IN;
    } else if (str_starts(op, "lit")) {
        return opcode::LIT;
    } else if (str_starts(op, "cat")) {
        return opcode::CAT;
    } else if (str_starts(op, "rsh")) {
        return opcode::RSH;
    } else if (str_starts(op, "msk")) {
        return opcode::MSK;
    } else if (str_starts(op, "ld")) {
        return opcode::LD;
    } else if (str_starts(op, "neq")) {
        return opcode::NEQ;
    } else if (str_starts(op, "arsh")) {
        return opcode::ARSH;
    } else if (str_starts(op, "lsh")) {
        return opcode::LSH;
    } else if (str_starts(op, "xor")) {
        return opcode::XOR;
    } else if (str_starts(op, "st")) {
        return opcode::XOR;
    } else if (str_starts(op, "mem")) {
        return opcode::MEM;
    } else if (str_starts(op, "out")) {
        return opcode::OUT;
    } else if (str_starts(op, "gte")) {
        return opcode::GTE;
    } else if (str_starts(op, "mov")) {
        return opcode::MOV;
    } else if (str_starts(op, "nop")) {
        return opcode::NOP;
    } else if (str_starts(op, "mul")) {
        return opcode::MUL;
    } else if (str_starts(op, "log2")) {
        return opcode::LOG2;
    } else if (str_starts(op, "neg")) {
        return opcode::NEG;
    } else if (str_starts(op, "rd")) {
        return opcode::RD;
    } else if (str_starts(op, "wr")) {
        return opcode::WR;
    } else {
        fprintf(stderr, "Unable to parse operation '%s'\n", op.c_str());
        abort();
    }
}

const std::string&& libflo::opcode_to_string(enum opcode op)
{
    switch (op) {
    case opcode::EAT:  return std::move(std::string("eat"));
    case opcode::RND:  return std::move(std::string("rnd"));
    case opcode::MUX:  return std::move(std::string("mux"));
    case opcode::SUB:  return std::move(std::string("sub"));
    case opcode::AND:  return std::move(std::string("and"));
    case opcode::LT:   return std::move(std::string("lt"));
    case opcode::NOT:  return std::move(std::string("not"));
    case opcode::REG:  return std::move(std::string("reg"));
    case opcode::OR:   return std::move(std::string("or"));
    case opcode::EQ:   return std::move(std::string("eq"));
    case opcode::ADD:  return std::move(std::string("add"));
    case opcode::RST:  return std::move(std::string("rst"));
    case opcode::IN:   return std::move(std::string("in"));
    case opcode::LIT:  return std::move(std::string("lit"));
    case opcode::CAT:  return std::move(std::string("cat"));
    case opcode::RSH:  return std::move(std::string("rsh"));
    case opcode::MSK:  return std::move(std::string("msk"));
    case opcode::LD:   return std::move(std::string("ld"));
    case opcode::NEQ:  return std::move(std::string("neq"));
    case opcode::ARSH: return std::move(std::string("arsh"));
    case opcode::LSH:  return std::move(std::string("lsh"));
    case opcode::XOR:  return std::move(std::string("xor"));
    case opcode::ST:   return std::move(std::string("st"));
    case opcode::MEM:  return std::move(std::string("mem"));
    case opcode::OUT:  return std::move(std::string("out"));
    case opcode::GTE:  return std::move(std::string("gte"));
    case opcode::MOV:  return std::move(std::string("mov"));
    case opcode::NOP:  return std::move(std::string("nop"));
    case opcode::MUL:  return std::move(std::string("mul"));
    case opcode::LOG2: return std::move(std::string("log2"));
    case opcode::NEG:  return std::move(std::string("neg"));
    case opcode::RD:   return std::move(std::string("rd"));
    case opcode::WR:   return std::move(std::string("wr"));
    }

    fprintf(stderr, "Reached the end of a switch...\n");
    abort();
}

const unknown<size_t> libflo::opstring_to_width(const std::string string)
{
    char buffer[LINE_MAX];
    snprintf(buffer, LINE_MAX, "%s", string.c_str());

    char *num = strstr(buffer, "/");
    if (num == NULL)
        return unknown<size_t>();
    num++;

    for (size_t i = 0; i < strlen(num); ++i)
        if (!isdigit(num[i]))
            return unknown<size_t>();

    return unknown<size_t>(atoi(num));
}

bool str_starts(const std::string haystack, const std::string needle)
{
    return (strncmp(haystack.c_str(),
                    needle.c_str(),
                    strlen(needle.c_str())) == 0);
}
