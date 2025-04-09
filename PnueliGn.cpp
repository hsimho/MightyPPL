#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelign(const MitlParser::AtomContext* phi_) {

        std::stringstream out_str;

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)phi_;

        std::string name;

        if (phi->interval() == nullptr) {

                assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                /***** Pnueli Gn
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

                bdd in_null = encode(0, phi->id, phi->bits / 2);
                bdd out_null = encode(0, phi->id + phi->bits / 2, phi->bits / 2);
                bdd in_i;
                bdd out_i;
                bdd in_next_i;
                bdd out_next_i;
                bdd in_prev_i;
                bdd out_prev_i;


                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    name = "TA_" + std::to_string(phi->id) + "_" + std::to_string(i);
                    in_i = encode(i + 1, phi->id, phi->bits / 2);
                    out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2);

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    name_id_map.insert({"0", 0});
                    for (auto j = 0; j < phi->atoms.size(); ++j) {
                        locations.push_back(monitaal::location_t(true, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                        name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                    }

                    if (out_format.has_value() && out_format.value()) {

                            out_str << std::endl << std::endl;
                            out_str << "# " << "TA_" << phi->id << "_" << i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                            out_str << "# " << const_cast<MitlParser::AtomGnContext*>(phi)->getText() << std::endl;
                            out_str << "process:" << "TA_" << phi->id << "_" << i << std::endl;

                            out_str << "location:" << "TA_" << phi->id << "_" << i << ":ell_0{initial: : labels: accept_" << phi->id << "_" << i << "}" << std::endl;
                            for (auto j = 0; j < phi->atoms.size(); ++j) {
                                out_str << "location:" << "TA_" << phi->id << "_" << i << ":ell_1_" << j << "{labels: accept_" << phi->id << "_" << i << "}" << std::endl;
                            }

                    }

                    // 0 -> 0, !in_i, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "0", std::string{}, std::string{}, 1, !in_i & !out_i);

                    // 0 -> 1_0, in_i, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "1_0", std::string{}, std::string{}, 1, in_i & !out_i);

                    for (auto j = 0; j < phi->atoms.size(); ++j) {

                        // 1_j -> 1_j, !in_i (in_null) && ^p_j, x <= a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, (j == 0 ? in_null : !in_i) & phi->atoms[j]->hat & !out_i);

                        // 1_j -> 1_j, in_i && ^p_j, x := 0, x <= a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, in_i & phi->atoms[j]->hat & !out_i);

                        if (j + 1 != phi->atoms.size()) {

                            // 1_j -> 1_j+1, !in_i && ~p_j, x <= a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !in_i & phi->atoms[j]->tilde & !out_i);

                            // 1_j -> 1_j+1, in_i && ~p_j, x := 0, x <= a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, in_i & phi->atoms[j]->tilde & !out_i);

                        }

                        // 1_j -> 0, !in_i && *p_j, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, !in_i & phi->atoms[j]->star & out_i);

                        // 1_j -> 1_0, in_i && *p_j, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_" + std::to_string(j), "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, in_i & phi->atoms[j]->star & out_i);


                    }

                    components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
                    locations.clear();
                    name_id_map.clear();
                    bdd_edges.clear();

                }

                clocks.clear();

                clocks.insert({0, "x0"});        // clock 0 is needed anyway

                for (int i = 0; i < phi->atoms.size(); ++i) {

                    locations.push_back(monitaal::location_t(true, i, "h_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"h_" + std::to_string(i), i});
                    locations.push_back(monitaal::location_t(true, phi->atoms.size() + i, "e_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"e_" + std::to_string(i), phi->atoms.size() + i});

                }

                if (out_format.has_value() && out_format.value()) {

                        out_str << std::endl << std::endl;
                        out_str << "# " << "seq_in_" << phi->id << std::endl;
                        out_str << "process:" << "seq_in_" << phi->id << std::endl;

                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << "seq_in_" << phi->id << ":h_" << i << "{" << "" << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                        }
                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << "seq_in_" << phi->id << ":e_" << i << "{" << (i == phi->atoms.size() - 1 ? "initial: : " : "") << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                        }

                }


                for (int i = 0; i < phi->atoms.size(); ++i) {

                    in_i = encode(i + 1, phi->id, phi->bits / 2);
                    out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2);

                    in_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id, phi->bits / 2);
                    out_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2);

                    in_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id, phi->bits / 2);
                    out_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id + phi->bits / 2, phi->bits / 2);

                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i), (in_i | in_null) & !out_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "e_" + std::to_string(i), in_null & out_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), in_next_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "e_" + std::to_string(i), in_null & out_null);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), in_next_i & !out_next_i);

                }

                components.push_back(monitaal::TAwithBDDEdges("seq_in_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("e_" + std::to_string(phi->atoms.size() - 1))));
                locations.clear();
                name_id_map.clear();
                bdd_edges.clear();


                for (int i = 0; i < phi->atoms.size(); ++i) {

                    locations.push_back(monitaal::location_t(true, i, "t_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"t_" + std::to_string(i), i});
                    locations.push_back(monitaal::location_t(true, phi->atoms.size() + i, "f_" + std::to_string(i), empty_invariant));
                    name_id_map.insert({"f_" + std::to_string(i), phi->atoms.size() + i});

                }

                if (out_format.has_value() && out_format.value()) {

                        out_str << std::endl << std::endl;
                        out_str << "# " << "seq_out_" << phi->id << std::endl;
                        out_str << "process:" << "seq_out_" << phi->id << std::endl;

                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << "seq_out_" << phi->id << ":t_" << i << "{" << (i == 0 ? "initial: : " : "") << "labels: accept_seq_out_" << phi->id << "}" << std::endl;
                        }
                        for (int i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "location:" << "seq_out_" << phi->id << ":f_" << i << "{" << "" << "labels: accept_seq_out_" << phi->id << "}" << std::endl;
                        }

                }


                for (int i = 0; i < phi->atoms.size(); ++i) {

                    in_i = encode(i + 1, phi->id, phi->bits / 2);
                    out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2);

                    in_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id, phi->bits / 2);
                    out_next_i = encode((i + 2 > phi->atoms.size() ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2);

                    in_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id, phi->bits / 2);
                    out_prev_i = encode((i == 0 ? phi->atoms.size() : i), phi->id + phi->bits / 2, phi->bits / 2);

                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i), out_null & !in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "f_" + std::to_string(i), out_null & in_prev_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), out_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i), out_null & (in_null | in_prev_i));
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), out_i & !in_i);
                    build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i + 1 == phi->atoms.size() ? 0 : i + 1), out_i & in_i);

                }

                components.push_back(monitaal::TAwithBDDEdges("seq_out_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("t_" + std::to_string(0))));
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
