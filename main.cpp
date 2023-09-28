#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

#include "antlr4-runtime.h"
#include "bdd.h"

#include "MitlLexer.h"
#include "MitlParser.h"

#include "MitlCheckNNFVisitor.h"
#include "MitlToNNFVisitor.h"
#include "MitlAtomNumberingVisitor.h"
#include "MitlCollectTemporalVisitor.h"
#include "MitlGetBDDVisitor.h"

#include "AtomCmp.h"
#include "EnumAtoms.h"

#include "Monitor.h"
#include "Parser.h"


using namespace antlrcpptest;
using namespace antlr4;

int num_all_props;
std::vector<std::string> sat_paths;

std::vector<std::string> get_letters (const std::string& pattern) {

    std::vector<std::string> output;

    std::set<int> dont_cares;

    for (int i = 0; i < pattern.size(); ++i) {
        if (pattern[i] == 'X') {
            dont_cares.insert(i);
        }
    }

    std::string current = pattern;
    std::replace(current.begin(), current.end(), 'X', '0');

    bool incremented = false;

    do {

        output.push_back(current);

        incremented = false;
        for (auto it = dont_cares.rbegin(); !incremented && it != dont_cares.rend(); ++it) {

            if (current[*it] == '0') {

                current[*it] = '1';
                incremented = true;

            } else {

                current[*it] = '0';

            }
        }

    } while (incremented);

    return output;

}

// int edges_increment_count() {
//
//     int i = 0;
//     for (auto p : sat_paths) {
//         for (auto s : get_letters(p)) {
//             ++i;
//         }
//     }
//     return i;
// }

void allsat_print_handler(char* varset, int size) {

    std::string output;

    std::cout << std::setw(12);     // only affects the first char below, actually
    for (int v = 0; v < size; ++v) {
        output += (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
        std::cout << (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
    }
    std::cout << std::endl;
    std::cout << std::setw(0);

    sat_paths.push_back(output);

}

monitaal::TA build_ta(const MitlParser::AtomContext* phi_) {

    std::string name = "TA_" + std::to_string(phi_->id);

    monitaal::clock_map_t clocks;
    clocks.insert({0, "x0"});        // clock 0 is needed anyway

    bdd label;

    if (phi_->type == FINALLY) {

        MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)phi_;

        if (phi->interval() == nullptr) {

            // Finally
            // "untimed" case

            monitaal::constraints_t empty_invariant;
            monitaal::locations_t locations;

            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
            locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

            monitaal::edges_t edges;
            monitaal::constraints_t guard;
            monitaal::clocks_t reset;

            // 0 -> 0, !r && *p && *q

            label = !bdd_ithvar(phi->id) & phi->atom()->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 0 -> 1, r && *p && *q

            label = bdd_ithvar(phi->id) & phi->atom()->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 1, ^p && ~q

            label = bdd_true() & phi->atom()->tilde;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 2 -> 1, ^p && ~q

            label = bdd_true() & phi->atom()->tilde;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 2, r && *p && ^q

            label = bdd_ithvar(phi->id) & phi->atom()->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 2, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 2 -> 2, r && *p && ^q

            label = bdd_ithvar(phi->id) & phi->atom()->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(2, 2, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 0, !r && *p && ^q

            label = !bdd_ithvar(phi->id) & phi->atom()->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 2 -> 0, !r && *p && ^q

            label = !bdd_ithvar(phi->id) & phi->atom()->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(2, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                // Finally
                // [0, u) or [0, u]

                int u;

                if (right->children[0]->getText() == "infty") {
                    assert(false);
                }

                clocks.insert({1, "x1"});


                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, ^p && ~q, x <= a

                label = bdd_true() & phi->atom()->tilde;

                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, ^p && ~q, x <= a

                label = bdd_true() & phi->atom()->tilde;

                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 2, r && *p && ^q, x := 0, x <= a

                label = bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 2, r && *p && ^q, x := 0, x <= a

                label = bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                label = !bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                label = !bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();


                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id




            } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                // Finally
                // [l, infty) or (l, infty)

                int l;


                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));
                locations.push_back(monitaal::location_t(false, 3, "s3", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ^p && ~q

                label = !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde;

                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ^p && *q, x < a

                label = !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && ~q, x := 0

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, !r && ^p && ~q

                label = !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde;

                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, !r && ^p && *q, x < a

                label = !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, r && ^p && ~q, x := 0

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 2, r && *p && ^q, x := 0, x >= a

                label = bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 2, r && *p && ^q, x := 0, x >= a

                label = bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                label = !bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                label = !bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 3, r && ^p && ~q, x := 0

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 3, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 3 -> 3, ^p && ~q

                label = bdd_true() & phi->atom()->tilde;

                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(3, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 3 -> 3, ^p && *q, x < a

                label = bdd_true() & phi->atom()->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(3, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 3 -> 2, ^p && ^q, x := 0, x >= a

                label = bdd_true() & phi->atom()->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(3, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id


            }


        }

    } else if (phi_->type == GLOBALLY) {

        MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)phi_;

        if (phi->interval() == nullptr) {

            // Globally
            // "untimed" case

            monitaal::constraints_t empty_invariant;
            monitaal::locations_t locations;

            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

            monitaal::edges_t edges;
            monitaal::constraints_t guard;
            monitaal::clocks_t reset;


            // 0 -> 0, !r && *p && *q

            label = !bdd_ithvar(phi->id) & phi->atom()->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 0 -> 1, r && *p && *q

            label = bdd_ithvar(phi->id) & phi->atom()->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 1, ~p && ^q

            label = bdd_true() & phi->atom()->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 1, r && ^p && ^q

            label = bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 0, !r && ^p && ^q

            label = !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                // Globally
                // [0, u) or [0, u]

                int u;

                if (right->children[0]->getText() == "infty") {
                    assert(false);
                }

                clocks.insert({1, "x1"});


                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);


                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ~p && ^q, x <= a

                label = !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat;

                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && *p && ^q, x := 0, x <= a

                label = bdd_ithvar(phi->id) & phi->atom()->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && *p && *q, x := 0, x > a

                label = bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                label = !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && *p && *q, x := 0, x > a

                label = !bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();


                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id



            } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                // Globally
                // [l, infty) or (l, infty)


                int l;


                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom()->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ~p && *q, x < a

                label = !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ~p && ^q, x >= a

                label = !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ~p && *q, x < a

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ~p && ^q, x >= a

                label = bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                label = bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && ^p && *q, x := 0, x < a

                label = !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                label = !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();


                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id


            }

        }



    } else if (phi_->type == UNTIL) {

        MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)phi_;

        if (phi->interval() == nullptr) {

            // Until
            // "untimed" case

            monitaal::constraints_t empty_invariant;
            monitaal::locations_t locations;

            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
            locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

            monitaal::edges_t edges;
            monitaal::constraints_t guard;
            monitaal::clocks_t reset;

            // 0 -> 0, !r && *p && *q

            label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 0 -> 1, r && *p && *q

            label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 1, ^p && ~q

            label = phi->atom(0)->hat & phi->atom(1)->tilde;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 2 -> 1, ^p && ~q

            label = phi->atom(0)->hat & phi->atom(1)->tilde;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 2, r && *p && ^q

            label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 2, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 2 -> 2, r && *p && ^q

            label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(2, 2, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 0, !r && *p && ^q

            label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 2 -> 0, !r && *p && ^q

            label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(2, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();


            return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                // Until
                // [0, u) or [0, u]

                int u;

                if (right->children[0]->getText() == "infty") {
                    assert(false);
                }

                clocks.insert({1, "x1"});


                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, ^p && ~q, x <= a

                label = phi->atom(0)->hat & phi->atom(1)->tilde;

                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, ^p && ~q, x <= a

                label = phi->atom(0)->hat & phi->atom(1)->tilde;

                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 2, r && *p && ^q, x := 0, x <= a

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 2, r && *p && ^q, x := 0, x <= a

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();


                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id


            } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                // Until
                // [l, infty) or (l, infty)

                int l;


                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));
                locations.push_back(monitaal::location_t(false, 3, "s3", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ^p && ~q

                label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde;

                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ^p && *q, x < a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && ~q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, !r && ^p && ~q

                label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde;

                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, !r && ^p && *q, x < a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, r && ^p && ~q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 1, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 2, r && *p && ^q, x := 0, x >= a

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 2, r && *p && ^q, x := 0, x >= a

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(2, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 3, r && ^p && ~q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 3, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 3 -> 3, ^p && ~q

                label = phi->atom(0)->hat & phi->atom(1)->tilde;

                l = std::stoi(left->children[0]->getText());

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(3, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 3 -> 3, ^p && *q, x < a

                label = phi->atom(0)->hat & phi->atom(1)->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(3, 3, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 3 -> 2, ^p && ^q, x := 0, x >= a

                label = phi->atom(0)->hat & phi->atom(1)->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(3, 2, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id

            }


        }


    } else if (phi_->type == RELEASE) {

        MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)phi_;

        if (phi->interval() == nullptr) {

            // Release
            // "untimed" case

            monitaal::constraints_t empty_invariant;
            monitaal::locations_t locations;

            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

            monitaal::edges_t edges;
            monitaal::constraints_t guard;
            monitaal::clocks_t reset;

            // 0 -> 0, !r && *p && *q

            label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 0 -> 1, r && *p && *q

            label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 1, ~p && ^q

            label = phi->atom(0)->tilde & phi->atom(1)->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 1, r && ^p && ^q

            label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();

            // 1 -> 0, !r && ^p && ^q

            label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat;
            bdd_allsat(label, *allsat_print_handler);

            for (auto p : sat_paths) {
                for (auto s : get_letters(p)) {
                    edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                }
            }

            // assert(edges_increment_count() == bdd_satcount(label));
            reset.clear();
            guard.clear();
            sat_paths.clear();


            return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                // Release
                // [0, u) or [0, u]

                int u;

                if (right->children[0]->getText() == "infty") {
                    assert(false);
                }

                clocks.insert({1, "x1"});


                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ~p && ^q, x <= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat;

                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && *p && ^q, x := 0, x <= a

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && *p && *q, x := 0, x > a

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && *p && *q, x := 0, x > a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                u = std::stoi(right->children[0]->getText());

                if (right_delim->getSymbol()->getType() == MitlParser::RBrack) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, u));

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, u));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();


                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id



            } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                // Release
                // [l, infty) or (l, infty)

                int l;


                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                monitaal::edges_t edges;
                monitaal::constraints_t guard;
                monitaal::clocks_t reset;

                // 0 -> 0, !r && *p && *q, x := 0

                label = !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 0 -> 1, r && *p && *q, x := 0

                label = bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star;
                reset.push_back(1);
                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ~p && *q, x < a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, !r && ~p && ^q, x >= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ~p && *q, x < a

                label = bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && *q, x := 0, x < a

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ~p && ^q, x >= a

                label = bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat;

                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                label = bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && ^p && *q, x := 0, x < a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::upper_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::upper_non_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();

                // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                label = !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat;
                reset.push_back(1);
                l = std::stoi(left->children[0]->getText());

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack) {

                    guard.push_back(monitaal::constraint_t::lower_non_strict(1, l));

                } else if (left_delim->getSymbol()->getType() == MitlParser::LParen) {

                    guard.push_back(monitaal::constraint_t::lower_strict(1, l));

                } else {
                    assert(false);
                }

                bdd_allsat(label, *allsat_print_handler);

                for (auto p : sat_paths) {
                    for (auto s : get_letters(p)) {
                        edges.push_back(monitaal::edge_t(1, 0, guard, reset, s));     // from, to, guard, reset, label
                    }
                }

                // assert(edges_increment_count() == bdd_satcount(label));
                reset.clear();
                guard.clear();
                sat_paths.clear();


                return monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id

            }

        }

    }

    assert(false);

}

int main(int argc, const char ** argv) {

    const char* spec_file;
    const char* trace_file;

    if (argc >= 2) {

        spec_file = argv[1];

    } else {

        std::cerr << "Usage: demo <in_spec_file>" << std::endl;
        return 1;

    }

    std::ifstream spec_in(spec_file, std::ios::in);
    if (!spec_in) {

        std::cerr << "Error: Could not open in_spec_file" << std::endl;
        return 1;

    }


    std::cout << "\n<<<<<< Pre-processing input formula... >>>>>>\n\n";

    std::cout << "\nInput formula (as read):\n";

    std::stringstream buf;
    buf << spec_in.rdbuf();
    std::cout << buf.str();

    std::string nnf_in = buf.str();

    spec_in.clear();
    spec_in.seekg(0, std::ios::beg);


    ANTLRInputStream input = ANTLRInputStream(spec_in);
    MitlLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    MitlParser parser(&tokens);
    MitlParser::MainContext* original_formula = parser.main();

    MitlCheckNNFVisitor check_nnf_visitor;
    check_nnf_visitor.loadParser(&parser);      // This is for ruleNames[]


    if (!std::any_cast<bool>(check_nnf_visitor.visitMain(original_formula))) {

        std::cout << "\nThe input formula is not in negation normal form (i.e. AND and OR only, and all negations appear just before letters).\n";

        std::cout << "\nRewriting into NNF...\n";

        MitlToNNFVisitor to_nnf_visitor;

        nnf_in = std::any_cast<std::string>(to_nnf_visitor.visitMain(original_formula));


    }

    std::cout << "\nInput formula (in NNF):\n";

    std::cout << nnf_in;

    std::cout << "\nRe-parsing...\n";

    ANTLRInputStream nnf_input = ANTLRInputStream(nnf_in);

    MitlLexer nnf_lexer(&nnf_input);
    CommonTokenStream nnf_tokens(&nnf_lexer);

    MitlParser nnf_parser(&nnf_tokens);

    MitlParser::MainContext* nnf_formula = nnf_parser.main();

    assert(std::any_cast<bool>(check_nnf_visitor.visitMain(nnf_formula)));

    std::cout << "\n<<<<<< Numbering temporal subformulae... >>>>>>\n\n";

    // Numbering temporal atoms

    MitlAtomNumberingVisitor temporal_numbering_visitor;

    num_all_props = std::any_cast<int>(temporal_numbering_visitor.visitMain(nnf_formula));


    std::cout << "\n# of atomic propositions:\n";

    std::cout << nnf_formula->props.size() << std::endl;

    std::cout << "\n# of temporal subformulae:\n";

    std::cout << num_all_props - nnf_formula->props.size() << std::endl;


    std::set<MitlParser::AtomContext*, atom_cmp> temporal_atoms;
    MitlCollectTemporalVisitor collect_temporal_visitor;
    temporal_atoms = std::any_cast<decltype(temporal_atoms)>(collect_temporal_visitor.visitMain(nnf_formula));

    {   // re-numbering

        int i = num_all_props - nnf_formula->props.size();
        for (auto const &[k, v] : nnf_formula->props) {
            nnf_formula->props[k] = ++i;
        }

        i = 0;
        for (auto const& x : temporal_atoms) {

            x->id = ++i;

        }
    }


    std::cout << "\nList of them:\n";

    for (auto const &[k, v] : nnf_formula->props) {
        std::cout << v << ": " << k << std::endl;
    }

    for (auto it = temporal_atoms.rbegin(); it != temporal_atoms.rend(); ++it) {

        std::cout << (*it)->id << ": " << (*it)->getText() << std::endl;

    }

    std::cout << "\nGenerating BDDs for labels...\n";

    bdd_init(1000, 100);
    bdd_setvarnum(num_all_props + 1);   // We leave the 0-th variable unused (temporal atoms and props start from 1)

    MitlGetBDDVisitor get_bdd_visitor;
    get_bdd_visitor.visitMain(nnf_formula);


    for (auto it = temporal_atoms.rbegin(); it != temporal_atoms.rend(); ++it) {

        std::cout << "\n" << (*it)->id << ": " << (*it)->getText() << "\n" << std::endl;

        std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
        bdd_allsat((*it)->overline, *allsat_print_handler);
        std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
        bdd_allsat((*it)->star, *allsat_print_handler);
        std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
        bdd_allsat((*it)->tilde, *allsat_print_handler);
        std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
        bdd_allsat((*it)->hat, *allsat_print_handler);

        if ((*it)->type == FINALLY) {

            std::cout << "\nphi:\n";

            std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomFContext*)(*it))->atom()->overline, *allsat_print_handler);
            std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomFContext*)(*it))->atom()->star, *allsat_print_handler);
            std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomFContext*)(*it))->atom()->tilde, *allsat_print_handler);
            std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomFContext*)(*it))->atom()->hat, *allsat_print_handler);

        } else if ((*it)->type == GLOBALLY) {

            std::cout << "\nphi:\n";

            std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomGContext*)(*it))->atom()->overline, *allsat_print_handler);
            std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomGContext*)(*it))->atom()->star, *allsat_print_handler);
            std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomGContext*)(*it))->atom()->tilde, *allsat_print_handler);
            std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomGContext*)(*it))->atom()->hat, *allsat_print_handler);

        } else if ((*it)->type == UNTIL) {

            std::cout << "\nphi_1:\n";

            std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(0)->overline, *allsat_print_handler);
            std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(0)->star, *allsat_print_handler);
            std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(0)->tilde, *allsat_print_handler);
            std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(0)->hat, *allsat_print_handler);

            std::cout << "\nphi_2:\n";

            std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(1)->overline, *allsat_print_handler);
            std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(1)->star, *allsat_print_handler);
            std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(1)->tilde, *allsat_print_handler);
            std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomUContext*)(*it))->atom(1)->hat, *allsat_print_handler);

        } else if ((*it)->type == RELEASE) {

            std::cout << "\nphi_1:\n";

            std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(0)->overline, *allsat_print_handler);
            std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(0)->star, *allsat_print_handler);
            std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(0)->tilde, *allsat_print_handler);
            std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(0)->hat, *allsat_print_handler);

            std::cout << "\nphi_2:\n";

            std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(1)->overline, *allsat_print_handler);
            std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(1)->star, *allsat_print_handler);
            std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(1)->tilde, *allsat_print_handler);
            std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
            bdd_allsat(((MitlParser::AtomRContext*)(*it))->atom(1)->hat, *allsat_print_handler);

        } else {
            assert(false);
        }

    }

    std::cout << "\n" << 0 << ": " << nnf_formula->formula()->getText() << "\n" << std::endl;

    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
    bdd_allsat(nnf_formula->overline, *allsat_print_handler);
    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
    bdd_allsat(nnf_formula->star, *allsat_print_handler);
    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
    bdd_allsat(nnf_formula->tilde, *allsat_print_handler);
    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
    bdd_allsat(nnf_formula->hat, *allsat_print_handler);

    sat_paths.clear();


    std::cout << "\n<<<<<< Convering into TAs... >>>>>>\n\n";

    auto div = monitaal::TA::time_divergence_ta(get_letters(std::string(num_all_props + 1, 'X')), true);

    std::vector<monitaal::TA> components;

    for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

        std::cout << "\nGenerating TA_" << (*it)->id << "...\n";
        components.push_back(build_ta(*it));

    }

    std::cout << "\nGenerating TA_0" << "...\n";

    std::string name = "TA_0";

    monitaal::clock_map_t clocks;
    clocks.insert({0, "x0"});        // clock 0 is needed anyway

    bdd label;

    monitaal::constraints_t empty_invariant;
    monitaal::locations_t locations;

    locations.push_back(monitaal::location_t(false, 0, "s0", empty_invariant));
    locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

    monitaal::edges_t edges;
    monitaal::constraints_t guard;
    monitaal::clocks_t reset;

    // 0 -> 1, varphi

    label = nnf_formula->hat;
    bdd_allsat(label, *allsat_print_handler);

    for (auto p : sat_paths) {
        for (auto s : get_letters(p)) {
            edges.push_back(monitaal::edge_t(0, 1, guard, reset, s));     // from, to, guard, reset, label
        }
    }

    reset.clear();
    guard.clear();
    sat_paths.clear();

    // 1 -> 1, *varphi

    label = nnf_formula->star;
    bdd_allsat(label, *allsat_print_handler);

    for (auto p : sat_paths) {
        for (auto s : get_letters(p)) {
            edges.push_back(monitaal::edge_t(1, 1, guard, reset, s));     // from, to, guard, reset, label
        }
    }

    reset.clear();
    guard.clear();
    sat_paths.clear();

    auto varphi = monitaal::TA(name, clocks, locations, edges, 0);   // last arg: initial location id

    bdd_done();

    std::cout << "\n<<<<<< Taking intersection... >>>>>>\n\n";

    varphi.intersection(div);

    for (auto c : components) {

        std::cout << "one component done" << std::endl;
        varphi.intersection(c);

    }


    std::cout << varphi;


    return 0;

}
