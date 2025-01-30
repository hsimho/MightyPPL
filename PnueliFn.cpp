#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelifn(std::stringstream& out_str, monitaal::clock_map_t& clocks, const MitlParser::AtomContext* phi_) {

        MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)phi_;

        std::string name = "TA_" + std::to_string(phi->id);


        if (phi->interval() == nullptr) {

                assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                /***** Pnueli Fn
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

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    name_id_map.insert({"0", 0});
                    for (auto j = 0; j < phi->atoms.size(); ++j) {
                        locations.push_back(monitaal::location_t(false, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                        name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                    }

                    locations.push_back(monitaal::location_t(out_fin ? false : true, 1 + phi->atoms.size(), "s2", empty_invariant));
                    name_id_map.insert({"2", 1 + phi->atoms.size()});

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << std::endl << std::endl;
                            out_str << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                            out_str << "# " << const_cast<MitlParser::AtomFnContext*>(phi)->getText() << std::endl;
                            out_str << "process:" << "TA_" << phi->id + i << std::endl;

                            out_str << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                            for (auto j = 0; j < phi->atoms.size(); ++j) {
                                out_str << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{}" << std::endl;
                            }

                            out_str << "location:" << "TA_" << phi->id + i << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id + i)) << "}" << std::endl;

                        }

                    }

                    // 0 -> 0, !p1, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "0", std::string{}, std::string{}, 1, !encode(i + 1, phi->id, phi->bits));

                    // 0 -> 1_0, p1, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "1_0", std::string{}, std::string{}, 1, encode(i + 1, phi->id, phi->bits));

                    for (auto j = 0; j < phi->atoms.size(); ++j) {

                        // 1_j -> 1_j, !p1 (!r) && ~p_j

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), std::string{}, 0, (j == 0 ? phi->tilde : !encode(i + 1, phi->id, phi->bits)) & phi->atoms[j]->star);

                        // 1_j -> 1_j+1, !p1 && ^p_j (x := 0, x <= a)

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? "0" : "1_" + std::to_string(j + 1)), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), std::string{}, (j + 1 == phi->atoms.size() ? true : false), !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                    }

                    // 1_0 -> 1_0, p1 && *p_0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_0", "1_0", std::string{}, std::string{}, 0, encode(i + 1, phi->id, phi->bits) & phi->atoms[0]->star);

                    // 1_n-1 -> 2, p1 && ^p_j, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->atoms.size() - 1), "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[phi->atoms.size() - 1]->hat);

                    // 2 -> 1_0, !r && ~p_0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "2", "1_0", std::string{}, std::string{}, 0, phi->tilde & phi->atoms[0]->star);

                    // 2 -> 1_1, !p1 && ^p_0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "2", "1_1", std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[0]->hat);

                    // 2 -> 1_0, p1 && *p_0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "2", "1_0", std::string{}, std::string{}, 0, encode(i + 1, phi->id, phi->bits) & phi->atoms[0]->star);


                    components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
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
