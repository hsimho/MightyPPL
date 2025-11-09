#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelihn(const MitlParser::AtomContext* phi_) {

        std::stringstream out_str;

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)phi_;

        std::string name;

        if (phi->interval() == nullptr) {

                assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                /***** Pnueli Hn
                 [0, u) or [0, u]
                *****/

                if (right->children[0]->getText() == "infty") {
                    assert(("Please remove superflous interval [0, infty)", false));
                }

                clocks.insert({1, "x1"});

                std::vector<monitaal::TAwithBDDEdges> components;

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                std::map<std::string, monitaal::location_id_t> name_id_map;

                monitaal::bdd_edges_t bdd_edges;

                bdd in_null = encode(0, phi->id, phi->bits / 2 - 1);
                bdd out_null = encode(0, phi->id + phi->bits / 2, phi->bits / 2 - 1);
                bdd in_i;
                bdd out_i;
                bdd in_next_i;
                bdd out_next_i;
                bdd in_prev_i;
                bdd out_prev_i;


                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    name = "TA_" + std::to_string(phi->id) + "_" + std::to_string(i);
                    in_i = encode(i + 1, phi->id, phi->bits / 2 - 1);
                    out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 1);

                    // The current version of MoniTAal only allows 1 initial location

                    if (i == 0) {
                        locations.push_back(monitaal::location_t(false, 1 + phi->atoms.size(), "s0i", empty_invariant));
                        name_id_map.insert({"0i", 1 + phi->atoms.size()});
                    }
                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    name_id_map.insert({"0", 0});
                    for (auto j = 0; j < phi->atoms.size(); ++j) {
                        locations.push_back(monitaal::location_t(true, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                        name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                    }

                    // assert(!acc_map.count(name));
                    // {
                    //     std::set<size_t> s;
                    //     s.insert(0);
                    //     for (int i = 0; i < phi->atoms.size(); ++i) {
                    //         s.insert(s.end(), 1 + i);
                    //     }
                    //     acc_map.insert({ name, { s, s } });
                    // }

                    if (out_format.has_value() && !out_flatten) {

                        if (out_format.value()) {

                            out_str << std::endl << std::endl;
                            out_str << "# " << name << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                            out_str << "# " << const_cast<MitlParser::AtomHnContext*>(phi)->getText() << std::endl;
                            out_str << "process:" << name << std::endl;

                            if (i == 0) {
                                out_str << "location:" << name << ":ell_" << name_id_map.at("0i") << "{initial: }" << std::endl;
                            }
                            out_str << "location:" << name << ":ell_" << name_id_map.at("0") << "{" << (i == 0 ? "" : "initial: : ") << "labels: accept_" << phi->id << "_" << i << "}" << std::endl;
                            for (auto j = 0; j < phi->atoms.size(); ++j) {
                                out_str << "location:" << name << ":ell_" << name_id_map.at("1_" + std::to_string(j)) << "{labels: accept_" << phi->id << "_" << i << "}" << std::endl;
                            }

                        } else {

                            out_str << "\t<template>" << std::endl;
                            out_str << "\t\t<name>" << name << "</name>" << std::endl;

                            out_str << "\t\t<declaration>" << std::endl;


                            out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at(i == 0 ? "0i" : "1_" + std::to_string(phi->atoms.size() - 1)) << ";" << std::endl;


                            out_str << "\t\t</declaration>" << std::endl;

                            out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                            out_str << "\t\t</location>" << std::endl;
                            out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                        }

                    }

                    if (i == 0) {

                        // 0i -> 0, !out_i, x := 0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "0", std::string{}, std::string{}, 1, !out_i & !in_i);

                        // 0i -> 0, out_i, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, out_i & !in_i);

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            if (j != 0) {

                                // 0i -> 1_j, !out_i && !out_null && ^p_j, x <= a

                                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !out_i & !out_null & phi->atoms[j]->hat & !in_i);

                            }

                            // 0i -> 1_j, out_null && ^p_j

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), std::string{}, std::string{}, 0, out_null & phi->atoms[j]->hat & !in_i);

                            // 0i -> 1_j, out_i && ^p_j, x > a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, out_i & phi->atoms[j]->hat & !in_i);

                            if (j + 1 != phi->atoms.size()) {

                                // 0i -> 1_j, !out_i & !out_null && ~p_j, x <= a

                                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !out_i & !out_null & phi->atoms[j]->tilde & !in_i);

                                // 0i -> 1_j, out_null && ~p_j

                                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), std::string{}, std::string{}, 0, out_null & phi->atoms[j]->tilde & !in_i);

                                // 0i -> 1_j, out_i && ~p_j, x > a

                                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, out_i & phi->atoms[j]->tilde & !in_i);

                            }

                            // 0i -> 1_j, !out_i && *p_j, x := 0

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), std::string{}, std::string{}, 1, !out_i & phi->atoms[j]->star & in_i);

                            // 0i -> 1_j, out_i && *p_j, x := 0, x > a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, out_i & phi->atoms[j]->star & in_i);
                        }

                    }


                    // 0 -> 0, !out_i, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "0", std::string{}, std::string{}, 1, !out_i & !in_i);

                    // 1_0 -> 0, out_i, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, out_i & !in_i);

                    for (auto j = 0; j < phi->atoms.size(); ++j) {

                        if (j != 0) {

                            // 1_j -> 1_j, !out_i && !out_null && ^p_j, x <= a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !out_i & !out_null & phi->atoms[j]->hat & !in_i);

                        }

                        // 1_j -> 1_j, out_null && ^p_j

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, std::string{}, 0, out_null & phi->atoms[j]->hat & !in_i);

                        // 1_j -> 1_j, out_i && ^p_j, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, out_i & phi->atoms[j]->hat & !in_i);

                        if (j + 1 != phi->atoms.size()) {

                            // 1_j+1 -> 1_j, !out_i & !out_null && ~p_j, x <= a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !out_i & !out_null & phi->atoms[j]->tilde & !in_i);

                            // 1_j+1 -> 1_j, out_null && ~p_j

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), std::string{}, std::string{}, 0, out_null & phi->atoms[j]->tilde & !in_i);

                            // 1_j+1 -> 1_j, out_i && ~p_j, x > a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, out_i & phi->atoms[j]->tilde & !in_i);

                        }

                        // 0 -> 1_j, !out_i && *p_j, x := 0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "1_" + std::to_string(j), std::string{}, std::string{}, 1, !out_i & phi->atoms[j]->star & in_i);

                        // 1_0 -> 1_j, out_i && *p_j, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, out_i & phi->atoms[j]->star & in_i);
                    }

                    if (out_format.has_value() && !out_format.value() && !out_flatten) {
                        out_str << "\t</template>" << std::endl << std::endl;
                    }

                    ++components_counter;
                    components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, (i == 0 ? 1 + phi->atoms.size() : 0)));
                    locations.clear();
                    name_id_map.clear();
                    bdd_edges.clear();

                }

                clocks.clear();

                clocks.insert({0, "x0"});        // clock 0 is needed anyway
                name = "seq_in_" + std::to_string(phi->id);

                for (int i = 0; i < phi->atoms.size(); ++i) {

                    locations.push_back(monitaal::location_t(true, i, "h_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"h_" + std::to_string(i), i});
                    locations.push_back(monitaal::location_t(true, phi->atoms.size() + i, "e_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"e_" + std::to_string(i), phi->atoms.size() + i});

                }

                // assert(!acc_map.count(name));
                // acc_map.insert({ name, { {}, {} } });

                if (out_format.has_value() && !out_flatten) {

                    if (out_format.value()) {

                        out_str << std::endl << std::endl;
                        out_str << "# " << name << std::endl;
                        out_str << "process:" << name << std::endl;

                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << name << ":ell_" << name_id_map.at("h_" + std::to_string(i)) << "{" << "" << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                        }
                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << name << ":ell_" << name_id_map.at("e_" + std::to_string(i)) << "{" << (i == phi->atoms.size() - 1 ? "initial: : " : "") << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                        }

                    } else {

                        out_str << "\t<template>" << std::endl;
                        out_str << "\t\t<name>" << name << "</name>" << std::endl;

                        out_str << "\t\t<declaration>" << std::endl;


                        out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("e_" + std::to_string(phi->atoms.size() - 1)) << ";" << std::endl;


                        out_str << "\t\t</declaration>" << std::endl;

                        out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                        out_str << "\t\t</location>" << std::endl;
                        out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                    }

                }


                for (int i = 0; i < phi->atoms.size(); ++i) {

                    in_i = encode(i + 1, phi->id, phi->bits / 2 - 1);
                    out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 1);

                    in_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id, phi->bits / 2 - 1);
                    out_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2 - 1);

                    in_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id, phi->bits / 2 - 1);
                    out_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id + phi->bits / 2, phi->bits / 2 - 1);

                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i), in_null & !out_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "e_" + std::to_string(i), in_null & out_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), in_next_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "e_" + std::to_string(i), in_null & (out_null | out_i));
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), in_next_i & !out_next_i);

                }

                if (out_format.has_value() && !out_format.value() && !out_flatten) {
                    out_str << "\t</template>" << std::endl << std::endl;
                }

                ++components_counter;
                components.push_back(monitaal::TAwithBDDEdges("seq_in_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("e_" + std::to_string(phi->atoms.size() - 1))));
                locations.clear();
                name_id_map.clear();
                bdd_edges.clear();

                name = "seq_out_" + std::to_string(phi->id);

                for (int i = 0; i < phi->atoms.size(); ++i) {

                    locations.push_back(monitaal::location_t(true, i, "t_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"t_" + std::to_string(i), i});
                    locations.push_back(monitaal::location_t(true, phi->atoms.size() + i, "f_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"f_" + std::to_string(i), phi->atoms.size() + i});

                }

                // assert(!acc_map.count(name));
                // acc_map.insert({ name, { {}, {} } });

                if (out_format.has_value() && !out_flatten) {

                    if (out_format.value()) {

                        out_str << std::endl << std::endl;
                        out_str << "# " << name << std::endl;
                        out_str << "process:" << name << std::endl;

                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << name << ":ell_" << name_id_map.at("t_" + std::to_string(i)) << "{" << (i == 0 ? "initial: : " : "") << "labels: accept_seq_out_" << phi->id << "}" << std::endl;
                        }
                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << name << ":ell_" << name_id_map.at("f_" + std::to_string(i)) << "{" << "" << "labels: accept_seq_out_" << phi->id << "}" << std::endl;
                        }

                    } else {

                        out_str << "\t<template>" << std::endl;
                        out_str << "\t\t<name>" << name << "</name>" << std::endl;

                        out_str << "\t\t<declaration>" << std::endl;


                        out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("t_0") << ";" << std::endl;


                        out_str << "\t\t</declaration>" << std::endl;

                        out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                        out_str << "\t\t</location>" << std::endl;
                        out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                    }

                }


                for (int i = 0; i < phi->atoms.size(); ++i) {

                    in_i = encode(i + 1, phi->id, phi->bits / 2 - 1);
                    out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 1);

                    in_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id, phi->bits / 2 - 1);
                    out_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2 - 1);

                    in_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id, phi->bits / 2 - 1);
                    out_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id + phi->bits / 2, phi->bits / 2 - 1);

                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i), out_null & !in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "f_" + std::to_string(i), out_null & in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), out_i & !in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i), out_i & !in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), out_i & in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "f_" + std::to_string(i), out_i & in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i), out_null & in_null);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i), out_i & in_null);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), out_i & in_null);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), out_i & in_i);

                }

                if (out_format.has_value() && !out_format.value() && !out_flatten) {
                    out_str << "\t</template>" << std::endl << std::endl;
                }

                ++components_counter;
                components.push_back(monitaal::TAwithBDDEdges("seq_out_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("t_" + std::to_string(0))));
                locations.clear();
                name_id_map.clear();
                bdd_edges.clear();

                if (comp_flatten) {

                    components_counter = components_counter - phi->atoms.size() - 2;

                    components.push_back(varphi_);

                    name = "TA_" + std::to_string(phi->id) + "_trig";

                    clocks.clear();
                    clocks.insert({0, "x0"});        // clock 0 is needed anyway

                    // monitaal::constraints_t empty_invariant;
                    // monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));

                    monitaal::constraints_t guard;
                    monitaal::clocks_t reset;

                    // 0 -> 0, (p && hat) || (!p && star)
                    
                    bdd label;
                    label = (bdd_ithvar(phi->id + phi->bits / 2 + phi->bits / 2 - 1) & phi->comp_hat) | (!bdd_ithvar(phi->id + phi->bits / 2 + phi->bits / 2 - 1) & phi->comp_star);

                    bdd_edges.push_back(monitaal::bdd_edge_t(0, 0, guard, reset, label));

                    monitaal::TAwithBDDEdges trig = monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0);   // last arg: initial location id
                    clocks.clear();
                    locations.clear();
                    bdd_edges.clear();

                    components.push_back(trig);

                    monitaal::TAwithBDDEdges product = monitaal::TAwithBDDEdges::intersection(components);

                    std::set<int> props_to_remove;

                    for (int j = phi->id; j < phi->id + phi->bits; ++j) {

                        if (j != phi->id + phi->bits / 2 + phi->bits / 2 - 1) {
                            props_to_remove.insert(j);
                        }

                    }
                    monitaal::TAwithBDDEdges projected = product.projection_bdd(props_to_remove);

                    std::stringstream().swap(out_str);
                    build_model_from_ta_bdd(projected, phi->id, out_str);

                    ++components_counter;
                    return { { projected }, out_str.str() };

                } else if (out_flatten) {

                    components_counter = components_counter - phi->atoms.size() - 2;

                    components.push_back(varphi_);
                    monitaal::TAwithBDDEdges product = monitaal::TAwithBDDEdges::intersection(components);

                    ++components_counter;
                    return { { product }, std::string{} };

                } else {

                    return { components, out_str.str() };   // components not really used further in this case

                }

            } else {

                assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

            }

        }

        assert(false);

    }


} // namespace mightypplcpp
