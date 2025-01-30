#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_countfn(std::stringstream& out_str, monitaal::clock_map_t& clocks, const MitlParser::AtomContext* phi_) {

        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)phi_;

        std::string name = "TA_" + std::to_string(phi->id);     // TODO: individual names for sub-components?

        if (phi->interval() == nullptr) {

                assert(("Unexpected interval on CFn COn CGn CHn", false));

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (
                (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0")
                || 
                (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty")
               ) {

                assert(("Unexpected interval on CFn COn CGn CHn", false));

            } else {

                /***** Count Fn
                 <l, u>
                *****/
                
                clocks.insert({1, "x1"});
                clocks.insert({2, "y1"});

                std::vector<monitaal::TAwithBDDEdges> components;

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                std::map<std::string, monitaal::location_id_t> name_id_map;

                monitaal::bdd_edges_t bdd_edges;


                for (auto i = 0; i < phi->max_l + 1; ++i) {

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    name_id_map.insert({"0", 0});
                    for (auto j = 0; j < phi->max_l + 1; ++j) {
                        locations.push_back(monitaal::location_t(false, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                        name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                    }

                    // TODO: distinguish fin/inf acceptance?

                    locations.push_back(monitaal::location_t(false, 1 + phi->max_l + 1, "s1_f", empty_invariant));
                    name_id_map.insert({"1_f", 1 + phi->max_l + 1});

                    locations.push_back(monitaal::location_t(false, 1 + phi->max_l + 2, "s1_g", empty_invariant));
                    name_id_map.insert({"1_g", 1 + phi->max_l + 2});

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << std::endl << std::endl;
                            out_str << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->max_l + 1 << ")" << std::endl;
                            out_str << "# " << const_cast<MitlParser::AtomCFnContext*>(phi)->getText() << std::endl;
                            out_str << "process:" << "TA_" << phi->id + i << std::endl;

                            out_str << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                            for (auto j = 0; j < phi->max_l + 1; ++j) {
                                out_str << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{labels: accept_" << phi->id + i << "}" << std::endl;
                            }

                            out_str << "location:" << "TA_" << phi->id + i << ":ell_1_f{labels: accept_" << phi->id + i << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id + i << ":ell_1_g{labels: accept_" << phi->id + i << "}" << std::endl;

                        }

                    }


                    // 0 -> 0, !p1, x := 0, y := 0
                    
                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "0", std::string{}, std::string{}, 3, !encode(i + 1, phi->id, phi->bits));

                    // 0 -> 1_0, p1, x := 0, y := 0
                    
                    // 0 -> 1_1, p1, x := 0, y := 0
                    
                    // 0 -> 1_2, p1, x := 0, y := 0
                   
                    // 0 -> 1_3, p1, x := 0, y := 0
                    
                    for (auto j = 0; j < phi->max_l + 1; ++j) {

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "1_" + std::to_string(j), std::string{}, std::string{}, 3, encode(i + 1, phi->id, phi->bits));

                    }


                    for (auto j = 0; j < phi->max_l; ++j) {

                        // 1_j -> 1_j, !p1 && ~phi, x <= b

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat);

                        // 1_j -> 1_j, p1 && ~phi, y := 0, x <= b

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 2, encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat);

                        // 1_j -> 1_j+1, !p1 && phi, x <= b

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atom(0)->hat);

                        // 1_j -> 1_j+1, p1 && phi, y := 0, x <= b

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 2, encode(i + 1, phi->id, phi->bits) & phi->atom(0)->hat);

                    }


                    // 1_3 -> 1_3, !p1 && ~phi && ~phi2, x <= b

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_" + std::to_string(phi->max_l), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat & phi->atom(3)->hat);
                    
                    // 1_3 -> 1_3, p1 && ~phi && ~phi2, y := 0, x <= b

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_" + std::to_string(phi->max_l), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 2, encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat & phi->atom(3)->hat);

                    // 1_3 -> 1_f, !p1 && ~phi && phi2, x := 0, x <= b

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_f", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, !encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat & phi->atom(2)->hat);

                    // 1_3 -> 1_f, p1 && ~phi && phi2, x := 0, y := 0, x <= b

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_f", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 3, encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat & phi->atom(2)->hat);

                    // 1_f -> 1_f, !p1 && ~phi, x := 0, y < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_f", "1_f", std::string{}, (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 1, !encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat);

                    // 1_f -> 1_f, p1 && ~phi, x := 0, y := 0, y < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_f", "1_f", std::string{}, (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 3, encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat);

                    // 1_f -> 0, !p1, x := 0; y := 0, y >= a
                    
                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_f", "0", std::string{}, (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, !encode(i + 1, phi->id, phi->bits));
    
                    // 1_f -> 1_0, p1, x := 0; y := 0, y >= a

                    // 1_f -> 1_1, p1, x := 0; y := 0, y >= a

                    // 1_f -> 1_2, p1, x := 0; y := 0, y >= a

                    // 1_f -> 1_3, p1, x := 0; y := 0, y >= a

                    for (auto j = 0; j < phi->max_l + 1; ++j) {

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_f", "1_" + std::to_string(j), std::string{}, (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, encode(i + 1, phi->id, phi->bits));

                    }

                    // 1_3 -> 1_g, !p1 && ~phi && ~phi2, y := 0, y >= a, x <= b

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_g", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 2, !encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat & phi->atom(3)->hat);

                    // 1_g -> 1_g, !p1 && ~phi && ~phi2, y := 0, x <= b
                    
                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_g", "1_g", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 2, !encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat & phi->atom(3)->hat);

                    // 1_g -> 0, !p1 && phi2, x := 0, y := 0, x <= b

                    build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_g", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 3, !encode(i + 1, phi->id, phi->bits) & phi->atom(2)->hat);



                    components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
                    locations.clear();
                    name_id_map.clear();
                    bdd_edges.clear();

                }


                clocks.clear();
                clocks.insert({0, "x0"});        // clock 0 is needed anyway

                for (int i = 0; i < phi->max_l + 1; ++i) {

                    locations.push_back(monitaal::location_t(true, i, "s" + std::to_string(i), empty_invariant));

                }

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << std::endl << std::endl;
                        out_str << "# " << "seq_" << phi->id << std::endl;
                        out_str << "process:" << "seq_" << phi->id << std::endl;

                        for (int i = 0; i < phi->max_l + 1; ++i) {
                            out_str << "location:" << "seq_" << phi->id << ":ell_" << i << "{" << (i == 0 ? "initial: : " : "") << "labels: accept_seq_" << phi->id << "}" << std::endl;
                        }
                    }
                }


                bdd bdd_mutex = bdd_true();

                bdd_mutex = bdd_mutex & ((phi->atom(0)->hat & phi->atom(1)->tilde) | (phi->atom(1)->hat & phi->atom(0)->tilde));
                bdd_mutex = bdd_mutex & ((phi->atom(2)->hat & phi->atom(3)->tilde) | (phi->atom(3)->hat & phi->atom(2)->tilde));

                for (int i = 0; i < phi->max_l + 1; ++i) {

                    bdd_edges.push_back(monitaal::bdd_edge_t(i, i, monitaal::constraints_t{}, monitaal::clocks_t{}, bdd_mutex & encode(0, phi->id, phi->bits) | encode(i + 1, phi->id, phi->bits)));
                    
                    int from = i;
                    int to = i;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            bdd_allsat(bdd_mutex & encode(0, phi->id, phi->bits) | encode(i + 1, phi->id, phi->bits), *allsat_print_handler);

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

                for (int i = 0; i < phi->max_l + 1; ++i) {

                    bdd_edges.push_back(monitaal::bdd_edge_t(i, (i + 1 == phi->max_l + 1 ? 0 : i + 1), monitaal::constraints_t{}, monitaal::clocks_t{}, bdd_mutex & encode((i + 2 > phi->max_l + 1 ? 1 : i + 2), phi->id, phi->bits)));

                    int from = i;
                    int to = (i + 1 == phi->max_l + 1 ? 0 : i + 1);

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            bdd_allsat(bdd_mutex & encode((i + 2 > phi->max_l + 1 ? 1 : i + 2), phi->id, phi->bits), *allsat_print_handler);

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

            }

        }

        assert(false);

    }

} // namespace mightypplcpp
