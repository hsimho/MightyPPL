#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_trigger(const MitlParser::AtomContext* phi_) {

        std::stringstream out_str;

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)phi_;

        std::string name = "TA_" + std::to_string(phi->id) + "_" + std::to_string(0);

        if (out_format.has_value() && out_format.value() && !out_flatten) {

            out_str << std::endl << std::endl;
            out_str << "# " << "TA_" << phi->id << "_" << 0 << std::endl;
            out_str << "# " << const_cast<MitlParser::AtomTContext*>(phi)->getText() << std::endl;
            out_str << "process:" << name << std::endl;

        }

        if (phi->interval() == nullptr) {

            /***** Trigger
             "untimed" case
            *****/

            monitaal::constraints_t empty_invariant;
            monitaal::locations_t locations;

            // The current version of MoniTAal only allows 1 initial location

            locations.push_back(monitaal::location_t(false, 2, "s0i", empty_invariant));
            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

            std::map<std::string, monitaal::location_id_t> name_id_map;
            name_id_map.insert({"0i", 2});
            name_id_map.insert({"0", 0});
            name_id_map.insert({"1", 1});

            assert(!acc_map.count(name));
            acc_map.insert({ name, { {0, 1}, {0, 1} } });

            monitaal::bdd_edges_t bdd_edges;

            if (out_format.has_value() && !out_flatten) {

                if (out_format.value()) {

                    out_str << "location:" << name << ":ell_" << name_id_map.at("0i") << "{initial: }" << std::endl;
                    out_str << "location:" << name << ":ell_" << name_id_map.at("0") << "{labels: accept_" << phi->id << "}" << std::endl;
                    out_str << "location:" << name << ":ell_" << name_id_map.at("1") << "{labels: accept_" << phi->id << "}" << std::endl;

                } else {

                    out_str << "\t<template>" << std::endl;
                    out_str << "\t\t<name>" << name << "</name>" << std::endl;

                    out_str << "\t\t<declaration>" << std::endl;


                    out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("0i") << ";" << std::endl;


                    out_str << "\t\t</declaration>" << std::endl;

                    out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                    out_str << "\t\t</location>" << std::endl;
                    out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                }

            }

            // 0i -> 0, !r && *p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

            // 0i -> 0, r && *p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

            // 0i -> 1, ~p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

            // 0i -> 1, r && ^p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

            // 0i -> 1, !r && ^p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


            // 0 -> 0, !r && *p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

            // 1 -> 0, r && *p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

            // 1 -> 1, ~p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

            // 1 -> 1, r && ^p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

            // 0 -> 1, !r && ^p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

            if (out_format.has_value() && !out_format.value() && !out_flatten) {
                out_str << "\t</template>" << std::endl << std::endl;
            }
            ++components_counter;
            return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 2) }, out_str.str() };   // last arg: initial location id


        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                /***** Trigger
                 [0, u) or [0, u]
                *****/

                if (right->children[0]->getText() == "infty") {
                    assert(("Please remove superflous interval [0, infty)", false));
                }

                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(false, 3, "s0i", empty_invariant));
                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0i", 3});
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});
                name_id_map.insert({"2", 2});

                assert(!acc_map.count(name));
                acc_map.insert({ name, { {0, 1, 2}, {0, 1, 2} } });

                monitaal::bdd_edges_t bdd_edges;

                if (out_format.has_value() && !out_flatten) {

                    if (out_format.value()) {

                        out_str << "location:" << name << ":ell_" << name_id_map.at("0i") << "{initial: }" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("0") << "{labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("1") << "{labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("2") << "{labels: accept_" << phi->id << "}" << std::endl;

                    } else {

                        out_str << "\t<template>" << std::endl;
                        out_str << "\t\t<name>" << name << "</name>" << std::endl;

                        out_str << "\t\t<declaration>" << std::endl;


                        out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("0i") << ";" << std::endl;


                        out_str << "\t\t</declaration>" << std::endl;

                        out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                        out_str << "\t\t</location>" << std::endl;
                        out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                    }

                }

                // 0i -> 0, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 0, r && *p && *q, x := 0, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 0, r && *p && *q, x := 0, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 2, r && ~p && ^q, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                // 0i -> 1, ~p && ^q, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);
                
                // 0i -> 2, r && ~p && ^q, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                // 0i -> 2, !r && ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                // 0i -> 1, r && ^p && ^q, x := 0, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0i -> 1, r && ^p && ^q, x := 0, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0i -> 2, r && *p && *q, x := 0, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 2, r && *p && *q, x := 0, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 1, !r && ^p && ^q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0i -> 2, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);


                // 0 -> 0, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q, x := 0, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 2 -> 0, r && *p && *q, x := 0, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 2, r && ~p && ^q, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, ~p && ^q, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);
                
                // 2 -> 2, r && ~p && ^q, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                // 2 -> 2, !r && ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, r && ^p && ^q, x := 0, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 2 -> 1, r && ^p && ^q, x := 0, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 1 -> 2, r && *p && *q, x := 0, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 2 -> 2, r && *p && *q, x := 0, x > a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0 -> 1, !r && ^p && ^q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0 -> 2, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                if (out_format.has_value() && !out_format.value() && !out_flatten) {
                    out_str << "\t</template>" << std::endl << std::endl;
                }
                ++components_counter;
                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) }, out_str.str() };   // last arg: initial location id


            } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                /***** Trigger
                 [l, infty) or (l, infty)
                *****/

                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(false, 3, "s0i", empty_invariant));
                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0i", 3});
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});
                name_id_map.insert({"2", 2});

                assert(!acc_map.count(name));
                acc_map.insert({ name, { {0, 1, 2}, {0, 1, 2} } });

                monitaal::bdd_edges_t bdd_edges;

                if (out_format.has_value() && !out_flatten) {

                    if (out_format.value()) {

                        out_str << "location:" << name << ":ell_" << name_id_map.at("0i") << "{initial: }" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("0") << "{labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("1") << "{labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << name << ":ell_" << name_id_map.at("2") << "{labels: accept_" << phi->id << "}" << std::endl;

                    } else {

                        out_str << "\t<template>" << std::endl;
                        out_str << "\t\t<name>" << name << "</name>" << std::endl;

                        out_str << "\t\t<declaration>" << std::endl;


                        out_str << "\t\t\tint[0, " << name_id_map.size() - 1 << "] loc = " << name_id_map.at("0i") << ";" << std::endl;


                        out_str << "\t\t</declaration>" << std::endl;

                        out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                        out_str << "\t\t</location>" << std::endl;
                        out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                    }

                }

                // 0i -> 0, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 0, r && *p && *q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 0, r && *p && *q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 0i -> 2, ~p && *q, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->star);

                // 0i -> 2, ~p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", std::string{}, std::string{}, 1, phi->atom(0)->tilde & phi->atom(1)->star);

                // 0i -> 2, r && ^p && *q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                // 0i -> 2, r && ^p && *q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                // 0i -> 1, r && ^p && ^q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0i -> 1, r && ^p && ^q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0i -> 2, !r && ^p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                // 0i -> 1, !r && ^p && ^q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0i", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


                // 0 -> 0, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 2 -> 0, r && *p && *q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 2 -> 2, ~p && *q, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->star);

                // 1 -> 2, ~p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "2", std::string{}, std::string{}, 1, phi->atom(0)->tilde & phi->atom(1)->star);

                // 1 -> 2, r && ^p && *q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                // 2 -> 2, r && ^p && *q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 2 -> 1, r && ^p && ^q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0 -> 2, !r && ^p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                // 0 -> 1, !r && ^p && ^q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, 0, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                if (out_format.has_value() && !out_format.value() && !out_flatten) {
                    out_str << "\t</template>" << std::endl << std::endl;
                }
                ++components_counter;
                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) }, out_str.str() };   // last arg: initial location id

            } else {

                assert(("Unexpected <l, u> on F O G H U S R T?", false));

            }

        }

        assert(false);

    }

} // namespace mightypplcpp
