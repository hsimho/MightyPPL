#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_counton(const MitlParser::AtomContext* phi_) {

        std::stringstream out_str;

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)phi_;

        std::string name;

        if (phi->interval() == nullptr) {

            assert(("There should be a general interval on CFn COn CGn CHn", false));

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            /***** Count On
             <l, u>
            *****/
            
            clocks.insert({1, "x1"});
            clocks.insert({2, "y1"});

            std::vector<monitaal::TAwithBDDEdges> components;

            monitaal::constraints_t empty_invariant;
            monitaal::locations_t locations;

            std::map<std::string, monitaal::location_id_t> name_id_map;

            monitaal::bdd_edges_t bdd_edges;

            bdd in_null = encode(0, phi->id, phi->bits / 2 - 2);
            bdd out_null = encode(0, phi->id + phi->bits / 2, phi->bits / 2 - 2);
            bdd in_i;
            bdd out_i;
            bdd in_next_i;
            bdd out_next_i;
            bdd in_prev_i;
            bdd out_prev_i;


            for (auto i = 0; i < phi->num_pairs; ++i) {

                name = "TA_" + std::to_string(phi->id) + "_" + std::to_string(i);
                in_i = encode(i + 1, phi->id, phi->bits / 2 - 2);
                out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 2);

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                name_id_map.insert({"0", 0});

                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));
                name_id_map.insert({"1", 1});

                locations.push_back(monitaal::location_t(true, 2, "s1_0", empty_invariant));
                name_id_map.insert({"1_0", 2});

                locations.push_back(monitaal::location_t(true, 3, "s1_1", empty_invariant));
                name_id_map.insert({"1_1", 3});

                // assert(!acc_map.count(name));
                // acc_map.insert({ name, { {}, {} } });


                if (out_format.has_value() && !out_flatten) {

                    if (out_format.value()) {

                        out_str << std::endl << std::endl;
                        out_str << "# " << name << " (" << i + 1 << " / " << phi->num_pairs << ")" << std::endl;
                        out_str << "# " << const_cast<MitlParser::AtomCOnContext*>(phi)->getText() << std::endl;
                        out_str << "process:" << name << std::endl;

                        out_str << "location:" << name << ":ell_" << name_id_map.at("0") << "{initial: : labels: accept_" << phi->id << "_" << i << "}" << std::endl;

                        out_str << "location:" << name << ":ell_" << name_id_map.at("1") << "{labels: accept_" << phi->id << "_" << i << "}" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("1_0") << "{labels: accept_" << phi->id << "_" << i << "}" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("1_1") << "{labels: accept_" << phi->id << "_" << i << "}" << std::endl;

                    } else {

                        out_str << "\t<template>" << std::endl;
                        out_str << "\t\t<name>" << name << "</name>" << std::endl;

                        out_str << "\t\t<declaration>" << std::endl;


                        out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("0") << ";" << std::endl;


                        out_str << "\t\t</declaration>" << std::endl;

                        out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                        out_str << "\t\t</location>" << std::endl;
                        out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                    }

                }

                // 0 -> 0, !out_i & !in_i, x := 0, y := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "0", std::string{}, std::string{}, 3, !out_i & !in_i);


                // 1 -> 0, out_i & !in_i, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, out_i & !in_i);


                // 1 -> 1, out_null & !in_i, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 0, out_null & !in_i);

                // 1 -> 1, out_i & !in_i, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 0, out_i & !in_i);


                // // 1_0 -> 0, out_i & !in_i, x := 0, y := 0, x <= b, y > a

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, out_i & !in_i);

                // // 1_0 -> 1, !out_i & !out_null & !in_i, x <= b, y <= a

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 0, !out_i & !out_null & !in_i);

                // 1_0 -> 1, out_i & !in_i, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 0, out_i & !in_i);

                // 1_0 -> 1_0, !out_i & !in_i, x <= b

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !out_i & !in_i);


                // 1 -> 1, out_i & in_i & ^phi, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, out_i & in_i & phi->atom(1)->hat);

                // // 1 -> 1_0, out_i & in_i & ^phi, x := 0, y := 0, x <= b, y > a

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, out_i & in_i & phi->atom(1)->hat);

                // 1 -> 1_1, out_i & in_i & ^phi, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1_1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, out_i & in_i & phi->atom(1)->hat);


                // // 0 -> 1, !out_i & in_i & ^phi, x := 0, y := 0

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "1", std::string{}, std::string{}, 3, !out_i & in_i & phi->atom(1)->hat);

                // // 0 -> 1_0, !out_i & in_i & ^phi, x := 0, y := 0

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "1_0", std::string{}, std::string{}, 3, !out_i & in_i & phi->atom(1)->hat);

                // 1_1 -> 1, out_i & !in_i, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 0, out_i & !in_i);

                // 1_1 -> 0, out_i & !in_i, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, out_i & !in_i);

                // // 1_1 -> 1, !out_i & in_i & ^phi, x := 0, x < 1

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1", (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RParen ? "< " : "<= ") + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), std::string{}, 1, !out_i & in_i & phi->atom(1)->hat);

                // // 1_1 -> 1_0, !out_i & in_i & ^phi, x := 0, x < 1

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1_0", (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RParen ? "< " : "<= ") + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), std::string{}, 1, !out_i & in_i & phi->atom(1)->hat);

                // 1_1 -> 1_0, !out_i & !in_i && !in_null, x >= 1

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1_0", (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RParen ? ">= " : "> ") + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), std::string{}, 0, !out_i & !in_i & !in_null);

                // 1_1 -> 1_1, !out_i & in_i & ^phi, x := 0, x < 1

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1_1", (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RParen ? "< " : "<= ") + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), std::string{}, 1, !out_i & in_i & phi->atom(1)->hat);

                // 1_1 -> 1_1, !out_i & in_null & ~phi, x < 1

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1_1", (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RParen ? "< " : "<= ") + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), std::string{}, 0, !out_i & in_null & phi->atom(1)->tilde);

                // 0 -> 1_1, !out_i & in_i & ^phi, x := 0, y := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "1_1", std::string{}, std::string{}, 3, !out_i & in_i & phi->atom(1)->hat);


                if (out_format.has_value() && !out_format.value() && !out_flatten) {
                    out_str << "\t</template>" << std::endl << std::endl;
                }

                ++components_counter;
                components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
                locations.clear();
                name_id_map.clear();
                bdd_edges.clear();

            }

            clocks.clear();

            clocks.insert({0, "x0"});        // clock 0 is needed anyway
            name = "seq_in_" + std::to_string(phi->id);

            for (int i = 0; i < phi->num_pairs; ++i) {

                locations.push_back(monitaal::location_t(true, i, "h_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"h_" + std::to_string(i), i});
                locations.push_back(monitaal::location_t(true, phi->num_pairs + i, "e_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"e_" + std::to_string(i), phi->num_pairs + i});

            }

            // assert(!acc_map.count(name));
            // acc_map.insert({ name, { {}, {} } });

            if (out_format.has_value() && !out_flatten) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << name << std::endl;
                    out_str << "# " << "in = " << "p" << phi->id << " to " << "p" << phi->id + phi->bits / 2 - 2 << std::endl;
                    out_str << "# " << "out = " << "p" << phi->id + phi->bits / 2 << " to " << "p" << phi->id + phi->bits - 2 << std::endl;
                    out_str << "process:" << name << std::endl;

                    for (int i = 0; i < phi->num_pairs; ++i) {
                        out_str << "location:" << name << ":ell_" << name_id_map.at("h_" + std::to_string(i)) << "{" << "" << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                    }
                    for (int i = 0; i < phi->num_pairs; ++i) {
                        out_str << "location:" << name << ":ell_" << name_id_map.at("e_" + std::to_string(i)) << "{" << (i == phi->num_pairs - 1 ? "initial: : " : "") << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                    }

                } else {

                    out_str << "\t<template>" << std::endl;
                    out_str << "\t\t<name>" << name << "</name>" << std::endl;

                    out_str << "\t\t<declaration>" << std::endl;


                    out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("e_" + std::to_string(phi->num_pairs - 1)) << ";" << std::endl;


                    out_str << "\t\t</declaration>" << std::endl;

                    out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                    out_str << "\t\t</location>" << std::endl;
                    out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                }

            }


            for (int i = 0; i < phi->num_pairs; ++i) {

                in_i = encode(i + 1, phi->id, phi->bits / 2 - 2);
                out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 2);

                in_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id, phi->bits / 2 - 2);
                out_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2 - 2);

                in_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id, phi->bits / 2 - 2);
                out_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id + phi->bits / 2, phi->bits / 2 - 2);

                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i), (in_i | in_null) & !out_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "e_" + std::to_string(i), in_null & out_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), in_next_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "e_" + std::to_string(i), in_null & (out_null | out_i));
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), in_next_i & !out_next_i);

            }

            if (out_format.has_value() && !out_format.value() && !out_flatten) {
                out_str << "\t</template>" << std::endl << std::endl;
            }

            ++components_counter;
            components.push_back(monitaal::TAwithBDDEdges("seq_in_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("e_" + std::to_string(phi->num_pairs - 1))));
            locations.clear();
            name_id_map.clear();
            bdd_edges.clear();

            name = "seq_out_" + std::to_string(phi->id);

            for (int i = 0; i < phi->num_pairs; ++i) {

                locations.push_back(monitaal::location_t(true, i, "t_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"t_" + std::to_string(i), i});
                locations.push_back(monitaal::location_t(true, phi->num_pairs + i, "f_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"f_" + std::to_string(i), phi->num_pairs + i});

            }

            // assert(!acc_map.count(name));
            // acc_map.insert({ name, { {}, {} } });

            if (out_format.has_value() && !out_flatten) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << name << std::endl;
                    out_str << "# " << "in = " << "p" << phi->id << " to " << "p" << phi->id + phi->bits / 2 - 2 << std::endl;
                    out_str << "# " << "out = " << "p" << phi->id + phi->bits / 2 << " to " << "p" << phi->id + phi->bits - 2 << std::endl;
                    out_str << "process:" << name << std::endl;

                    for (int i = 0; i < phi->num_pairs; ++i) {
                        out_str << "location:" << name << ":ell_" << name_id_map.at("t_" + std::to_string(i)) << "{" << (i == 0 ? "initial: : " : "") << "labels: accept_seq_out_" << phi->id << "}" << std::endl;
                    }
                    for (int i = 0; i < phi->num_pairs; ++i) {
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


            for (int i = 0; i < phi->num_pairs; ++i) {

                in_i = encode(i + 1, phi->id, phi->bits / 2 - 2);
                out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 2);

                in_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id, phi->bits / 2 - 2);
                out_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2 - 2);

                in_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id, phi->bits / 2 - 2);
                out_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id + phi->bits / 2, phi->bits / 2 - 2);

                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i), out_null & !in_prev_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "f_" + std::to_string(i), out_null & in_prev_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), out_i & !in_prev_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i), out_i & !in_prev_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), out_i & in_prev_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "t_" + std::to_string(i), "f_" + std::to_string(i), out_i & in_prev_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i), out_null & (in_null | in_prev_i));
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i), out_i & (in_null | in_prev_i));
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "t_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), out_i & (in_null | in_prev_i));
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_out_" + std::to_string(phi->id), "f_" + std::to_string(i), "f_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), out_i & in_i);

            }

            if (out_format.has_value() && !out_format.value() && !out_flatten) {
                out_str << "\t</template>" << std::endl << std::endl;
            }

            ++components_counter;
            components.push_back(monitaal::TAwithBDDEdges("seq_out_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("t_" + std::to_string(0))));
            locations.clear();
            name_id_map.clear();
            bdd_edges.clear();

            /***** Since
             [l, infty) or (l, infty)
            *****/

            clocks.insert({1, "x1"});

            name = "TA_" + std::to_string(phi->id) + "_" + std::to_string(phi->num_pairs);
            in_i = bdd_ithvar(phi->id + phi->bits / 2 - 2);
            out_i = bdd_ithvar(phi->id + phi->bits - 2);

            if (out_format.has_value() && out_format.value() && !out_flatten) {

                out_str << std::endl << std::endl;
                out_str << "# " << name << std::endl;
                // out_str << "# " << const_cast<MitlParser::AtomUContext*>(phi)->getText() << std::endl;
                out_str << "process:" << name << std::endl;

            }

            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));
            // locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

            name_id_map.insert({"0", 0});
            name_id_map.insert({"1", 1});
            // name_id_map.insert({"2", 2});
            
            // assert(!acc_map.count(name));
            // acc_map.insert({ name, { {}, {} } });

            if (out_format.has_value() && !out_flatten) {

                if (out_format.value()) {

                    out_str << "location:" << name << ":ell_" << name_id_map.at("0") << "{initial: : labels: accept_" << phi->id << "_" << phi->num_pairs << "}" << std::endl;
                    out_str << "location:" << name << ":ell_" << name_id_map.at("1") << "{labels: accept_" << phi->id << "_" << phi->num_pairs << "}" << std::endl;
                    // out_str << "location:" << "TA_" << phi->id << ":ell_" << name_id_map.at("2") << "{labels: accept_" << phi->id << "}" << std::endl;
                    // out_str << "location:" << "TA_" << phi->id << ":ell_" << name_id_map.at("3") << "{}" << std::endl;


                } else {

                    out_str << "\t<template>" << std::endl;
                    out_str << "\t\t<name>" << name << "</name>" << std::endl;

                    out_str << "\t\t<declaration>" << std::endl;


                    out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("0") << ";" << std::endl;


                    out_str << "\t\t</declaration>" << std::endl;

                    out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                    out_str << "\t\t</location>" << std::endl;
                    out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                }

            }


            // 0 -> 0, !r && *p && *q, x := 0

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "0", "0", std::string{}, std::string{}, 1, !out_i & phi->atom(0)->star & phi->atom(1)->star & !in_i);

            // 1 -> 0, r && *p && *q, x := 0, x >= a

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, out_i & phi->atom(0)->star & phi->atom(1)->star & !in_i);

            // 1 -> 1, !r && ^p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "1", std::string{}, std::string{}, 0, !out_i & phi->atom(0)->hat & phi->atom(1)->star & !in_i);

            // 1 -> 1, r && ^p && *q, x >= a

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, out_i & phi->atom(0)->hat & phi->atom(1)->star & !in_i);

            // 1 -> 1, ~p && ^q, x := 0, x >= a

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, phi->atom(0)->tilde & phi->atom(1)->hat & in_i);

            // 0 -> 1, !r && *p && ^q, x := 0

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "0", "1", std::string{}, std::string{}, 1, !out_i & phi->atom(0)->star & phi->atom(1)->hat & in_i);


            if (out_format.has_value() && !out_format.value() && !out_flatten) {
                out_str << "\t</template>" << std::endl << std::endl;
            }

            ++components_counter;
            components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
            locations.clear();
            name_id_map.clear();
            bdd_edges.clear();

            if (comp_flatten) {

                components_counter = components_counter - phi->num_pairs - 3;

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

                components_counter = components_counter - phi->num_pairs - 3;

                components.push_back(varphi_);
                monitaal::TAwithBDDEdges product = monitaal::TAwithBDDEdges::intersection(components);

                ++components_counter;
                return { { product }, std::string{} };

            } else {

                return { components, out_str.str() };   // components not really used further in this case

            }

        }

        assert(false);

    }


} // namespace mightypplcpp
 
