#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelihn(const MitlParser::AtomContext* phi_) {

        std::stringstream out_str;

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)phi_;

        std::string name = "TA_" + std::to_string(phi->id);

        if (phi->interval() == nullptr) {

                assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                /***** Pnueli On Dual
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

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    // The current version of MoniTAal only allows 1 initial location

                    locations.push_back(monitaal::location_t(false, 1 + phi->atoms.size(), "s0i", empty_invariant));
                    name_id_map.insert({"0i", 1 + phi->atoms.size()});
                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    name_id_map.insert({"0", 0});
                    for (auto j = 0; j < phi->atoms.size(); ++j) {
                        locations.push_back(monitaal::location_t(true, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                        name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                    }

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << std::endl << std::endl;
                            out_str << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                            out_str << "# " << const_cast<MitlParser::AtomHnContext*>(phi)->getText() << std::endl;
                            out_str << "process:" << "TA_" << phi->id + i << std::endl;

                            out_str << "location:" << "TA_" << phi->id + i << ":ell_0i{initial: }" << std::endl;
                            out_str << "location:" << "TA_" << phi->id + i << ":ell_0{labels: accept_" << phi->id + i << "}" << std::endl;
                            for (auto j = 0; j < phi->atoms.size(); ++j) {
                                out_str << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{labels: accept_" << phi->id + i << "}" << std::endl;
                            }

                        }

                    }


                    // 0i -> 0, !r, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "0", std::string{}, std::string{}, 1, !encode(i + 1, phi->id, phi->bits));

                    // 0i -> 0, r, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits));

                    for (auto j = 0; j < phi->atoms.size(); ++j) {

                        // 0i -> 1_j, !r && ^p_j

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "1_" + std::to_string(j), std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                        // 0i -> 1_j, r && ^p_j, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                        if (j + 1 != phi->atoms.size()) {

                            // 0i -> 1_j, !r && ~p_j

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "1_" + std::to_string(j), std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->tilde);

                            // 0i -> 1_j, r && ~p_j, x > a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->tilde);

                        }

                        // 0i -> 1_j, !r && *p_j, x := 0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "1_" + std::to_string(j), std::string{}, std::string{}, 1, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->star);

                        // 0i -> 1_j, r && *p_j, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->star);
                    }



                    // 0 -> 0, !r, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "0", std::string{}, std::string{}, 1, !encode(i + 1, phi->id, phi->bits));

                    // 1_0 -> 0, r, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits));

                    for (auto j = 0; j < phi->atoms.size(); ++j) {

                        // 1_j -> 1_j, !r && ^p_j

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                        // 1_j -> 1_j, r && ^p_j, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                        if (j + 1 != phi->atoms.size()) {

                            // 1_j+1 -> 1_j, !r && ~p_j

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->tilde);

                            // 1_j+1 -> 1_j, r && ~p_j, x > a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->tilde);

                        }

                        // 0 -> 1_j, !r && *p_j, x := 0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "1_" + std::to_string(j), std::string{}, std::string{}, 1, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->star);

                        // 1_0 -> 1_j, r && *p_j, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_0", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->star);
                    }

                    components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 1 + phi->atoms.size()));
                    locations.clear();
                    name_id_map.clear();
                    bdd_edges.clear();

                }

                clocks.clear();
                clocks.insert({0, "x0"});        // clock 0 is needed anyway

                for (int i = 0; i < phi->atoms.size(); ++i) {

                    locations.push_back(monitaal::location_t(true, i, "s" + std::to_string(i), empty_invariant));

                }

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << std::endl << std::endl;
                        out_str << "# " << "seq_" << phi->id << std::endl;
                        out_str << "process:" << "seq_" << phi->id << std::endl;

                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << "seq_" << phi->id << ":ell_" << i << "{" << (i == 0 ? "initial: : " : "") << "labels: accept_seq_" << phi->id << "}" << std::endl;
                        }
                    }
                }


                for (int i = 0; i < phi->atoms.size(); ++i) {

                    bdd_edges.push_back(monitaal::bdd_edge_t(i, i, monitaal::constraints_t{}, monitaal::clocks_t{}, encode(0, phi->id, phi->bits) | encode(i + 1, phi->id, phi->bits)));

                    int from = i;
                    int to = i;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            bdd_allsat(encode(0, phi->id, phi->bits) | encode(i + 1, phi->id, phi->bits), *allsat_print_handler);

                            std::stringstream p_constraint;
                            std::string s;
                            for (auto i = 0; i < sat_paths.size(); ++i) {

                                int largest = 0;
                                for (auto j = 1; j < sat_paths[i].size(); ++j) {
                                    if (sat_paths[i][j] != 'X' && j > largest) {
                                        largest = j;
                                    }
                                }
                                for (auto j = 1; j < sat_paths[i].size(); ++j) {
                                    if (sat_paths[i][j] != 'X') {
                                        p_constraint << "p_" << j << (sat_paths[i][j] == '0' ? " == 0" : " == 1");
                                        if (j != largest) {
                                            p_constraint << " && ";
                                        }
                                    }
                                }
                                s = s + std::string("edge:") + "seq_" + std::to_string(phi->id) + ":ell_" + std::to_string(from) + ":ell_" + std::to_string(to) + ":a{provided: turn == 1"
                                                    + (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                                    + "}" 
                                                    + "\n"; 

                                std::stringstream().swap(p_constraint);

                            }

                            sat_paths.clear();
                            out_str << s;

                        }
                    }

                }

                for (int i = 0; i < phi->atoms.size(); ++i) {

                    bdd_edges.push_back(monitaal::bdd_edge_t(i, (i + 1 == phi->atoms.size() ? 0 : i + 1), monitaal::constraints_t{}, monitaal::clocks_t{}, encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id, phi->bits)));

                    int from = i;
                    int to = (i + 1 == phi->atoms.size() ? 0 : i + 1);

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            bdd_allsat(encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id, phi->bits), *allsat_print_handler);

                            std::stringstream p_constraint;
                            std::string s;
                            for (auto i = 0; i < sat_paths.size(); ++i) {

                                int largest = 0;
                                for (auto j = 1; j < sat_paths[i].size(); ++j) {
                                    if (sat_paths[i][j] != 'X' && j > largest) {
                                        largest = j;
                                    }
                                }
                                for (auto j = 1; j < sat_paths[i].size(); ++j) {
                                    if (sat_paths[i][j] != 'X') {
                                        p_constraint << "p_" << j << (sat_paths[i][j] == '0' ? " == 0" : " == 1");
                                        if (j != largest) {
                                            p_constraint << " && ";
                                        }
                                    }
                                }
                                s = s + std::string("edge:") + "seq_" + std::to_string(phi->id) + ":ell_" + std::to_string(from) + ":ell_" + std::to_string(to) + ":a{provided: turn == 1"
                                                    + (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                                    + "}" 
                                                    + "\n"; 

                                std::stringstream().swap(p_constraint);

                            }

                            sat_paths.clear();
                            out_str << s;

                        }
                    }
                
                }

                components.push_back(monitaal::TAwithBDDEdges("seq_" + std::to_string(phi->id), clocks, locations, bdd_edges, 0));
                locations.clear();
                name_id_map.clear();
                bdd_edges.clear();

                return { components, out_str.str() };

            } else {

                assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

            }

        }

        assert(false);

    }


} // namespace mightypplcpp
