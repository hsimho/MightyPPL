#include "MightyPPL.h"

namespace mightylcpp {

    size_t num_all_props;

    std::vector<std::string> sat_paths;

    std::vector<std::string> get_letters(const std::string& pattern) {

        std::vector<std::string> output;

        std::set<int> dont_cares;

        for (auto i = 0; i < pattern.size(); ++i) {
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
    //     for (const auto& p : sat_paths) {
    //         for (const auto& s : get_letters(p)) {
    //             ++i;
    //         }
    //     }
    //     return i;
    // }

    void allsat_print_handler(char* varset, int size) {

        std::string output;

        // std::cout << std::setw(12);     // only affects the first char below, actually
        for (int v = 0; v < size; ++v) {
            output += (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
          //  std::cout << (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
        }
        // std::cout << std::endl;
        // std::cout << std::setw(0);

        sat_paths.push_back(output);

    }


    void build_edge(monitaal::bdd_edges_t& bdd_edges, const std::map<std::string, monitaal::location_id_t> name_id_map, const std::string& source, const std::string& target, const std::string& guard, bool reset, bdd label) {

        monitaal::constraints_t guard_constraints_t;
        monitaal::clocks_t reset_clocks_t;

        if (reset) {
            reset_clocks_t.push_back(1);
        }

        if (!guard.empty()) {

            std::stringstream ss(guard);
            std::string comp_operator;
            std::string constant;
            ss >> comp_operator;
            ss >> constant;

            // std::cout << "comp_operator: " << comp_operator << std::endl;
            // std::cout << "constant: " << constant << std::endl;

            // TODO: maybe some error processing

            int c = std::stoi(constant);

            if (comp_operator == "<=") {

                guard_constraints_t.push_back(monitaal::constraint_t::upper_non_strict(1, c));

            } else if (comp_operator == "<") {

                guard_constraints_t.push_back(monitaal::constraint_t::upper_strict(1, c));

            } else if (comp_operator == ">=") {

                guard_constraints_t.push_back(monitaal::constraint_t::lower_non_strict(1, c));

            } else { // comp_operator == ">"

                guard_constraints_t.push_back(monitaal::constraint_t::lower_strict(1, c));

            }

        }

        bdd_edges.push_back(monitaal::bdd_edge_t(name_id_map.at(source), name_id_map.at(target), guard_constraints_t, reset_clocks_t, label));

        return;

    }
    

    std::vector<monitaal::TAwithBDDEdges> build_ta_from_atom(const MitlParser::AtomContext* phi_) {


        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        if (phi_->type == FINALLY) {

            MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                // Finally
                // "untimed" case

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});
                name_id_map.insert({"2", 2});

                monitaal::bdd_edges_t bdd_edges;

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_true() & phi->atom()->tilde);

                // 2 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, false, bdd_true() & phi->atom()->tilde);

                // 1 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 2 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 1 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 2 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, "2", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    // Finally
                    // [0, u) or [0, u]

                    if (right->children[0]->getText() == "infty") {
                        assert(("Please remove superflous interval [0, infty)", false));
                    }

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});

                    monitaal::bdd_edges_t bdd_edges;

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_true() & phi->atom()->tilde);

                    // 1 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);


                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    // Finally
                    // [l, infty) or (l, infty)

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));
                    // locations.push_back(monitaal::location_t(false, 3, "s3", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});
                    // name_id_map.insert({"3", 3});

                    monitaal::bdd_edges_t bdd_edges;

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);                    

                    // 1 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // // 1 -> 3, r && ^p && ~q, x := 0

                    // build_edge(bdd_edges, name_id_map, "1", "3", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // // 1 -> 3, r && ^p && *q, x := 0, x < a

                    // build_edge(bdd_edges, name_id_map, "1", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 3 -> 3, ^p && ~q

                    // build_edge(bdd_edges, name_id_map, "3", "3", std::string{}, false, bdd_true() & phi->atom()->tilde);

                    // // 3 -> 3, ^p && *q, x < a

                    // build_edge(bdd_edges, name_id_map, "3", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_true() & phi->atom()->star);

                    // // 3 -> 2, ^p && ^q, x := 0, x >= a

                    // build_edge(bdd_edges, name_id_map, "3", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_true() & phi->atom()->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }


            }

        } else if (phi_->type == ONCE) {

            MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

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

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_true() & phi->atom()->tilde);

                // 1 -> 1, r && *p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 0 -> 1, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


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

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a 

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

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

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ^p && *q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 2 -> 1, !r && ^p && *q

                    // build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 1 -> 2, !r && ^p && *q

                    // build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, !r && ^p && *q

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ~q, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, r && ^p && ~q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // // 1 -> 2, r && ^p && ~q, x >= a

                    // build_edge(bdd_edges, name_id_map, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 2, r && ^p && ~q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 1, r && *p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }


        } else if (phi_->type == GLOBALLY) {

            MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                // Globally
                // "untimed" case

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});

                monitaal::bdd_edges_t bdd_edges;

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 1 -> 0, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    // Globally
                    // [0, u) or [0, u]

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

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);


                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    // Globally
                    // [l, infty) or (l, infty)


                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});

                    monitaal::bdd_edges_t bdd_edges;

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 1, r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 0, !r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == HISTORICALLY) {

            MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                /***** Historically
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

                monitaal::bdd_edges_t bdd_edges;

                // 0i -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0i -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0i -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                // 0i -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 0i -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 0 -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 2) };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Historically
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

                    monitaal::bdd_edges_t bdd_edges;

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0i -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);
                    
                    // 0i -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);                    

                    // 0i -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0i -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);


                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);
                    
                    // 2 -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0 -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) };   // last arg: initial location id


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Historically
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

                    monitaal::bdd_edges_t bdd_edges;

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                    // 0i -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_true() & phi->atom()->star);

                    // 0i -> 2, !r && ~p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, !r && ~p && *q

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0i -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);


                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                    // 2 -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_true() & phi->atom()->star);
                    
                    // 1 -> 2, !r && ~p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, !r && ~p && *q

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0 -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) };   // last arg: initial location id

                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }


        } else if (phi_->type == UNTIL) {

            MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                // Until
                // "untimed" case

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});
                name_id_map.insert({"2", 2});

                monitaal::bdd_edges_t bdd_edges;

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 2 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 1 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 2 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 1 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 2 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, "2", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    // Until
                    // [0, u) or [0, u]

                    if (right->children[0]->getText() == "infty") {
                        assert(("Please remove superflous interval [0, infty)", false));
                    }

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});

                    monitaal::bdd_edges_t bdd_edges;

                    // 0 -> 0, , x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);


                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    // Until
                    // [l, infty) or (l, infty)

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(true, 2, "s2", empty_invariant));
                    // locations.push_back(monitaal::location_t(false, 3, "s3", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});
                    // name_id_map.insert({"3", 3});

                    monitaal::bdd_edges_t bdd_edges;

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // // 1 -> 3, r && ^p && ~q, x := 0

                    // build_edge(bdd_edges, name_id_map, "1", "3", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 1 -> 3, r && ^p && *q, x := 0, x < a

                    // build_edge(bdd_edges, name_id_map, "1", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // // 3 -> 3, ^p && ~q

                    // build_edge(bdd_edges, name_id_map, "3", "3", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 3 -> 3, ^p && *q, x < a

                    // build_edge(bdd_edges, name_id_map, "3", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->star);

                    // // 3 -> 2, ^p && ^q, x := 0, x >= a

                    // build_edge(bdd_edges, name_id_map, "3", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, phi->atom(0)->hat & phi->atom(1)->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }


        } else if (phi_->type == SINCE) {

            MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                /***** Since
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

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 1 -> 1, r && *p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 0 -> 1, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Since
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

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a 

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Since
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

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ^p && *q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // // 2 -> 1, !r && ^p && *q

                    // build_edge(bdd_edges, name_id_map, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 1 -> 2, !r && ^p && *q

                    // build_edge(bdd_edges, name_id_map, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, !r && ^p && *q

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ~q, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, r && ^p && ~q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 1 -> 2, r && ^p && ~q, x >= a

                    // build_edge(bdd_edges, name_id_map, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 2, r && ^p && ~q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 1, r && *p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }


        } else if (phi_->type == RELEASE) {

            MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                // Release
                // "untimed" case

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});

                monitaal::bdd_edges_t bdd_edges;

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 1 -> 0, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    // Release
                    // [0, u) or [0, u]

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

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 1, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);


                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id



                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    // Release
                    // [l, infty) or (l, infty)


                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});

                    monitaal::bdd_edges_t bdd_edges;

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) };   // last arg: initial location id

                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == TRIGGER) {

            MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

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

                monitaal::bdd_edges_t bdd_edges;

                // 0i -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 0i -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0i -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0 -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 2) };   // last arg: initial location id


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

                    monitaal::bdd_edges_t bdd_edges;

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);
                    
                    // 0i -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);


                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);
                    
                    // 2 -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 2 -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0 -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) };   // last arg: initial location id

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

                    monitaal::bdd_edges_t bdd_edges;

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, phi->atom(0)->tilde & phi->atom(1)->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0i -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "2", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0i", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);



                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, "1", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 2 -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0 -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    return { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) };   // last arg: initial location id

                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == PNUELIFN) {

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

                        locations.push_back(monitaal::location_t(true, 1 + phi->atoms.size(), "s2", empty_invariant));
                        name_id_map.insert({"2", 1 + phi->atoms.size()});

                        // 0 -> 0, !r, x := 0

                        build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 0 -> 1_0, r, x := 0

                        build_edge(bdd_edges, name_id_map, "0", "1_0", std::string{}, true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ~p_j

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            // 1_j -> 1_j+1, !r && ^p_j (x := 0, x <= a)

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? "0" : "1_" + std::to_string(j + 1)), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), (j + 1 == phi->atoms.size() ? true : false), !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_j -> 1_0, r && *p_j (r && ~p_j, x <= a)

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_0", (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), false, bdd_ithvar(phi->id + i) & (j + 1 == phi->atoms.size() ? phi->atoms[phi->atoms.size() - 1]->tilde : phi->atoms[j]->star));

                        }

                        // 1_n-1 -> 2, r && ^p_j, x := 0, x <= a

                        build_edge(bdd_edges, name_id_map, "1_" + std::to_string(phi->atoms.size() - 1), "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[phi->atoms.size() - 1]->hat);

                        // 2 -> 1_0, !r && ~p_0

                        build_edge(bdd_edges, name_id_map, "2", "1_0", std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[0]->tilde);

                        // 2 -> 1_1, !r && ^p_0

                        build_edge(bdd_edges, name_id_map, "2", "1_1", std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[0]->hat);

                        // 2 -> 1_0, r

                        build_edge(bdd_edges, name_id_map, "2", "1_0", std::string{}, false, bdd_ithvar(phi->id + i));


                        components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
                        locations.clear();
                        name_id_map.clear();
                        bdd_edges.clear();

                    }

                    return components;

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else if (phi_->type == PNUELION) {

            MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Pnueli On
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
                            locations.push_back(monitaal::location_t(true, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                            name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                        }

                        // 0 -> 0, !r, x := 0

                        build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 1_0 -> 0, r, x := 0, x <= a

                        build_edge(bdd_edges, name_id_map, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ~p_j

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            // 1_j+1 -> 1_j, !r && ^p_j (x := 0)

                            build_edge(bdd_edges, name_id_map, (j + 1 == phi->atoms.size() ? "0" : "1_" + std::to_string(j + 1)), "1_" + std::to_string(j), std::string{}, (j + 1 == phi->atoms.size() ? true : false), !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_0 -> 1_j, r && *p_j (r && ~p_j)

                            build_edge(bdd_edges, name_id_map, "1_0", "1_" + std::to_string(j), std::string{}, false, bdd_ithvar(phi->id + i) & (j + 1 == phi->atoms.size() ? phi->atoms[phi->atoms.size() - 1]->tilde : phi->atoms[j]->star));

                        }

                        // 1_0 -> 1_n-1, r && ^p_j, x := 0, x <= a

                        build_edge(bdd_edges, name_id_map, "1_0", "1_" + std::to_string(phi->atoms.size() - 1), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[phi->atoms.size() - 1]->hat);

                        components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
                        locations.clear();
                        name_id_map.clear();
                        bdd_edges.clear();

                    }

                    return components;

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else if (phi_->type == PNUELIFNDUAL) {


            MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (phi->interval() == nullptr) {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Pnueli Fn Dual
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
                            locations.push_back(monitaal::location_t(true, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                            name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                        }

                        // 0 -> 0, !r, x := 0

                        build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 0 -> 1_0, r, x := 0

                        build_edge(bdd_edges, name_id_map, "0", "1_0", std::string{}, true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ^p_j

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_j -> 1_j, r && ^p_j, x := 0

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, true, bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            if (j + 1 != phi->atoms.size()) {

                                // 1_j -> 1_j+1, !r && ~p_j

                                build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                                // 1_j -> 1_j+1, r && ~p_j, x := 0

                                build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), std::string{}, true, bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            }

                            // 1_j -> 0, !r && *p_j, x := 0, x > a

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id + i) & phi->atoms[j]->star);

                            // 1_j -> 1_0, r && *p_j, x := 0, x > a

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[j]->star);


                        }

                        components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0));
                        locations.clear();
                        name_id_map.clear();
                        bdd_edges.clear();

                    }

                    return components;

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else if (phi_->type == PNUELIONDUAL) {


            MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)phi_;

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


                        // 0i -> 0, !r, x := 0

                        build_edge(bdd_edges, name_id_map, "0i", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 0i -> 0, r, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 0i -> 1_j, !r && ^p_j

                            build_edge(bdd_edges, name_id_map, "0i", "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 0i -> 1_j, r && ^p_j, x > a

                            build_edge(bdd_edges, name_id_map, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            if (j + 1 != phi->atoms.size()) {

                                // 0i -> 1_j, !r && ~p_j

                                build_edge(bdd_edges, name_id_map, "0i", "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                                // 0i -> 1_j, r && ~p_j, x > a

                                build_edge(bdd_edges, name_id_map, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            }

                            // 0i -> 1_j, !r && *p_j, x := 0

                            build_edge(bdd_edges, name_id_map, "0i", "1_" + std::to_string(j), std::string{}, true, !bdd_ithvar(phi->id + i) & phi->atoms[j]->star);

                            // 0i -> 1_j, r && *p_j, x := 0, x > a

                            build_edge(bdd_edges, name_id_map, "0i", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[j]->star);
                        }



                        // 0 -> 0, !r, x := 0

                        build_edge(bdd_edges, name_id_map, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 1_0 -> 0, r, x := 0, x > a

                        build_edge(bdd_edges, name_id_map, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ^p_j

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_j -> 1_j, r && ^p_j, x > a

                            build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            if (j + 1 != phi->atoms.size()) {

                                // 1_j+1 -> 1_j, !r && ~p_j

                                build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                                // 1_j+1 -> 1_j, r && ~p_j, x > a

                                build_edge(bdd_edges, name_id_map, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            }

                            // 0 -> 1_j, !r && *p_j, x := 0

                            build_edge(bdd_edges, name_id_map, "0", "1_" + std::to_string(j), std::string{}, true, !bdd_ithvar(phi->id + i) & phi->atoms[j]->star);

                            // 1_0 -> 1_j, r && *p_j, x := 0, x > a

                            build_edge(bdd_edges, name_id_map, "1_0", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[j]->star);
                        }

                        components.push_back(monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 1 + phi->atoms.size()));
                        locations.clear();
                        name_id_map.clear();
                        bdd_edges.clear();

                    }

                    return components;

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else {

            assert(("Unsupported atom type", false));

        }

        assert(false);

    }

    monitaal::TA build_ta_from_main(MitlParser::MainContext* phi_) {

        std::string nnf_in = phi_->getText();

        std::cout << "\nInput formula (getText() on the parse tree):\n" << std::endl;

        std::cout << nnf_in << std::endl;

        MitlCheckNNFVisitor check_nnf_visitor;
//        check_nnf_visitor.loadParser(parser);      // This is for ruleNames[]

        if (!std::any_cast<bool>(check_nnf_visitor.visitMain(phi_))) {

            std::cout << "\nThe input formula is not in negation normal form (i.e. AND and OR only, and all negations appear just before letters).\n";

            std::cout << "\nRewriting into NNF...\n";

            MitlToNNFVisitor to_nnf_visitor;

            nnf_in = std::any_cast<std::string>(to_nnf_visitor.visitMain(phi_));


        } else {

            MitlToNNFVisitor to_nnf_visitor;

            nnf_in = std::any_cast<std::string>(to_nnf_visitor.visitMain(phi_));

        }

        std::cout << "\nInput formula (in NNF):\n";

        std::cout << nnf_in;

        std::cout << "\nRe-parsing...\n";

        antlr4::ANTLRInputStream nnf_input = antlr4::ANTLRInputStream(nnf_in);

        MitlLexer nnf_lexer(&nnf_input);
        antlr4::CommonTokenStream nnf_tokens(&nnf_lexer);

        MitlParser nnf_parser(&nnf_tokens);

        MitlParser::MainContext* nnf_formula = nnf_parser.main();

        assert(std::any_cast<bool>(check_nnf_visitor.visitMain(nnf_formula)));

        std::cout << "\n<<<<<< Numbering temporal subformulae... >>>>>>\n\n";

        // Numbering temporal atoms

        MitlAtomNumberingVisitor temporal_numbering_visitor;

        num_all_props = std::any_cast<int>(temporal_numbering_visitor.visitMain(nnf_formula));

        assert(("There must be at least one temporal subformula", num_all_props - nnf_formula->props.size()));

        std::cout << "\n# of atomic propositions:\n";

        std::cout << nnf_formula->props.size() << std::endl;

        std::cout << "\n# of temporal subformulae:\n";

        std::cout << num_all_props - nnf_formula->props.size() << std::endl;


        std::set<MitlParser::AtomContext*, atom_cmp> temporal_atoms;
        MitlCollectTemporalVisitor collect_temporal_visitor;
        temporal_atoms = std::any_cast<decltype(temporal_atoms)>(collect_temporal_visitor.visitMain(nnf_formula));

        // std::cout << "\nCollected: " << temporal_atoms.size() << std::endl;

        {   // re-numbering

            int i = num_all_props - nnf_formula->props.size();
            for (const auto & [k, v] : nnf_formula->props) {
                nnf_formula->props[k] = ++i;
            }

            i = 0;
            for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

                if ((*it)->type == PNUELIFN) {
                    (*it)->id = ++i;
                    i = i + ((MitlParser::AtomFnContext*)(*it))->atoms.size() - 1;
                } else if ((*it)->type == PNUELION) {
                    (*it)->id = ++i;
                    i = i + ((MitlParser::AtomOnContext*)(*it))->atoms.size() - 1;
                } else if ((*it)->type == PNUELIFNDUAL) {
                    (*it)->id = ++i;
                    i = i + ((MitlParser::AtomFnDualContext*)(*it))->atoms.size() - 1;
                } else if ((*it)->type == PNUELIONDUAL) {
                    (*it)->id = ++i;
                    i = i + ((MitlParser::AtomOnDualContext*)(*it))->atoms.size() - 1;
                } else {
                    (*it)->id = ++i;
                }

            }
        }


        std::cout << "\nList of them:\n";

        for (const auto & [k, v] : nnf_formula->props) {
            std::cout << v << ": " << k << std::endl;
        }

        for (auto it = temporal_atoms.rbegin(); it != temporal_atoms.rend(); ++it) {

            std::cout << (*it)->id << ": " << (*it)->getText() << std::endl;

        }

        std::cout << "\nGenerating BDDs for labels...\n";

        bdd_setvarnum(num_all_props + 1);   // We leave the 0-th variable unused (temporal atoms and props start from 1)

        MitlGetBDDVisitor get_bdd_visitor;
        get_bdd_visitor.visitMain(nnf_formula);


        for (auto it = temporal_atoms.rbegin(); it != temporal_atoms.rend(); ++it) {

            std::cout << "\n" << "TA_" << (*it)->id << ": " << (*it)->getText() << "\n" << std::endl;

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

            } else if ((*it)->type == ONCE) {

                std::cout << "\nphi:\n";

                std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomOContext*)(*it))->atom()->overline, *allsat_print_handler);
                std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomOContext*)(*it))->atom()->star, *allsat_print_handler);
                std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomOContext*)(*it))->atom()->tilde, *allsat_print_handler);
                std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomOContext*)(*it))->atom()->hat, *allsat_print_handler);

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

            } else if ((*it)->type == HISTORICALLY) {

                std::cout << "\nphi:\n";

                std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomHContext*)(*it))->atom()->overline, *allsat_print_handler);
                std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomHContext*)(*it))->atom()->star, *allsat_print_handler);
                std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomHContext*)(*it))->atom()->tilde, *allsat_print_handler);
                std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomHContext*)(*it))->atom()->hat, *allsat_print_handler);

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

            } else if ((*it)->type == SINCE) {

                std::cout << "\nphi_1:\n";

                std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(0)->overline, *allsat_print_handler);
                std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(0)->star, *allsat_print_handler);
                std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(0)->tilde, *allsat_print_handler);
                std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(0)->hat, *allsat_print_handler);

                std::cout << "\nphi_2:\n";

                std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(1)->overline, *allsat_print_handler);
                std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(1)->star, *allsat_print_handler);
                std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(1)->tilde, *allsat_print_handler);
                std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomSContext*)(*it))->atom(1)->hat, *allsat_print_handler);

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

            } else if ((*it)->type == TRIGGER) {

                std::cout << "\nphi_1:\n";

                std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(0)->overline, *allsat_print_handler);
                std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(0)->star, *allsat_print_handler);
                std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(0)->tilde, *allsat_print_handler);
                std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(0)->hat, *allsat_print_handler);

                std::cout << "\nphi_2:\n";

                std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(1)->overline, *allsat_print_handler);
                std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(1)->star, *allsat_print_handler);
                std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(1)->tilde, *allsat_print_handler);
                std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                bdd_allsat(((MitlParser::AtomTContext*)(*it))->atom(1)->hat, *allsat_print_handler);

            } else if ((*it)->type == PNUELIFN) {

                for (auto i = 0; i < ((MitlParser::AtomFnContext*)(*it))->atoms.size(); ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnContext*)(*it))->atoms[i]->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnContext*)(*it))->atoms[i]->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnContext*)(*it))->atoms[i]->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnContext*)(*it))->atoms[i]->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == PNUELION) {

                for (auto i = 0; i < ((MitlParser::AtomOnContext*)(*it))->atoms.size(); ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnContext*)(*it))->atoms[i]->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnContext*)(*it))->atoms[i]->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnContext*)(*it))->atoms[i]->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnContext*)(*it))->atoms[i]->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == PNUELIFNDUAL) {

                for (auto i = 0; i < ((MitlParser::AtomFnDualContext*)(*it))->atoms.size(); ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnDualContext*)(*it))->atoms[i]->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnDualContext*)(*it))->atoms[i]->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnDualContext*)(*it))->atoms[i]->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomFnDualContext*)(*it))->atoms[i]->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == PNUELIONDUAL) {

                for (auto i = 0; i < ((MitlParser::AtomOnDualContext*)(*it))->atoms.size(); ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnDualContext*)(*it))->atoms[i]->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnDualContext*)(*it))->atoms[i]->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnDualContext*)(*it))->atoms[i]->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomOnDualContext*)(*it))->atoms[i]->hat, *allsat_print_handler);

                }

            } else {
                assert(false);
            }

        }

        std::cout << "\n" << "TA_0" << ": " << nnf_formula->formula()->getText() << "\n" << std::endl;

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

        // auto div = monitaal::TA::time_divergence_ta(get_letters(std::string(num_all_props + 1, 'X')), true);

        auto div = monitaal::TAwithBDDEdges::time_divergence_ta(bdd_true());

        std::vector<monitaal::TAwithBDDEdges> temporal_components;
        std::vector<monitaal::TAwithBDDEdges> generated_components;

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIFNDUAL || (*it)->type == PNUELIONDUAL) {
                std::cout << "\nGenerating TA_" << (*it)->id << " (and other sub-components)...\n";
            } else {
                std::cout << "\nGenerating TA_" << (*it)->id << "...\n";
            }
            generated_components = build_ta_from_atom(*it);
            temporal_components.insert(temporal_components.end(), generated_components.begin(), generated_components.end());

            std::cout << std::endl;

            std::cout << std::setw(20) << "# of locations: " << std::setw(10) << temporal_components.back().locations().size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << temporal_components.back().number_of_clocks() << std::setw(0) << std::endl;

            // for (auto i = 0; i < temporal_components.back().number_of_clocks(); ++i) {
            //     std::cout << std::setw(20) << temporal_components.back().clock_name(i) << std::setw(0) << std::endl;
            // }

            std::cout << std::endl;

            for (const auto & [k, v] : temporal_components.back().locations()) {

                std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
                std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << temporal_components.back().bdd_edges_from(k).size() << std::setw(0) << std::endl;
                // for (const auto& e : temporal_components.back().bdd_edges_from(k)) {
                //     std::cout << e.from() << " -> " << e.to() << ": " << std::endl;
                //     bdd_printset(e.bdd_label());
                //     std::cout << std::endl;
                // }
                std::cout << std::setw(20) << "# incoming: " << std::setw(10) << temporal_components.back().bdd_edges_to(k).size() << std::setw(0) << std::endl;

            }

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

        monitaal::bdd_edges_t bdd_edges;
        monitaal::constraints_t guard;
        monitaal::clocks_t reset;

        // 0 -> 1, varphi

        label = nnf_formula->hat;

        bdd_edges.push_back(monitaal::bdd_edge_t(0, 1, guard, reset, label));

        // 1 -> 1, *varphi

        label = nnf_formula->star;

        bdd_edges.push_back(monitaal::bdd_edge_t(1, 1, guard, reset, label));

        auto varphi = monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0);   // last arg: initial location id
        clocks.clear();
        locations.clear();
        bdd_edges.clear();

        std::cout << std::endl;

        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << varphi.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << varphi.number_of_clocks() << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto & [k, v] : varphi.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << varphi.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << varphi.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }

        std::cout << "\nGenerating M" << "...\n";

        name = "M";

        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));

        // 0 -> 0, true

        label = bdd_true();

        bdd_edges.push_back(monitaal::bdd_edge_t(0, 0, guard, reset, label));

        auto model = monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0);   // last arg: initial location id
        clocks.clear();
        locations.clear();
        bdd_edges.clear();

        std::cout << std::endl;

        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << model.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << model.number_of_clocks() << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto & [k, v] : model.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << model.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << model.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }


        std::cout << "\n<<<<<< Taking intersection... >>>>>>\n\n";

        std::vector<monitaal::TAwithBDDEdges> automata = temporal_components;
        automata.insert(automata.begin(), varphi);
        automata.insert(automata.begin(), div);
        automata.insert(automata.begin(), model);

        monitaal::TAwithBDDEdges product = monitaal::TAwithBDDEdges::intersection(automata);

        // std::cout << product;
        
        std::cout << std::endl;

        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << product.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << product.number_of_clocks() << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto & [k, v] : product.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << product.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            // for (const auto& e : product.bdd_edges_from(k)) {
            //     std::cout << e.from() << " -> " << e.to() << ": " << std::endl;
            //     bdd_printset(e.bdd_label());
            //     std::cout << std::endl;
            // }
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << product.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }

        // return product;

        std::set<int> props_to_remove;
        {
            int i = 0;
            for (const auto& x : temporal_atoms) {

                props_to_remove.insert(++i);

            }
        }

        return product.projection(props_to_remove);

    }

} // namespace mightylcpp
