#include "MightyPPL.h"

namespace mightypplcpp {


    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_once(std::stringstream& out_str, monitaal::clock_map_t& clocks, const MitlParser::AtomContext* phi_) {

        MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)phi_;

        std::string name = "TA_" + std::to_string(phi->id);


        if (out_format.has_value()) {

            if (out_format.value()) {

                out_str << std::endl << std::endl;
                out_str << "# " << "TA_" << phi->id << std::endl;
                out_str << "# " << const_cast<MitlParser::AtomOContext*>(phi)->getText() << std::endl;
                out_str << "process:" << name << std::endl;

            }

        }


        if (phi->interval() == nullptr) {

            /***** Once
             "untimed" case
            *****/

            monitaal::constraints_t empty_invariant;
            monitaal::locations_t locations;

            locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
            locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

            std::map<std::string, monitaal::location_id_t> name_id_map;
            name_id_map.insert({"0", 0});
            name_id_map.insert({"1", 1});

            monitaal::bdd_edges_t bdd_edges;

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                }

            }

            // 0 -> 0, !r && *p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

            // 1 -> 0, r && *p && *q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

            // 1 -> 1, ^p && ~q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->tilde);

            // 1 -> 1, r && *p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

            // 0 -> 1, !r && *p && ^q

            build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

            return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

            if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                /***** Once
                 [0, u) or [0, u]
                *****/

                if (right->children[0]->getText() == "infty") {
                    assert(("Please remove superflous interval [0, infty)", false));
                }

                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});

                monitaal::bdd_edges_t bdd_edges;

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    }

                }

                // 0 -> 0, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 0, r && *p && *q, x := 0, x <= a 

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ^p && ~q, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_true() & phi->atom()->tilde);

                // 1 -> 1, r && *p && ^q, x := 0, x <= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 0 -> 1, !r && *p && ^q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

            } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                /***** Once
                 [l, infty) or (l, infty)
                *****/

                clocks.insert({1, "x1"});

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});
                name_id_map.insert({"2", 2});

                monitaal::bdd_edges_t bdd_edges;

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;
                        // out_str << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                    }

                }

                // 0 -> 0, !r && *p && *q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 0, r && *p && *q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 2 -> 0, r && *p && *q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, !r && ^p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // // 2 -> 1, !r && ^p && *q

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // // 1 -> 2, !r && ^p && *q

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 2 -> 2, !r && ^p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, r && ^p && ~q, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                // 2 -> 1, r && ^p && ~q, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                // // 1 -> 2, r && ^p && ~q, x >= a

                // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                // 2 -> 2, r && ^p && ~q, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                // 1 -> 2, r && ^p && *q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 2 -> 2, r && ^p && *q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, r && *p && ^q, x := 0, x >= a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 2 -> 1, r && *p && ^q, x := 0, x < a

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 0 -> 1, !r && *p && ^q, x := 0

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

            } else {

                assert(("Unexpected <l, u> on F O G H U S R T?", false));

            }

        }

        assert(false);

    }

} // namespace mightypplcpp
