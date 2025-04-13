#include "MightyPPL.h"

namespace mightypplcpp {

    size_t num_all_props;

    bdd encode(const int i, const int offset, const int bits) {

        assert(i >= 0);
        assert(std::ceil(std::log2(i + 1)) <= bits);

        bdd ret = bdd_true();

        int j = 0;
        for (int k = i; k != 0; k = k / 2) {

            if (k % 2) {

                ret = ret & bdd_ithvar(offset + j++);

            } else {

                ret = ret & !bdd_ithvar(offset + j++);

            }

        }
        while (j < bits) {

                ret = ret & !bdd_ithvar(offset + j++);

        }
        return ret;
    }

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

        //std::cout << std::setw(12);     // only affects the first char below, actually
        for (int v = 0; v < size; ++v) {
            output += (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
          //  std::cout << (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
        }
        //std::cout << std::endl;
        //std::cout << std::setw(0);

        sat_paths.push_back(output);

    }


    void build_edge(monitaal::bdd_edges_t& bdd_edges, const std::map<std::string, monitaal::location_id_t> name_id_map, std::stringstream& out_s, const int id, const std::string& source, const std::string& target, const std::string& guard_x, const std::string& guard_y, int reset, bdd label) {

        monitaal::constraints_t guard_constraints;
        monitaal::clocks_t reset_clocks;

        if (reset == 1) {
            reset_clocks.push_back(1);
        } else if (reset == 2) {
            reset_clocks.push_back(2);
        } else if (reset == 3) {
            reset_clocks.push_back(1);
            reset_clocks.push_back(2);
        } else {
            assert(reset == 0);
        }


        if (!guard_x.empty()) {

            std::stringstream ss(guard_x);
            std::string comp_operator;
            std::string constant;
            ss >> comp_operator;
            ss >> constant;

            // std::cout << "comp_operator: " << comp_operator << std::endl;
            // std::cout << "constant: " << constant << std::endl;

            // TODO: maybe some error processing

            int c = std::stoi(constant);

            if (comp_operator == "<=") {

                guard_constraints.push_back(monitaal::constraint_t::upper_non_strict(1, c));

            } else if (comp_operator == "<") {

                guard_constraints.push_back(monitaal::constraint_t::upper_strict(1, c));

            } else if (comp_operator == ">=") {

                guard_constraints.push_back(monitaal::constraint_t::lower_non_strict(1, c));

            } else { // comp_operator == ">"

                guard_constraints.push_back(monitaal::constraint_t::lower_strict(1, c));

            }

        }

        if (!guard_y.empty()) {

            std::stringstream ss(guard_y);
            std::string comp_operator;
            std::string constant;
            ss >> comp_operator;
            ss >> constant;

            // std::cout << "comp_operator: " << comp_operator << std::endl;
            // std::cout << "constant: " << constant << std::endl;

            // TODO: maybe some error processing

            int c = std::stoi(constant);

            if (comp_operator == "<=") {

                guard_constraints.push_back(monitaal::constraint_t::upper_non_strict(2, c));

            } else if (comp_operator == "<") {

                guard_constraints.push_back(monitaal::constraint_t::upper_strict(2, c));

            } else if (comp_operator == ">=") {

                guard_constraints.push_back(monitaal::constraint_t::lower_non_strict(2, c));

            } else { // comp_operator == ">"

                guard_constraints.push_back(monitaal::constraint_t::lower_strict(2, c));

            }

        }

        bdd_edges.push_back(monitaal::bdd_edge_t(name_id_map.at(source), name_id_map.at(target), guard_constraints, reset_clocks, label));


        if (out_format.has_value()) {

            if (out_format.value()) {

                bdd_allsat(label, *allsat_print_handler);

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
                    s = s + std::string("edge:") + "TA_" + std::to_string(id) + ":ell_" + source + ":ell_" + target + ":a{provided: turn == 1"
                                        + (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                        + (guard_x.size() ? " && x_" + std::to_string(id) + " " + guard_x : std::string{})
                                        + (guard_y.size() ? " && y_" + std::to_string(id) + " " + guard_y : std::string{});
                    std::string reset_clocks_str;

                    if (reset == 1) {
                        reset_clocks_str = " : do: x_" + std::to_string(id) + " = 0}";
                    } else if (reset == 2) {
                        reset_clocks_str = " : do: y_" + std::to_string(id) + " = 0}";
                    } else if (reset == 3) {
                        reset_clocks_str = " : do: x_" + std::to_string(id) + " = 0; " + "y_" + std::to_string(id) + " = 0}";
                    } else {
                        assert(reset == 0);
                        reset_clocks_str = "}";
                    }
                    
                    s = s + reset_clocks_str + "\n"; 

                    std::stringstream().swap(p_constraint);

                }

                sat_paths.clear();
                out_s << s;

            }

        }

        return;

    }
    

    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_ta_from_atom(const MitlParser::AtomContext* phi_) {

        std::stringstream out_str;

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        if (phi_->type == FINALLY) {

            MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomFContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << name << std::endl;

                }

            }

            if (phi->interval() == nullptr) {

                /***** Finally
                 "untimed" case
                *****/

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});
                name_id_map.insert({"2", 2});

                monitaal::bdd_edges_t bdd_edges;


                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id)) << "}" << std::endl;

                    }

                }


                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->tilde);

                // 2 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->tilde);

                // 1 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 2 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 1 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 2 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Finally
                     [0, u) or [0, u]
                    *****/

                    if (right->children[0]->getText() == "infty") {
                        assert(("Please remove superflous interval [0, infty)", false));
                    }

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});

                    monitaal::bdd_edges_t bdd_edges;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id)) << "}" << std::endl;

                        }

                    }

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_true() & phi->atom()->tilde);

                    // 1 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);


                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Finally
                     [l, infty) or (l, infty)
                    *****/

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));
                    // locations.push_back(monitaal::location_t(false, 3, "s3", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});
                    // name_id_map.insert({"3", 3});

                    monitaal::bdd_edges_t bdd_edges;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id)) << "}" << std::endl;
                            // out_str << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                        }

                    }


                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);                    

                    // 1 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // // 1 -> 3, r && ^p && ~q, x := 0

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "3", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // // 1 -> 3, r && ^p && *q, x := 0, x < a

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 3 -> 3, ^p && ~q

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "3", "3", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->tilde);

                    // // 3 -> 3, ^p && *q, x < a

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "3", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_true() & phi->atom()->star);

                    // // 3 -> 2, ^p && ^q, x := 0, x >= a

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "3", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_true() & phi->atom()->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


                } else {

                    assert(("Unexpected <l, u> on F O G H U S R T?", false));

                }


            }

        } else if (phi_->type == ONCE) {

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


        } else if (phi_->type == GLOBALLY) {

            MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomGContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << name << std::endl;

                }

            }

            if (phi->interval() == nullptr) {

                /***** Globally
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

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 1 -> 0, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Globally
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

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);


                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Globally
                     [l, infty) or (l, infty)
                    *****/

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

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 1, r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 0, !r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


                } else {

                    assert(("Unexpected <l, u> on F O G H U S R T?", false));

                }

            }

        } else if (phi_->type == HISTORICALLY) {

            MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomHContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << name << std::endl;

                }

            }

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

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << "location:" << "TA_" << phi->id << ":ell_0i{initial: }" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_0{labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    }

                }

                // 0i -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0i -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0i -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->hat);

                // 0i -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 0i -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 0 -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 2) }, out_str.str() };   // last arg: initial location id

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

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0i{initial: }" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_0{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                        }

                    }

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0i -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);
                    
                    // 0i -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);                    

                    // 0i -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0i -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);


                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);
                    
                    // 2 -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0 -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) }, out_str.str() };   // last arg: initial location id


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

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0i{initial: }" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_0{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                        }

                    }

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->hat);

                    // 0i -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_true() & phi->atom()->star);

                    // 0i -> 2, !r && ~p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, !r && ~p && *q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0i -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0i -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);


                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_true() & phi->atom()->hat);

                    // 2 -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_true() & phi->atom()->star);
                    
                    // 1 -> 2, !r && ~p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, !r && ~p && *q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0 -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) }, out_str.str() };   // last arg: initial location id

                } else {

                    assert(("Unexpected <l, u> on F O G H U S R T?", false));

                }

            }


        } else if (phi_->type == UNTIL) {

            MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomUContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << name << std::endl;

                }

            }

            if (phi->interval() == nullptr) {

                /***** Until
                 "untimed" case
                *****/

                monitaal::constraints_t empty_invariant;
                monitaal::locations_t locations;

                locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));

                std::map<std::string, monitaal::location_id_t> name_id_map;
                name_id_map.insert({"0", 0});
                name_id_map.insert({"1", 1});
                name_id_map.insert({"2", 2});

                monitaal::bdd_edges_t bdd_edges;

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id)) << "}" << std::endl;

                    }

                }

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 2 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 1 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 2 -> 2, r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 1 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 2 -> 0, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Until
                     [0, u) or [0, u]
                    *****/

                    if (right->children[0]->getText() == "infty") {
                        assert(("Please remove superflous interval [0, infty)", false));
                    }

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});

                    monitaal::bdd_edges_t bdd_edges;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id)) << "}" << std::endl;

                        }

                    }

                    // 0 -> 0, , x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);


                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Until
                     [l, infty) or (l, infty)
                     *****/

                    clocks.insert({1, "x1"});

                    monitaal::constraints_t empty_invariant;
                    monitaal::locations_t locations;

                    locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
                    locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));
                    locations.push_back(monitaal::location_t(out_fin ? false : true, 2, "s2", empty_invariant));
                    // locations.push_back(monitaal::location_t(false, 3, "s3", empty_invariant));

                    std::map<std::string, monitaal::location_id_t> name_id_map;
                    name_id_map.insert({"0", 0});
                    name_id_map.insert({"1", 1});
                    name_id_map.insert({"2", 2});
                    // name_id_map.insert({"3", 3});

                    monitaal::bdd_edges_t bdd_edges;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{" << (out_fin ? "" : "labels: accept_" + std::to_string(phi->id)) << "}" << std::endl;
                            // out_str << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                        }

                    }

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 1, !r && ^p && ~q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, !r && ^p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 1, r && ^p && ~q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 2, !r && ^p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 2, r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // // 1 -> 3, r && ^p && ~q, x := 0

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "3", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 1 -> 3, r && ^p && *q, x := 0, x < a

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // // 3 -> 3, ^p && ~q

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "3", "3", std::string{}, std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 3 -> 3, ^p && *q, x < a

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "3", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->star);

                    // // 3 -> 2, ^p && ^q, x := 0, x >= a

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "3", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, phi->atom(0)->hat & phi->atom(1)->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

                } else {

                    assert(("Unexpected <l, u> on F O G H U S R T?", false));

                }

            }


        } else if (phi_->type == SINCE) {

            MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomSContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << name << std::endl;

                }

            }

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

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    }

                }

                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ^p && ~q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 1 -> 1, r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 0 -> 1, !r && *p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


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

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                        }

                    }

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a 

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

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

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;
                            // out_str << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                        }

                    }

                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ^p && *q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // // 2 -> 1, !r && ^p && *q

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 1 -> 2, !r && ^p && *q

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, !r && ^p && *q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ~q, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, r && ^p && ~q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 1 -> 2, r && ^p && ~q, x >= a

                    // build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 2, r && ^p && ~q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && *p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 1, r && *p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id


                } else {

                    assert(("Unexpected <l, u> on F O G H U S R T?", false));

                }

            }


        } else if (phi_->type == RELEASE) {

            MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomRContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << name << std::endl;

                }

            }

            if (phi->interval() == nullptr) {

                /***** Release
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

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0 -> 1, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 1 -> 0, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

            } else {

                antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[0];
                antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)phi->interval()->children[4];

                antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)phi->interval()->children[3];

                if (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0") {

                    /***** Release
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

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 1, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);


                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id



                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Release
                     [l, infty) or (l, infty)
                    *****/

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

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ~p && ^q, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0) }, out_str.str() };   // last arg: initial location id

                } else {

                    assert(("Unexpected <l, u> on F O G H U S R T?", false));

                }

            }

        } else if (phi_->type == TRIGGER) {

            MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)phi_;

            std::string name = "TA_" + std::to_string(phi->id);

            if (out_format.has_value()) {

                if (out_format.value()) {

                    out_str << std::endl << std::endl;
                    out_str << "# " << "TA_" << phi->id << std::endl;
                    out_str << "# " << const_cast<MitlParser::AtomTContext*>(phi)->getText() << std::endl;
                    out_str << "process:" << name << std::endl;

                }

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

                monitaal::bdd_edges_t bdd_edges;

                if (out_format.has_value()) {

                    if (out_format.value()) {

                        out_str << "location:" << "TA_" << phi->id << ":ell_0i{initial: }" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_0{labels: accept_" << phi->id << "}" << std::endl;
                        out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    }

                }

                // 0i -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0i -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 0i -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0i -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


                // 0 -> 0, !r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ~p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0 -> 1, !r && ^p && ^q

                build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

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

                    monitaal::bdd_edges_t bdd_edges;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0i{initial: }" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_0{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                        }

                    }

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);
                    
                    // 0i -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);


                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 2, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ~p && ^q, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);
                    
                    // 2 -> 2, r && ~p && ^q, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 0, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 2 -> 2, !r && ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", std::string{}, std::string{}, 0, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 2, r && *p && *q, x := 0, x <= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 2, r && *p && *q, x := 0, x > a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0 -> 2, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

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

                    monitaal::bdd_edges_t bdd_edges;

                    if (out_format.has_value()) {

                        if (out_format.value()) {

                            out_str << "location:" << "TA_" << phi->id << ":ell_0i{initial: }" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_0{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                            out_str << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                        }

                    }

                    // 0i -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0i -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 0i -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0i -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0i -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0i -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0i -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0i", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);



                    // 0 -> 0, !r && *p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "0", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ~p && ^q

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", std::string{}, std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 2 -> 2, ~p && *q, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x < a

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 1, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0 -> 2, !r && ^p && *q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "2", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    build_edge(bdd_edges, name_id_map, out_str, phi->id, "0", "1", std::string{}, std::string{}, 1, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    return { { monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 3) }, out_str.str() };   // last arg: initial location id

                } else {

                    assert(("Unexpected <l, u> on F O G H U S R T?", false));

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

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), std::string{}, 0, (j == 0 ? phi->tilde : !encode(i + 1, phi->id, phi->bits)) & phi->atoms[j]->tilde);

                            // 1_j -> 1_j+1, !p1 && ^p_j (x := 0, x <= a)

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? "0" : "1_" + std::to_string(j + 1)), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), std::string{}, (j + 1 == phi->atoms.size() ? true : false), !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                        }

                        // 1_0 -> 1_0, p1 && *p_0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_0", "1_0", std::string{}, std::string{}, 0, encode(i + 1, phi->id, phi->bits) & phi->atoms[0]->star);

                        // 1_n-1 -> 2, p1 && ^p_j, x := 0, x <= a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->atoms.size() - 1), "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[phi->atoms.size() - 1]->hat);

                        // 2 -> 1_0, !r && ~p_0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "2", "1_0", std::string{}, std::string{}, 0, phi->tilde & phi->atoms[0]->tilde);

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

                        if (out_format.has_value()) {

                            if (out_format.value()) {

                                out_str << std::endl << std::endl;
                                out_str << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                                out_str << "# " << const_cast<MitlParser::AtomOnContext*>(phi)->getText() << std::endl;
                                out_str << "process:" << "TA_" << phi->id + i << std::endl;

                                out_str << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                                for (auto j = 0; j < phi->atoms.size(); ++j) {
                                    out_str << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{labels: accept_" << phi->id + i << "}" << std::endl;
                                }

                            }

                        }

                        // 0 -> 0, !r, x := 0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "0", std::string{}, std::string{}, 1, !encode(i + 1, phi->id, phi->bits));

                        // 1_0 -> 0, r, x := 0, x <= a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ~p_j

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->tilde);

                            // 1_j+1 -> 1_j, !r && ^p_j (x := 0)

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, (j + 1 == phi->atoms.size() ? "0" : "1_" + std::to_string(j + 1)), "1_" + std::to_string(j), std::string{}, std::string{}, (j + 1 == phi->atoms.size() ? true : false), !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                            // 1_0 -> 1_j, r && *p_j (r && ~p_j)

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_0", "1_" + std::to_string(j), std::string{}, std::string{}, 0, encode(i + 1, phi->id, phi->bits) & (j + 1 == phi->atoms.size() ? phi->atoms[phi->atoms.size() - 1]->tilde : phi->atoms[j]->star));

                        }

                        // 1_0 -> 1_n-1, r && ^p_j, x := 0, x <= a

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_0", "1_" + std::to_string(phi->atoms.size() - 1), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[phi->atoms.size() - 1]->hat);

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

                        if (out_format.has_value()) {

                            if (out_format.value()) {

                                out_str << std::endl << std::endl;
                                out_str << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                                out_str << "# " << const_cast<MitlParser::AtomFnDualContext*>(phi)->getText() << std::endl;
                                out_str << "process:" << "TA_" << phi->id + i << std::endl;

                                out_str << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                                for (auto j = 0; j < phi->atoms.size(); ++j) {
                                    out_str << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{labels: accept_" << phi->id + i << "}" << std::endl;
                                }

                            }

                        }

                        // 0 -> 0, !r, x := 0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "0", std::string{}, std::string{}, 1, !encode(i + 1, phi->id, phi->bits));

                        // 0 -> 1_0, r, x := 0

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "0", "1_0", std::string{}, std::string{}, 1, encode(i + 1, phi->id, phi->bits));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ^p_j

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                            // 1_j -> 1_j, r && ^p_j, x := 0

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->hat);

                            if (j + 1 != phi->atoms.size()) {

                                // 1_j -> 1_j+1, !r && ~p_j

                                build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), std::string{}, std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->tilde);

                                // 1_j -> 1_j+1, r && ~p_j, x := 0

                                build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), std::string{}, std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->tilde);

                            }

                            // 1_j -> 0, !r && *p_j, x := 0, x > a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, !encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->star);

                            // 1_j -> 1_0, r && *p_j, x := 0, x > a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), std::string{}, 1, encode(i + 1, phi->id, phi->bits) & phi->atoms[j]->star);


                        }

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

                        if (out_format.has_value()) {

                            if (out_format.value()) {

                                out_str << std::endl << std::endl;
                                out_str << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                                out_str << "# " << const_cast<MitlParser::AtomOnDualContext*>(phi)->getText() << std::endl;
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



        } else if (phi_->type == COUNTFN) {

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

        } else if (phi_->type == COUNTFNDUAL) {

            MitlParser::AtomCFnDualContext* phi = (MitlParser::AtomCFnDualContext*)phi_;

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

                    /***** Count Fn Dual
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
                            locations.push_back(monitaal::location_t(true, 1 + j, "s1_" + std::to_string(j), empty_invariant));
                            name_id_map.insert({"1_" + std::to_string(j), 1 + j});
                        }

                        if (out_format.has_value()) {

                            if (out_format.value()) {

                                out_str << std::endl << std::endl;
                                out_str << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->max_l + 1 << ")" << std::endl;
                                out_str << "# " << const_cast<MitlParser::AtomCFnDualContext*>(phi)->getText() << std::endl;
                                out_str << "process:" << "TA_" << phi->id + i << std::endl;

                                out_str << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                                for (auto j = 0; j < phi->max_l + 1; ++j) {
                                    out_str << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{labels: accept_" << phi->id + i << "}" << std::endl;
                                }

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

                            // 1_j -> 1_j, !p1 && ~phi, x < a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atom(0)->hat);

                            // 1_j -> 1_j, p1 && ~phi, y := 0, x < a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 2, encode(i + 1, phi->id, phi->bits) & phi->atom(0)->hat);

                            // 1_j -> 1_j+1, !p1 && phi, x < a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, 0, !encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat);

                            // 1_j -> 1_j+1, p1 && phi, y := 0, x < a

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), std::string{}, (j + 1 == phi->max_l ? 3 : 2), encode(i + 1, phi->id, phi->bits) & phi->atom(1)->hat);

                        }

                        // 1_3 -> 1_3, !p1 && phi2, x := 0, y <= b

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_" + std::to_string(phi->max_l), std::string{}, (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), 1, !encode(i + 1, phi->id, phi->bits) & phi->atom(2)->hat);
                        
                        // 1_3 -> 1_3, p1 && phi2, x := 0, y := 0, y <= b

                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_" + std::to_string(phi->max_l), std::string{}, (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), 3, encode(i + 1, phi->id, phi->bits) & phi->atom(2)->hat);

                        // 1_3 -> 0, !p1, x := 0; y := 0, y > b
                        
                        build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "0", std::string{}, (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), 3, !encode(i + 1, phi->id, phi->bits));
        
                        // 1_3 -> 1_0, p1, x := 0; y := 0, y > b

                        // 1_3 -> 1_1, p1, x := 0; y := 0, y > b

                        // 1_3 -> 1_2, p1, x := 0; y := 0, y > b

                        // 1_3 -> 1_3, p1, x := 0; y := 0, y > b

                        for (auto j = 0; j < phi->max_l + 1; ++j) {

                            build_edge(bdd_edges, name_id_map, out_str, phi->id + i, "1_" + std::to_string(phi->max_l), "1_" + std::to_string(j), std::string{}, (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), 3, encode(i + 1, phi->id, phi->bits));

                        }




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

        } else {

            assert(("Unsupported atom type", false));

        }

        assert(false);

    }

    std::pair<monitaal::TA, std::string> build_ta_from_main(MitlParser::MainContext* phi_) {

        std::cout << "\nInput formula (getText() on the parse tree):\n" << std::endl;

        std::cout << phi_->getText() << std::endl;

        MitlFormulaVisitor formula_visitor;

        std::string uni_in;

        MitlCheckUniVisitor check_uni_visitor;

        if (!std::any_cast<bool>(check_uni_visitor.visitMain(phi_))) {

            std::cout << "\nThe input formula contains temporal operators with <l, u>." << std::endl;

            std::cout << "\nRewriting into unilateral formula...\n";

            MitlToUniVisitor to_uni_visitor;

            uni_in = std::any_cast<std::string>(to_uni_visitor.visitMain(phi_));


        } else {

            MitlToUniVisitor to_uni_visitor;

            uni_in = std::any_cast<std::string>(to_uni_visitor.visitMain(phi_));

        }

        std::cout << "\nInput formula (in unilateral form):\n";

        std::cout << uni_in;

        std::cout << "\nRe-parsing...\n";

        antlr4::ANTLRInputStream uni_input = antlr4::ANTLRInputStream(uni_in);

        MitlLexer uni_lexer(&uni_input);
        antlr4::CommonTokenStream uni_tokens(&uni_lexer);

        MitlParser uni_parser(&uni_tokens);

        MitlParser::MainContext* uni_formula = uni_parser.main();

        assert(std::any_cast<bool>(check_uni_visitor.visitMain(uni_formula)));

        std::string nnf_in;

        MitlCheckNNFVisitor check_nnf_visitor;
//        check_nnf_visitor.loadParser(parser);      // This is for ruleNames[]
//

        if (!std::any_cast<bool>(check_nnf_visitor.visitMain(uni_formula))) {

            std::cout << "\nThe input formula is not in negation normal form (i.e. AND and OR only, and all negations\n"
                      << "appear just before letters)." << std::endl;

            std::cout << "\nRewriting into NNF...\n";

            MitlToNNFVisitor to_nnf_visitor;

            nnf_in = std::any_cast<std::string>(to_nnf_visitor.visitMain(uni_formula));


        } else {

            MitlToNNFVisitor to_nnf_visitor;

            nnf_in = std::any_cast<std::string>(to_nnf_visitor.visitMain(uni_formula));

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

        std::cout << "\n# of other bits:\n";

        std::cout << num_all_props - nnf_formula->props.size() << std::endl;


        std::cout << std::endl;
        std::cout << "props:" << std::endl << std::endl;
        for (const auto & [k, v] : nnf_formula->props) {
            std::cout << v << ": " << k << std::endl;
        }

        std::cout << std::endl;
        std::cout << "temporals:" << std::endl << std::endl;

        std::set<MitlParser::AtomContext*, atom_cmp> temporal_atoms;
        MitlCollectTemporalVisitor collect_temporal_visitor;
        temporal_atoms = std::any_cast<decltype(temporal_atoms)>(collect_temporal_visitor.visitMain(nnf_formula));
        for (auto it = temporal_atoms.rbegin(); it != temporal_atoms.rend(); ++it) {

            MitlToNNFVisitor to_nnf_visitor;
            std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(*it));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            std::cout << (*it)->id << ": " << atom_clean << std::endl;
            assert(nnf_formula->temporals.count(atom_clean));
            assert(nnf_formula->temporals[atom_clean] == (*it)->id);

        }

        std::cout << std::endl;
        std::cout << "repeats:" << std::endl << std::endl;
        for (const auto& v : nnf_formula->repeats) {
            std::cout << v << std::endl << std::endl;
        }



        /*
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
        */

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

            } else if ((*it)->type == COUNTFN) {

                for (auto i = 0; i < 4; ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnContext*)(*it))->atom(i)->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnContext*)(*it))->atom(i)->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnContext*)(*it))->atom(i)->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnContext*)(*it))->atom(i)->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == COUNTON) {

                for (auto i = 0; i < 4; ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnContext*)(*it))->atom(i)->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnContext*)(*it))->atom(i)->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnContext*)(*it))->atom(i)->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnContext*)(*it))->atom(i)->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == COUNTFNDUAL) {

                for (auto i = 0; i < 4; ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnDualContext*)(*it))->atom(i)->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnDualContext*)(*it))->atom(i)->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnDualContext*)(*it))->atom(i)->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCFnDualContext*)(*it))->atom(i)->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == COUNTONDUAL) {

                for (auto i = 0; i < 4; ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnDualContext*)(*it))->atom(i)->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnDualContext*)(*it))->atom(i)->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnDualContext*)(*it))->atom(i)->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomCOnDualContext*)(*it))->atom(i)->hat, *allsat_print_handler);

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


        std::cout << "\n<<<<<< Converting into TAs... >>>>>>\n\n";

        // auto div = monitaal::TA::time_divergence_ta(get_letters(std::string(num_all_props + 1, 'X')), true);

        auto div = monitaal::TAwithBDDEdges::time_divergence_ta(bdd_true());

        std::vector<monitaal::TAwithBDDEdges> temporal_components;
        std::stringstream out_str;

        if (out_format.has_value()) {

            if (out_format.value()) {

                out_str << "# File generated by MightyPPL" << std::endl;
                out_str << "system:model_and_spec" << std::endl << std::endl << std::endl;

                out_str << "event:a" << std::endl << std::endl << std::endl;

                out_str << "clock:1:x" << std::endl;        // for timed lamp
                out_str << "clock:1:y" << std::endl;

                for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

                    if ((*it)->type == FINALLY) {
                        MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == ONCE) {
                        MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == GLOBALLY) {
                        MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == HISTORICALLY) {
                        MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == UNTIL) {
                        MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == SINCE) {
                        MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == RELEASE) {
                        MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == TRIGGER) {
                        MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << std::endl;
                        }
                    } else if ((*it)->type == PNUELIFN) {
                        MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                        }
                    } else if ((*it)->type == PNUELION) {
                        MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                        }
                    } else if ((*it)->type == PNUELIFNDUAL) {
                        MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                        }
                    } else if ((*it)->type == PNUELIONDUAL) {
                        MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                        }
                    } else if ((*it)->type == COUNTFN) {
                        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);
                        for (auto i = 0; i < phi->max_l + 1; ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                            out_str << "clock:1:y_" << phi->id + i << std::endl;
                        }
                    } else if ((*it)->type == COUNTON) {
                        MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);
                        for (auto i = 0; i < phi->max_l + 1; ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                            out_str << "clock:1:y_" << phi->id + i << std::endl;
                        }
                    } else if ((*it)->type == COUNTFNDUAL) {
                        MitlParser::AtomCFnDualContext* phi = (MitlParser::AtomCFnDualContext*)(*it);
                        for (auto i = 0; i < phi->max_l + 1; ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                            out_str << "clock:1:y_" << phi->id + i << std::endl;
                        }
                    } else if ((*it)->type == COUNTONDUAL) {
                        MitlParser::AtomCOnDualContext* phi = (MitlParser::AtomCOnDualContext*)(*it);
                        for (auto i = 0; i < phi->max_l + 1; ++i) {
                            out_str << "clock:1:x_" << phi->id + i << std::endl;
                            out_str << "clock:1:y_" << phi->id + i << std::endl;
                        }
                    } else {
                        assert(false);
                    }

                }

                out_str << "clock:1:x_div" << std::endl;
                out_str << std::endl << std::endl;


                for (auto i = 0; i < num_all_props; ++i) {
                    out_str << "int:1:0:1:0:p_" << i + 1 << std::endl;
                }

                out_str << "int:1:0:1:0:turn" << std::endl;


            }

        }


        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIFNDUAL || (*it)->type == PNUELIONDUAL
                    || (*it)->type == COUNTFN || (*it)->type == COUNTON || (*it)->type == COUNTFNDUAL || (*it)->type == COUNTONDUAL) {
                std::cout << "\nGenerating TA_" << (*it)->id << " (and other sub-components)...\n";
            } else {
                std::cout << "\nGenerating TA_" << (*it)->id << "...\n";
            }
            auto [ generated_components, component_str ] = build_ta_from_atom(*it);
            temporal_components.insert(temporal_components.end(), generated_components.begin(), generated_components.end());

            if (out_format.has_value()) {

                out_str << component_str;

            }


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

        if (out_format.has_value()) {

            if (out_format.value()) {

                out_str << std::endl << std::endl;
                out_str << "# " << "TA_0" << std::endl;
                out_str << "# " << nnf_formula->formula()->getText() << std::endl;
                out_str << "process:" << "TA_0" << std::endl;

                out_str << "location:" << "TA_0" << ":ell_0{initial: }" << std::endl;
                out_str << "location:" << "TA_0" << ":ell_1{labels: accept_0}" << std::endl;

                bdd label;

                // 0 -> 1, varphi

                label = nnf_formula->hat;

                bdd_allsat(label, *allsat_print_handler);

                for (const auto& p : sat_paths) {
                    for (const auto& s : get_letters(p)) {
                        if (s[0] == '0') {  // Recall that the 0-th variable is unused
                            if (s.size() > 1) {
                                std::stringstream p_assignments;
                                for (auto i = 1; i < s.size(); ++i) {
                                    p_assignments << "p_" << i << (s[i] == '0' ? " = 0" : " = 1");
                                    if (i != s.size() - 1) {
                                        p_assignments << "; ";
                                    }
                                }
                                out_str << "edge:" << "TA_0" << ":ell_0:ell_1:a{provided: turn == 0 : do: " << p_assignments.str() << "}" << std::endl;
                            } else {
                                out_str << "edge:" << "TA_0" << ":ell_0:ell_1:a{provided: turn == 0}" << std::endl;
                            }
                        }
                    }
                }

                sat_paths.clear();

                // 1 -> 1, *varphi

                label = nnf_formula->star;

                bdd_allsat(label, *allsat_print_handler);

                for (const auto& p : sat_paths) {
                    for (const auto& s : get_letters(p)) {
                        if (s[0] == '0') {  // Recall that the 0-th variable is unused
                            if (s.size() > 1) {
                                std::stringstream p_assignments;
                                for (auto i = 1; i < s.size(); ++i) {
                                    p_assignments << "p_" << i << (s[i] == '0' ? " = 0" : " = 1");
                                    if (i != s.size() - 1) {
                                        p_assignments << "; ";
                                    }
                                }
                                out_str << "edge:" << "TA_0" << ":ell_1:ell_1:a{provided: turn == 0 : do: " << p_assignments.str() << "}" << std::endl;
                            } else {
                                out_str << "edge:" << "TA_0" << ":ell_1:ell_1:a{provided: turn == 0}" << std::endl;
                            }
                        }
                    }
                }

                sat_paths.clear();

                std::cout << "\nGenerating TA_div" << "...\n";

                out_str << std::endl << std::endl;
                out_str << "# " << "TA_div" << std::endl;
                out_str << "process:" << "TA_div" << std::endl;

                out_str << "location:" << "TA_div" << ":ell_0{initial: : labels: accept_div}" << std::endl;
                out_str << "location:" << "TA_div" << ":ell_1{" << (out_fin ? "" : "labels: accept_div") << "}" << std::endl;

                out_str << "edge:" << "TA_div" << ":ell_0:ell_1:a{provided: turn == 0 : do: turn = 1}" << std::endl;
                out_str << "edge:" << "TA_div" << ":ell_0:ell_1:a{provided: turn == 1 : do: turn = 0}" << std::endl;

                out_str << "edge:" << "TA_div" << ":ell_1:ell_1:a{provided: turn == 0 && x_div < 1 : do: turn = 1}" << std::endl;
                out_str << "edge:" << "TA_div" << ":ell_1:ell_1:a{provided: turn == 1 && x_div < 1 : do: turn = 0}" << std::endl;

                out_str << "edge:" << "TA_div" << ":ell_1:ell_0:a{provided: turn == 0 && x_div >= 1 : do: turn = 1; x_div = 0}" << std::endl;
                out_str << "edge:" << "TA_div" << ":ell_1:ell_0:a{provided: turn == 1 && x_div >= 1 : do: turn = 0; x_div = 0}" << std::endl;


            }

        }

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
        clocks.insert({1, "x"});        // clock 0 is needed anyway
        clocks.insert({2, "y"});        // clock 0 is needed anyway

        locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
        locations.push_back(monitaal::location_t(false, 1, "s1", empty_invariant));


        //     if (comp_operator == "<=") {

        //         guard_constraints.push_back(monitaal::constraint_t::upper_non_strict(1, c));

        //     } else if (comp_operator == "<") {

        //         guard_constraints.push_back(monitaal::constraint_t::upper_strict(1, c));

        //     } else if (comp_operator == ">=") {

        //         guard_constraints.push_back(monitaal::constraint_t::lower_non_strict(1, c));

        //     } else { // comp_operator == ">"

        //         guard_constraints.push_back(monitaal::constraint_t::lower_strict(1, c));

        //     }



        label = !bdd_ithvar(nnf_formula->props.at("push")) & !bdd_ithvar(nnf_formula->props.at("blink"));
        reset.push_back(1);
        guard.push_back(monitaal::constraint_t::lower_non_strict(1, 1));
        guard.push_back(monitaal::constraint_t::upper_non_strict(1, 1));
        bdd_edges.push_back(monitaal::bdd_edge_t(0, 0, guard, reset, label));
        guard.clear();
        reset.clear();

        label = bdd_ithvar(nnf_formula->props.at("push")) & bdd_ithvar(nnf_formula->props.at("blink"));
        reset.push_back(1);
        reset.push_back(2);
        bdd_edges.push_back(monitaal::bdd_edge_t(0, 1, guard, reset, label));
        guard.clear();
        reset.clear();

        label = !bdd_ithvar(nnf_formula->props.at("push")) & bdd_ithvar(nnf_formula->props.at("blink"));
        reset.push_back(1);
        guard.push_back(monitaal::constraint_t::lower_non_strict(1, 1));
        guard.push_back(monitaal::constraint_t::upper_non_strict(1, 1));
        guard.push_back(monitaal::constraint_t::upper_non_strict(2, 2));
        bdd_edges.push_back(monitaal::bdd_edge_t(1, 1, guard, reset, label));
        guard.clear();
        reset.clear();

        label = bdd_ithvar(nnf_formula->props.at("push")) & bdd_ithvar(nnf_formula->props.at("blink"));
        reset.push_back(1);
        reset.push_back(2);
        bdd_edges.push_back(monitaal::bdd_edge_t(1, 1, guard, reset, label));
        guard.clear();
        reset.clear();


        label = !bdd_ithvar(nnf_formula->props.at("push")) & bdd_ithvar(nnf_formula->props.at("blink"));
        reset.push_back(1);
        guard.push_back(monitaal::constraint_t::lower_non_strict(1, 1));
        guard.push_back(monitaal::constraint_t::upper_non_strict(1, 1));
        guard.push_back(monitaal::constraint_t::lower_non_strict(2, 3));
        guard.push_back(monitaal::constraint_t::upper_non_strict(2, 3));
        bdd_edges.push_back(monitaal::bdd_edge_t(1, 0, guard, reset, label));
        guard.clear();

        auto model = monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0);   // last arg: initial location id
        clocks.clear();
        locations.clear();
        bdd_edges.clear();

        if (out_format.has_value()) {

            if (out_format.value()) {

                out_str << std::endl << std::endl;
                out_str << "# " << "M" << std::endl;
                out_str << "process:" << "M" << std::endl;

                out_str << "location:" << "M" << ":ell_0{initial: : labels: accept_M}" << std::endl;
                out_str << "location:" << "M" << ":ell_1{}" << std::endl;

                out_str << "edge:" << "M" << ":ell_0:ell_0:a{provided: turn == 1 && p_" << std::to_string(nnf_formula->props.at("push")) << " == 0 && p_" << std::to_string(nnf_formula->props.at("blink")) << " == 0 && x == 1 : do: x = 0}" << std::endl;
                out_str << "edge:" << "M" << ":ell_0:ell_1:a{provided: turn == 1 && p_" << std::to_string(nnf_formula->props.at("push")) << " == 1 && p_" << std::to_string(nnf_formula->props.at("blink")) << " == 1 : do: x = 0; y = 0}" << std::endl;
                out_str << "edge:" << "M" << ":ell_1:ell_1:a{provided: turn == 1 && p_" << std::to_string(nnf_formula->props.at("push")) << " == 0 && p_" << std::to_string(nnf_formula->props.at("blink")) << " == 1 && x == 1 && y <= 2 : do: x = 0}" << std::endl;
                out_str << "edge:" << "M" << ":ell_1:ell_1:a{provided: turn == 1 && p_" << std::to_string(nnf_formula->props.at("push")) << " == 1 && p_" << std::to_string(nnf_formula->props.at("blink")) << " == 1 : do: x = 0; y = 0}" << std::endl;
                out_str << "edge:" << "M" << ":ell_1:ell_0:a{provided: turn == 1 && p_" << std::to_string(nnf_formula->props.at("push")) << " == 0 && p_" << std::to_string(nnf_formula->props.at("blink")) << " == 1 && x == 1 && y == 3 : do: x = 0}" << std::endl;

                // std::cout << "\nGenerating sync constraints" << "...\n";

                out_str << std::endl << std::endl;
                out_str << "# " << "sync constraints" << std::endl;
                out_str << "sync:TA_div@a:TA_0@a" << std::endl;

                out_str << "sync:TA_div@a:M@a:";

                for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {


                    if ((*it)->type == PNUELIFN) {

                        MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else if ((*it)->type == PNUELION) {

                        MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else if ((*it)->type == PNUELIFNDUAL) {

                        MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else if ((*it)->type == PNUELIONDUAL) {

                        MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else if ((*it)->type == COUNTFN) {

                        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                        for (auto i = 0; i < (*it)->max_l + 1; ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else if ((*it)->type == COUNTON) {

                        MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                        for (auto i = 0; i < (*it)->max_l + 1; ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else if ((*it)->type == COUNTFNDUAL) {

                        MitlParser::AtomCFnDualContext* phi = (MitlParser::AtomCFnDualContext*)(*it);

                        for (auto i = 0; i < (*it)->max_l + 1; ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else if ((*it)->type == COUNTONDUAL) {

                        MitlParser::AtomCOnDualContext* phi = (MitlParser::AtomCOnDualContext*)(*it);

                        for (auto i = 0; i < (*it)->max_l + 1; ++i) {

                            out_str << "TA_" << (*it)->id + i << "@a";
                            out_str << ":";

                        }

                        out_str << "seq_" << (*it)->id << "@a";

                    } else {

                        out_str << "TA_" << (*it)->id << "@a";

                    }

                    if (std::next(it) != temporal_atoms.end()) {
                        out_str << ":";
                    }


                }
                out_str << std::endl;

            } else {

                // no need to generate XML in out_str at this point

            }

        }

        std::cout << std::endl;

        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << model.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << model.number_of_clocks() << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto & [k, v] : model.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << model.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << model.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }



        if (!out_format.has_value() || (out_format.has_value() && out_flatten)) {

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

            // return TA, but not the generated output for components 

            return { product.projection(props_to_remove), std::string{} };

        } else { // out_format.has_value() && !out_flatten

            if (out_format.value()) {

                std::cout << "\nPlease use the following command to check satisfiability:\n\n";
                if (out_fin) {
                    std::cout << "tck-reach -a covreach -l ";
                } else {
                    std::cout << "tck-liveness -a couvscc -l ";
                }

                for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

                    if ((*it)->type == PNUELIFN) {

                        MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else if ((*it)->type == PNUELION) {

                        MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else if ((*it)->type == PNUELIFNDUAL) {

                        MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else if ((*it)->type == PNUELIONDUAL) {

                        MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else if ((*it)->type == COUNTFN) {

                        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                        for (auto i = 0; i < phi->max_l + 1; ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else if ((*it)->type == COUNTON) {

                        MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                        for (auto i = 0; i < phi->max_l + 1; ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else if ((*it)->type == COUNTFNDUAL) {

                        MitlParser::AtomCFnDualContext* phi = (MitlParser::AtomCFnDualContext*)(*it);

                        for (auto i = 0; i < phi->max_l + 1; ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else if ((*it)->type == COUNTONDUAL) {

                        MitlParser::AtomCOnDualContext* phi = (MitlParser::AtomCOnDualContext*)(*it);

                        for (auto i = 0; i < phi->max_l + 1; ++i) {

                            std::cout << "accept_" << (*it)->id + i << ",";

                        }

                        std::cout << "accept_seq_" << (*it)->id << ",";

                    } else {

                        std::cout << "accept_" << (*it)->id << ",";

                    }
                }

                std::cout << "accept_0,accept_M,accept_div " << out_file << std::endl;
                

                // return no TA, but the genereated output for components

                return { monitaal::TA("dummy", {}, {}, {}, 0), out_str.str() };

            } else {

                // return nothing; out_str is empty at this point anyway

                return { monitaal::TA("dummy", {}, {}, {}, 0), std::string{} };

            }

        }


        assert(false);

    }

} // namespace mightypplcpp
