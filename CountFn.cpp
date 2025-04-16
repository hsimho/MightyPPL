#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_countfn(const MitlParser::AtomContext* phi_) {

        std::stringstream out_str;

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)phi_;

        std::string name;

        if (phi->interval() == nullptr) {

            assert(("There should be a general interval on CFn COn CGn CHn", false));

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

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

            bdd in_null = encode(0, phi->id, phi->bits / 2 - 1);
            bdd out_null = encode(0, phi->id + phi->bits / 2, phi->bits / 2 - 1);
            bdd in_i;
            bdd out_i;
            bdd in_next_i;
            bdd out_next_i;
            bdd in_prev_i;
            bdd out_prev_i;


            for (auto i = 0; i < phi->num_pairs; ++i) {

                name = "TA_" + std::to_string(phi->id) + "_" + std::to_string(i);
                in_i = encode(i + 1, phi->id, phi->bits / 2 - 1);
                out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 1);

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                name_id_map.insert({"0", 0});

                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                name_id_map.insert({"1", 1});

                locations.push_back(monitaal::location_t(false, 3, "s1_0", empty_invariant));
                name_id_map.insert({"1_0", 3});

                locations.push_back(monitaal::location_t(false, 4, "s1_1", empty_invariant));
                name_id_map.insert({"1_1", 4});

                locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));
                name_id_map.insert({"2", 2});

                if (out_format.has_value() && out_format.value() && !out_flatten) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << "_" << i << " (" << i + 1 << " / " << phi->num_pairs << ")" << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomCFnContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << "TA_" << phi->id << "_" << i << std::endl;

                    out_str << "location:" << "TA_" << phi->id << "_" << i << ":ell_0{initial: : labels: accept_" << phi->id << "_" << i << "}" << std::endl;

                    out_str << "location:" << "TA_" << phi->id << "_" << i << ":ell_1" << "{}" << std::endl;
                    out_str << "location:" << "TA_" << phi->id << "_" << i << ":ell_1_0" << "{}" << std::endl;
                    out_str << "location:" << "TA_" << phi->id << "_" << i << ":ell_1_1" << "{}" << std::endl;

                    out_str << "location:" << "TA_" << phi->id << "_" << i << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id) + "_" + std::to_string(i)) << "}" << std::endl;

                }

                // 0 -> 0, !in_i & !out_i, x := 0, y := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "0", std::string{}, std::string{}, 3, !in_i & !out_i);

                // 0 -> 1, in_i & !out_i, x := 0, y := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "0", "1", std::string{}, std::string{}, 3, in_i & !out_i);

                // 1 -> 1, in_null & !out_i & *phi, x <= b, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 0, in_null & !out_i & phi->atom(1)->star);

                // 1 -> 1, in_i & !out_i & *phi, y := 0, x <= b, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 2, in_i & !out_i & phi->atom(1)->star);

                // 1 -> 0, !in_i & out_i & ^phi, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, !in_i & out_i & phi->atom(1)->hat);

                // 1 -> 1_0, !in_i & out_i & ^phi, x := 0, x <= b, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 1, !in_i & out_i & phi->atom(1)->hat);

                // 1 -> 2, in_i & out_i & ^phi, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, in_i & out_i & phi->atom(1)->hat);

                // 2 -> 1, in_null & !out_i & *phi, x <= b, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 0, in_null & !out_i & phi->atom(1)->star);

                // 2 -> 1, in_i & !out_i & *phi, y := 0, x <= b, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 2, in_i & !out_i & phi->atom(1)->star);

                // 2 -> 0, !in_i & out_i & ^phi, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, !in_i & out_i & phi->atom(1)->hat);

                // 2 -> 1_0, !in_i & out_i & ^phi, x := 0, x <= b, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "2", "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 1, !in_i & out_i & phi->atom(1)->hat);

                // 2 -> 2, in_i & out_i & ^phi, x := 0, y := 0, x <= b, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, in_i & out_i & phi->atom(1)->hat);


                // 1_0 -> 1_0, !in_i & out_i & ^phi, x := 0, x < 1, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "1_0", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 1, !in_i & out_i & phi->atom(1)->hat);

                // 1_0 -> 1_1, !in_i & !out_i & ~phi, x < 1, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "1_1", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 0, !in_i & !out_i & phi->atom(1)->tilde);

                // 1_1 -> 1_1, !in_i & !out_i & ~phi, x < 1, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1_1", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 0, !in_i & !out_i & phi->atom(1)->tilde);

                // 1_1 -> 1_0, !in_i & out_i & ^phi, x := 0, x < 1, y <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "1_0", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), 1, !in_i & out_i & phi->atom(1)->hat);


                // 1_0 -> 2, in_i & out_i & ^phi, x := 0, y := 0, x < 1, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "2", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, in_i & out_i & phi->atom(1)->hat);

                // 1_1 -> 2, in_i & out_i & ^phi, x := 0, y := 0, x < 1, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "2", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, in_i & out_i & phi->atom(1)->hat);

                // 1_0 -> 0, !in_i & out_i & ^phi, x := 0, y := 0, x < 1, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_0", "0", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, !in_i & out_i & phi->atom(1)->hat);

                // 1_1 -> 0, !in_i & out_i & ^phi, x := 0, y := 0, x < 1, y > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, i, "1_1", "0", "< " + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), 3, !in_i & out_i & phi->atom(1)->hat);


                components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
                locations.clear();
                name_id_map.clear();
                bdd_edges.clear();

            }

            clocks.clear();

            clocks.insert({0, "x0"});        // clock 0 is needed anyway

            for (int i = 0; i < phi->num_pairs; ++i) {

                locations.push_back(monitaal::location_t(true, i, "h_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"h_" + std::to_string(i), i});
                locations.push_back(monitaal::location_t(true, phi->num_pairs + i, "e_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"e_" + std::to_string(i), phi->num_pairs + i});

            }

            if (out_format.has_value() && out_format.value() && !out_flatten) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "seq_in_" << phi->id << std::endl;
                    out_str << "# " << "in = " << "p" << phi->id << " to " << "p" << phi->id + phi->bits / 2 - 2 << std::endl;
                    out_str << "# " << "out = " << "p" << phi->id + phi->bits / 2 << " to " << "p" << phi->id + phi->bits - 2 << std::endl;
                    out_str << "process:" << "seq_in_" << phi->id << std::endl;

                    for (int i = 0; i < phi->num_pairs; ++i) {
                        out_str << "location:" << "seq_in_" << phi->id << ":h_" << i << "{" << "" << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                    }
                    for (int i = 0; i < phi->num_pairs; ++i) {
                        out_str << "location:" << "seq_in_" << phi->id << ":e_" << i << "{" << (i == phi->num_pairs - 1 ? "initial: : " : "") << "labels: accept_seq_in_" << phi->id << "}" << std::endl;
                    }

            }


            for (int i = 0; i < phi->num_pairs; ++i) {

                in_i = encode(i + 1, phi->id, phi->bits / 2 - 1);
                out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 1);

                in_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id, phi->bits / 2 - 1);
                out_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2 - 1);

                in_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id, phi->bits / 2 - 1);
                out_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id + phi->bits / 2, phi->bits / 2 - 1);

                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i), (in_i | in_null) & !out_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "e_" + std::to_string(i), in_null & out_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "h_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), in_next_i);
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "e_" + std::to_string(i), in_null & (out_null | out_i));
                build_untimed_edge(bdd_edges, name_id_map, out_str, "seq_in_" + std::to_string(phi->id), "e_" + std::to_string(i), "h_" + std::to_string(i + 1 == phi->num_pairs ? 0 : i + 1), in_next_i & !out_next_i);

            }

            components.push_back(monitaal::TAwithBDDEdges("seq_in_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("e_" + std::to_string(phi->num_pairs - 1))));
            locations.clear();
            name_id_map.clear();
            bdd_edges.clear();


            for (int i = 0; i < phi->num_pairs; ++i) {

                locations.push_back(monitaal::location_t(true, i, "t_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"t_" + std::to_string(i), i});
                locations.push_back(monitaal::location_t(true, phi->num_pairs + i, "f_" + std::to_string(i), empty_invariant));
                name_id_map.insert({"f_" + std::to_string(i), phi->num_pairs + i});

            }

            if (out_format.has_value() && out_format.value() && !out_flatten) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "seq_out_" << phi->id << std::endl;
                    out_str << "# " << "in = " << "p" << phi->id << " to " << "p" << phi->id + phi->bits / 2 - 2 << std::endl;
                    out_str << "# " << "out = " << "p" << phi->id + phi->bits / 2 << " to " << "p" << phi->id + phi->bits - 2 << std::endl;
                    out_str << "process:" << "seq_out_" << phi->id << std::endl;

                    for (int i = 0; i < phi->num_pairs; ++i) {
                        out_str << "location:" << "seq_out_" << phi->id << ":t_" << i << "{" << (i == 0 ? "initial: : " : "") << "labels: accept_seq_out_" << phi->id << "}" << std::endl;
                    }
                    for (int i = 0; i < phi->num_pairs; ++i) {
                        out_str << "location:" << "seq_out_" << phi->id << ":f_" << i << "{" << "" << "labels: accept_seq_out_" << phi->id << "}" << std::endl;
                    }

            }


            for (int i = 0; i < phi->num_pairs; ++i) {

                in_i = encode(i + 1, phi->id, phi->bits / 2 - 1);
                out_i = encode(i + 1, phi->id + phi->bits / 2, phi->bits / 2 - 1);

                in_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id, phi->bits / 2 - 1);
                out_next_i = encode((i + 2 > phi->num_pairs ? 1 : i + 2), phi->id + phi->bits / 2, phi->bits / 2 - 1);

                in_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id, phi->bits / 2 - 1);
                out_prev_i = encode((i == 0 ? phi->num_pairs : i), phi->id + phi->bits / 2, phi->bits / 2 - 1);

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

            components.push_back(monitaal::TAwithBDDEdges("seq_out_" + std::to_string(phi->id), clocks, locations, bdd_edges, name_id_map.at("t_" + std::to_string(0))));
            locations.clear();
            name_id_map.clear();
            bdd_edges.clear();


            /***** Until
             [l, infty) or (l, infty)
             *****/

            clocks.insert({1, "x1"});

            name = "TA_" + std::to_string(phi->id) + "_" + std::to_string(phi->num_pairs);
            in_i = bdd_ithvar(phi->id + phi->bits / 2 - 1);
            out_i = bdd_ithvar(phi->id + phi->bits  - 1);

            if (out_format.has_value() && out_format.value() && !out_flatten) {

                out_str << std::endl << std::endl;
                out_str << "# " << "TA_" << phi->id << "_" << phi->num_pairs << std::endl;
                // out_str << "# " << const_cast<MitlParser::AtomUContext*>(phi)->getText() << std::endl;
                out_str << "process:" << name << std::endl;

            }

            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));
  //          locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));
            // locations.push_back(monitaal::location_t(false, 3, "s3", empty_invariant));

            name_id_map.insert({"0", 0});
            name_id_map.insert({"1", 1});
 //           name_id_map.insert({"2", 2});
            // name_id_map.insert({"3", 3});

            if (out_format.has_value() && out_format.value() && !out_flatten) {

                out_str << "location:" << "TA_" << phi->id << "_" << phi->num_pairs << ":ell_0{initial: : labels: accept_" << phi->id << "_" << phi->num_pairs << "}" << std::endl;
                out_str << "location:" << "TA_" << phi->id << "_" << phi->num_pairs << ":ell_1{labels: accept_" << phi->id << "_" << phi->num_pairs << "}" << std::endl;
                // out_str << "location:" << "TA_" << phi->id << "_" << phi->num_pairs << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id) + "_" + std::to_string(phi->num_pairs)) << "}" << std::endl;
                // out_str << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

            }

            // 0 -> 0, !r && *p && *q, x := 0

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "0", "0", std::string{}, std::string{}, 1, !in_i & phi->atom(0)->star & phi->atom(1)->star & !out_i);

            // 0 -> 1, r && *p && *q, x := 0

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "0", "1", std::string{}, std::string{}, 1, in_i & phi->atom(0)->star & phi->atom(1)->star & !out_i);

            // 1 -> 1, !r && ^p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "1", std::string{}, std::string{}, 0, !in_i & phi->atom(0)->hat & phi->atom(1)->star & !out_i);

            // 1 -> 1, r && ^p && *q, x := 0

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "1", std::string{}, std::string{}, 1, in_i & phi->atom(0)->hat & phi->atom(1)->star & !out_i);

            // 1 -> 1, r && ^p && *q, x := 0, x < a

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, in_i & phi->atom(0)->hat & phi->atom(1)->star & !out_i);

            // 1 -> 1, r && *p && ^q, x := 0, x >= a

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, in_i & phi->atom(0)->star & phi->atom(1)->hat & out_i);

            // 1 -> 0, !r && *p && ^q, x := 0, x >= a

            build_edge(bdd_edges, name_id_map, out_str, phi->id, phi->num_pairs, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, !in_i & phi->atom(0)->star & phi->atom(1)->hat & out_i);


            components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
            locations.clear();
            name_id_map.clear();
            bdd_edges.clear();

            if (out_flatten) {
                components.push_back(varphi);
                return { { monitaal::TAwithBDDEdges::intersection(components) }, out_str.str() };
            } else {
                return { components, out_str.str() };
            }

        }

        assert(false);

    }

} // namespace mightypplcpp
