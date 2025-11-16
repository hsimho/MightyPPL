#include "MightyPPL.h"

namespace mightypplcpp {

    int gcd = 0;
    bool last_intersection = false;

    size_t num_all_props;

    size_t components_counter;

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


    /*
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
    */

    void allsat_print_handler(char* varset, int size) {

        std::string output;

        for (int v = 0; v < size; ++v) {
            output += (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
        }
        // std::cout << std::setw(20) << output << std::setw(0) << std::endl;

        sat_paths.push_back(output);

    }

    void build_model_from_ta_bdd(const monitaal::TAwithBDDEdges ta, const size_t id, std::stringstream& out_s) {

        assert(comp_flatten && !out_flatten);

        std::string name = "TA_" + std::to_string(id) + "_0";

        if (out_format.value()) {       // tck

            std::stringstream tck;

            tck << std::endl << std::endl;
            tck << "# " << name << std::endl;
            tck << "# " << ta.name() << std::endl;
            tck << "process:" << name << std::endl;

            for (const auto& [k, v] : ta.locations()) {

                tck << "location:" << name << ":ell_" << k << "{" << (k == ta.initial_location() ?  (v.is_accept() ? "initial: : " : "initial: ") : "")
                    << (v.is_accept() ? "labels: accept_" + std::to_string(id) : std::string{}) << "}" << std::endl;

            }

            for (const auto& [k, v] : ta.locations()) {

                for (const auto& e : ta.bdd_edges_from(k)) {

                    bdd_allsat(e.bdd_label(), *allsat_print_handler);

                    std::stringstream p_constraint;
                    std::stringstream p_assignments;

                    for (auto i = 0; i < sat_paths.size(); ++i) {

                        int largest = 0;
                        for (auto j = 1; j < sat_paths[i].size(); ++j) {
                            if (sat_paths[i][j] != 'X' && j > largest) {
                                largest = j;
                            }
                        }
                        for (auto j = 1; j < sat_paths[i].size(); ++j) {
                            if (sat_paths[i][j] != 'X') {
                                p_constraint << "p_" << j << (sat_paths[i][j] == '0' ? " % 2 == 0" : " >= 1");
                                p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                                if (j != largest) {
                                    p_constraint << " && ";
                                    p_assignments << "; ";
                                }
                            }
                        }

                        std::string provided_str;
                        for (const auto& g : e.guard()) {

                            if (!provided_str.empty()) {
                                provided_str += " && ";
                            }

                            if (g._i == 0) {

                                assert(g._j != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(id) + "_" + std::to_string(g._j - 1) + " > " + std::to_string(-1 * g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(id) + "_" + std::to_string(g._j - 1) + " >= " + std::to_string(-1 * g._bound.get_bound());
                                }

                            } else if (g._j == 0) {

                                assert(g._i != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(id) + "_"  + std::to_string(g._i - 1) + " < " + std::to_string(g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(id) + "_"  + std::to_string(g._i - 1) + " <= " + std::to_string(g._bound.get_bound());
                                }

                            } else {
                                assert(("Currently support only non-diagonal guards", false));
                            }

                        }

                        std::string do_str;
                        for (const auto& r : e.reset()) {
                    
                            if (!do_str.empty()) {
                                do_str += "; ";
                            }

                            do_str += "x_" + std::to_string(id) + "_" + std::to_string(r - 1) + " = 0";

                        }


                        tck << "edge:" << name << ":ell_" << e.from() << ":ell_" << e.to() << ":a{provided: g == 0 && turn == " << components_counter
                                       << (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                       << (provided_str.empty() ? std::string{} : " && " + provided_str);

                        tck << " : do: turn = " << components_counter + 1 << (p_assignments.str().size() ?  "; " + p_assignments.str() : std::string{})
                            << (do_str.empty() ? std::string{} : "; " + do_str)
                            << "}" << std::endl;

                        std::stringstream().swap(p_constraint);
                        std::stringstream().swap(p_assignments);

                    }

                    sat_paths.clear();

                }

            }

            out_s << tck.str();


        } else {        // xml

            std::stringstream xml;

            xml << "\t<template>" << std::endl;
            xml << "\t\t<name>" << name << "</name>" << std::endl;

            xml << "\t\t<declaration>" << std::endl;


            monitaal::location_id_t largest_loc = 0;
            std::set<size_t> acc_set;
            for (const auto& [k, v] : ta.locations()) {
                if (k > largest_loc) {
                    largest_loc = k;
                }
                if (v.is_accept()) {
                    acc_set.insert(k);
                }
            }

            xml << "\t\t\tint[0, " << largest_loc << "] loc = " << ta.initial_location() << ";" << std::endl;
            xml << "\t\t\tint[0, 1] acc = 0;" << std::endl;


            xml << "\t\t</declaration>" << std::endl;

            xml << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
            xml << "\t\t</location>" << std::endl;
            xml << "\t\t<init ref=\"id0\"/>" << std::endl;

            for (const auto& [k, v] : ta.locations()) {

                for (const auto& e : ta.bdd_edges_from(k)) {

                    bdd_allsat(e.bdd_label(), *allsat_print_handler);

                    std::stringstream p_constraint;
                    std::stringstream p_assignments;

                    for (auto i = 0; i < sat_paths.size(); ++i) {

                        int largest = 0;
                        for (auto j = 1; j < sat_paths[i].size(); ++j) {
                            if (sat_paths[i][j] != 'X' && j > largest) {
                                largest = j;
                            }
                        }
                        for (auto j = 1; j < sat_paths[i].size(); ++j) {
                            if (sat_paths[i][j] != 'X') {
                                if (sat_paths[i][j] == '0') {
                                    p_constraint << "(p_" << j << " == 2" << " || " << "p_" << j << " == 0)";
                                } else {
                                    p_constraint << "p_" << j << " &gt;= 1";
                                }
                                p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                                if (j != largest) {
                                    p_constraint << " &amp;&amp; ";
                                    p_assignments << ", ";
                                }
                            }
                        }

                        std::string provided_str;
                        for (const auto& g : e.guard()) {

                            if (!provided_str.empty()) {
                                provided_str += " &amp;&amp; ";
                            }

                            if (g._i == 0) {

                                assert(g._j != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(id) + "_" + std::to_string(g._j - 1) + " &gt; " + std::to_string(-1 * g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(id) + "_" + std::to_string(g._j - 1) + " &gt;= " + std::to_string(-1 * g._bound.get_bound());
                                }

                            } else if (g._j == 0) {

                                assert(g._i != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(id) + "_" + std::to_string(g._i - 1) + " &lt; " + std::to_string(g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(id) + "_" + std::to_string(g._i - 1) + " &lt;= " + std::to_string(g._bound.get_bound());
                                }

                            } else {
                                assert(("Currently support only non-diagonal guards", false));
                            }

                        }


                        std::string do_str;
                        for (const auto& r : e.reset()) {
                    
                            if (!do_str.empty()) {
                                do_str += ", ";
                            }

                            do_str += "x_" + std::to_string(id) + "_" + std::to_string(r - 1) + " = 0";

                        }
                        
                        xml << "\t\t<transition>\n";
                        xml << "\t\t\t<source ref=\"id0\"/>\n";
                        xml << "\t\t\t<target ref=\"id0\"/>\n";


                        xml << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">"
                            << "g == 0 &amp;&amp; turn == " << components_counter
                            << " &amp;&amp; " << "loc == " << e.from()
                            << (p_constraint.str().size() ? " &amp;&amp; " + p_constraint.str() : "")
                            << (provided_str.empty() ? std::string{} : " &amp;&amp; " + provided_str)
                            << "</label>\n";

                        xml << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">"
                            << "turn = " << components_counter + 1
                            << ", " << "loc = " << e.to()
                            << ", " << "acc = " << (acc_set.count(e.to()) ? 1 : 0)
                            << (p_assignments.str().size() ? ", " + p_assignments.str() : "")
                            << (do_str.empty() ? std::string{} : ", " + do_str)
                            << "</label>\n";

                        xml << "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                        xml << "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                        xml << "\t\t</transition>\n";

                        std::stringstream().swap(p_constraint);
                        std::stringstream().swap(p_assignments);

                    }

                    sat_paths.clear();

                }

            }

            xml << "\t</template>" << std::endl << std::endl;

            out_s << xml.str();

        }

    }

    // TODO: refactoring to avoid overlap with build_edge() below

    void build_untimed_edge(monitaal::bdd_edges_t& bdd_edges, const std::map<std::string, monitaal::location_id_t>& name_id_map, std::stringstream& out_s, const std::string& automaton_name, const std::string& source, const std::string& target, bdd label) {

        bdd_edges.push_back(monitaal::bdd_edge_t(name_id_map.at(source), name_id_map.at(target), monitaal::constraints_t{}, monitaal::clocks_t{}, label));

        if (out_format.has_value() && !out_flatten) {

            if (out_format.value()) {   // tck

                bdd_allsat(label, *allsat_print_handler);

                std::stringstream p_constraint;
                std::stringstream p_assignments;
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
                            p_constraint << "p_" << j << (sat_paths[i][j] == '0' ? " % 2 == 0" : " >= 1");
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " && ";
                                p_assignments << "; ";
                            }
                        }
                    }

                    s += std::string("edge:") + automaton_name + ":ell_" + std::to_string(name_id_map.at(source)) + ":ell_" + std::to_string(name_id_map.at(target)) + ":a{provided: g == 0 && turn == " + std::to_string(components_counter)
                                        + (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                        + " : do: turn = " + std::to_string(components_counter + 1)
                                        + (p_assignments.str().size() ?  + "; " + p_assignments.str() + "}" : "}") 
                                        + "\n"; 

                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();
                out_s << s;

            } else {    // xml

                bdd_allsat(label, *allsat_print_handler);

                std::stringstream p_constraint;
                std::stringstream p_assignments;
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
                            if (sat_paths[i][j] == '0') {
                                p_constraint << "(p_" << j << " == 2" << " || " << "p_" << j << " == 0)";
                            } else {
                                p_constraint << "p_" << j << " &gt;= 1";
                            }
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " &amp;&amp; ";
                                p_assignments << ", ";
                            }
                        }
                    }

                    s += "\t\t<transition>\n";
                    s += "\t\t\t<source ref=\"id0\"/>\n";
                    s += "\t\t\t<target ref=\"id0\"/>\n";

                    std::string provided_str;
                    provided_str = "g == 0 &amp;&amp; turn == " + std::to_string(components_counter) + " &amp;&amp; loc == " + std::to_string(name_id_map.at(source)) + (p_constraint.str().size() ? " &amp;&amp; " + p_constraint.str() : "");

                    s += "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                    std::string do_str;
                    do_str = "turn = " + std::to_string(components_counter + 1) + ", loc = " + std::to_string(name_id_map.at(target)) + (p_assignments.str().size() ? ", " + p_assignments.str() : "");

                    s += "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                    s += "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                    s += "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                    s += "\t\t</transition>\n";

                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();
                out_s << s;

            }

        }

    }

    void build_edge(monitaal::bdd_edges_t& bdd_edges, const std::map<std::string, monitaal::location_id_t>& name_id_map, std::stringstream& out_s, const int base_id, const int offset_id, const std::string& source, const std::string& target, const std::string& guard_x, const std::string& guard_y, const int reset, bdd label) {

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

        std::string escaped_guard_x;

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
                escaped_guard_x = std::string("&lt;=") + " " + constant;

            } else if (comp_operator == "<") {

                guard_constraints.push_back(monitaal::constraint_t::upper_strict(1, c));
                escaped_guard_x = std::string("&lt;") + " " + constant;

            } else if (comp_operator == ">=") {

                guard_constraints.push_back(monitaal::constraint_t::lower_non_strict(1, c));
                escaped_guard_x = std::string("&gt;=") + " " + constant;

            } else { // comp_operator == ">"

                guard_constraints.push_back(monitaal::constraint_t::lower_strict(1, c));
                escaped_guard_x = std::string("&gt;") + " " + constant;

            }

        }

        std::string escaped_guard_y;

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
                escaped_guard_y = std::string("&lt;=") + " " + constant;

            } else if (comp_operator == "<") {

                guard_constraints.push_back(monitaal::constraint_t::upper_strict(2, c));
                escaped_guard_y = std::string("&lt;") + " " + constant;

            } else if (comp_operator == ">=") {

                guard_constraints.push_back(monitaal::constraint_t::lower_non_strict(2, c));
                escaped_guard_y = std::string("&gt;=") + " " + constant;

            } else { // comp_operator == ">"

                guard_constraints.push_back(monitaal::constraint_t::lower_strict(2, c));
                escaped_guard_y = std::string("&gt;") + " " + constant;

            }

        }

        bdd_edges.push_back(monitaal::bdd_edge_t(name_id_map.at(source), name_id_map.at(target), guard_constraints, reset_clocks, label));


        if (out_format.has_value() && !out_flatten) {

            if (out_format.value()) {   // tck

                bdd_allsat(label, *allsat_print_handler);

                std::stringstream p_constraint;
                std::stringstream p_assignments;
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
                            p_constraint << "p_" << j << (sat_paths[i][j] == '0' ? " % 2 == 0" : " >= 1");
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " && ";
                                p_assignments << "; ";
                            }
                        }
                    }
                    s += std::string("edge:") + "TA_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + ":ell_" + std::to_string(name_id_map.at(source)) + ":ell_" + std::to_string(name_id_map.at(target)) + ":a{provided: g == 0 && turn == " + std::to_string(components_counter)
                                        + (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                        + (guard_x.size() ? " && x_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " " + guard_x : std::string{})
                                        + (guard_y.size() ? " && y_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " " + guard_y : std::string{});
                    std::string reset_clocks_str;

                    if (reset == 1) {
                        reset_clocks_str = " : do: turn = " + std::to_string(components_counter + 1) + (p_assignments.str().size() ?  "; " + p_assignments.str() + "; " : "; ") + "x_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0}";
                    } else if (reset == 2) {
                        reset_clocks_str = " : do: turn = " + std::to_string(components_counter + 1) + (p_assignments.str().size() ?  "; " + p_assignments.str() + "; " : "; ") + "y_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0}";
                    } else if (reset == 3) {
                        reset_clocks_str = " : do: turn = " + std::to_string(components_counter + 1) + (p_assignments.str().size() ?  "; " + p_assignments.str() + "; " : "; ") + "x_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0; " + "y_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0}";
                    } else {
                        assert(reset == 0);
                        reset_clocks_str = " : do: turn = " + std::to_string(components_counter + 1) + (p_assignments.str().size() ?  "; " + p_assignments.str() + "}" : "}");
                    }
                    
                    s += reset_clocks_str + "\n"; 

                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();
                out_s << s;

            } else {        // xml

                bdd_allsat(label, *allsat_print_handler);

                std::stringstream p_constraint;
                std::stringstream p_assignments;
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
                            if (sat_paths[i][j] == '0') {
                                p_constraint << "(p_" << j << " == 2" << " || " << "p_" << j << " == 0)";
                            } else {
                                p_constraint << "p_" << j << " &gt;= 1";
                            }
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " &amp;&amp; ";
                                p_assignments << ", ";
                            }
                        }
                    }

                    p_constraint << (guard_x.size() ? " &amp;&amp; x_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " " + escaped_guard_x : std::string{})
                                 << (guard_y.size() ? " &amp;&amp; y_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " " + escaped_guard_y : std::string{});


                    if (reset == 1) {
                        p_assignments << (p_assignments.str().size() ? ", " : "") << "x_" << base_id << "_" << offset_id << " = 0";
                    } else if (reset == 2) {
                        p_assignments << (p_assignments.str().size() ? ", " : "") << "y_" << base_id << "_" << offset_id << " = 0";
                    } else if (reset == 3) {
                        p_assignments << (p_assignments.str().size() ? ", " : "") << "x_" << base_id << "_" << offset_id << " = 0"
                                      << ", y_" << base_id << "_" << offset_id << " = 0";
                    } else {
                        assert(reset == 0);
                    }
                    
                    s += "\t\t<transition>\n";
                    s += "\t\t\t<source ref=\"id0\"/>\n";
                    s += "\t\t\t<target ref=\"id0\"/>\n";

                    std::string provided_str;
                    provided_str = "g == 0 &amp;&amp; turn == " + std::to_string(components_counter) + " &amp;&amp; loc == " + std::to_string(name_id_map.at(source)) + (p_constraint.str().size() ? " &amp;&amp; " + p_constraint.str() : "");

                    s += "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                    std::string do_str;
                    do_str = "turn = " + std::to_string(components_counter + 1) + ", loc = " + std::to_string(name_id_map.at(target)) + (p_assignments.str().size() ? ", " + p_assignments.str() : "");

                    s += "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                    s += "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                    s += "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                    s += "\t\t</transition>\n";

                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();
                out_s << s;


            }

        }

        return;

    }
    

    std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_ta_from_atom(const MitlParser::AtomContext* phi_) {


        if (phi_->type == FINALLY) {

            return build_finally(phi_);

        } else if (phi_->type == ONCE) {

            return build_once(phi_);

        } else if (phi_->type == GLOBALLY) {

            return build_globally(phi_);

        } else if (phi_->type == HISTORICALLY) {

            return build_historically(phi_);

        } else if (phi_->type == UNTIL) {

            return build_until(phi_);

        } else if (phi_->type == SINCE) {

            return build_since(phi_);

        } else if (phi_->type == RELEASE) {

            return build_release(phi_);

        } else if (phi_->type == TRIGGER) {

            return build_trigger(phi_);

        } else if (phi_->type == PNUELIFN) {

            return build_pnuelifn(phi_);

        } else if (phi_->type == PNUELION) {

            return build_pnuelion(phi_);

        } else if (phi_->type == PNUELIGN) {

            return build_pnuelign(phi_);

        } else if (phi_->type == PNUELIHN) {

            return build_pnuelihn(phi_);

        } else if (phi_->type == COUNTFN) {

            return build_countfn(phi_);

        } else if (phi_->type == COUNTON) {

            return build_counton(phi_);

        } else if (phi_->type == COUNTGN) {

            return build_countgn(phi_);

        } else if (phi_->type == COUNTHN) {

            return build_counthn(phi_);

        } else {

            assert(("Unsupported atom type", false));

        }

        assert(false);

    }

    std::pair<monitaal::TA, std::string> build_ta_from_main(MitlParser::MainContext* phi_) {

        std::cout << "\nInput formula (getText() on the parse tree):\n" << std::endl;

        std::cout << phi_->getText() << std::endl;

        MitlTypingVisitor typing_visitor;

        typing_visitor.visitMain(phi_);

        MitlCheckNNFVisitor check_nnf_visitor;

        if (!std::any_cast<bool>(check_nnf_visitor.visitMain(phi_))) {

            std::cout << "\nThe input formula is not in negation normal form (i.e. AND and OR only, and all negations\n"
                      << "appear just before letters)." << std::endl;

            std::cout << "\nRewriting into NNF...\n";

        }

        std::string nnf_in;

        MitlToNNFVisitor to_nnf_visitor;

        nnf_in = std::any_cast<std::string>(to_nnf_visitor.visitMain(phi_));

        std::cout << "\nInput formula (in NNF):\n" << std::endl;

        std::cout << nnf_in << std::endl;

        std::cout << "\nRe-parsing...\n";

        antlr4::ANTLRInputStream nnf_input = antlr4::ANTLRInputStream(nnf_in);

        MitlLexer nnf_lexer(&nnf_input);
        antlr4::CommonTokenStream nnf_tokens(&nnf_lexer);

        MitlParser nnf_parser(&nnf_tokens);

        MitlParser::MainContext* nnf_formula = nnf_parser.main();

        typing_visitor.visitMain(nnf_formula);

        assert(std::any_cast<bool>(check_nnf_visitor.visitMain(nnf_formula)));

        std::cout << "\ntoStringTree:\n" << std::endl;

        std::cout << nnf_formula->toStringTree(&nnf_parser, true) << std::endl << std::endl;

        if (debug) {
            std::cout << "\nPlease confirm that there is no parsing error?\n";
            std::cout << "\nPress any key to continue . . .\n";
            std::cin.get();
        }

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
        for (const auto& [k, v] : nnf_formula->props) {
            std::cout << v << ": " << k << std::endl;
        }

        std::cout << std::endl;
        std::cout << "temporals:" << std::endl << std::endl;

        MitlCollectTemporalVisitor collect_temporal_visitor;
        std::set<MitlParser::AtomContext*, atom_cmp> temporal_atoms = std::any_cast<decltype(temporal_atoms)>(collect_temporal_visitor.visitMain(nnf_formula));
        for (auto it = temporal_atoms.rbegin(); it != temporal_atoms.rend(); ++it) {

            std::cout << (*it)->id << ": " << (*it)->getText() << std::endl;
            assert(nnf_formula->temporals.count((*it)->getText()));
            assert(nnf_formula->temporals[(*it)->getText()] == (*it)->id);

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
                } else if ((*it)->type == PNUELIGN) {
                    (*it)->id = ++i;
                    i = i + ((MitlParser::AtomGnContext*)(*it))->atoms.size() - 1;
                } else if ((*it)->type == PNUELIHN) {
                    (*it)->id = ++i;
                    i = i + ((MitlParser::AtomHnContext*)(*it))->atoms.size() - 1;
                } else {
                    (*it)->id = ++i;
                }

            }
        }
        */

        if (debug) {
            std::cout << "\nSee collected atoms above\n";
            std::cout << "\nPress any key to continue . . .\n";
            std::cin.get();
        }

        std::cout << "\nGenerating BDDs for labels...\n";

        bdd_setvarnum(num_all_props + 1);   // We leave the 0-th variable unused (temporal atoms and props start from 1)

        MitlGetBDDVisitor get_bdd_visitor;
        get_bdd_visitor.visitMain(nnf_formula);

        /*
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

            } else if ((*it)->type == PNUELIGN) {

                for (auto i = 0; i < ((MitlParser::AtomGnContext*)(*it))->atoms.size(); ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomGnContext*)(*it))->atoms[i]->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomGnContext*)(*it))->atoms[i]->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomGnContext*)(*it))->atoms[i]->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomGnContext*)(*it))->atoms[i]->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == PNUELIHN) {

                for (auto i = 0; i < ((MitlParser::AtomHnContext*)(*it))->atoms.size(); ++i) {

                    std::cout << "\nphi_" << i << ":\n";

                    std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomHnContext*)(*it))->atoms[i]->overline, *allsat_print_handler);
                    std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomHnContext*)(*it))->atoms[i]->star, *allsat_print_handler);
                    std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomHnContext*)(*it))->atoms[i]->tilde, *allsat_print_handler);
                    std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
                    bdd_allsat(((MitlParser::AtomHnContext*)(*it))->atoms[i]->hat, *allsat_print_handler);

                }

            } else if ((*it)->type == COUNTFN) {

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

            } else if ((*it)->type == COUNTON) {

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

            } else if ((*it)->type == COUNTGN) {

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

            } else if ((*it)->type == COUNTHN) {

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

            } else {
                assert(false);
            }

        }

        std::cout << "\n" << "TA_0" << ": " << nnf_formula->formula()->getText() << "\n" << std::endl;

        std::cout << std::setw(12) << "overline: " << std::setw(0) << std::endl;
        bdd_allsat(nnf_formula->formula()->overline, *allsat_print_handler);
        std::cout << std::setw(12) << "star: " << std::setw(0) << std::endl;
        bdd_allsat(nnf_formula->formula()->star, *allsat_print_handler);
        std::cout << std::setw(12) << "tilde: " << std::setw(0) << std::endl;
        bdd_allsat(nnf_formula->formula()->tilde, *allsat_print_handler);
        std::cout << std::setw(12) << "hat: " << std::setw(0) << std::endl;
        bdd_allsat(nnf_formula->formula()->hat, *allsat_print_handler);

        sat_paths.clear();

        if (debug) {

            std::cout << "\nSee the BDDs above\n";
            std::cout << "\nPress any key to continue . . .\n";
            std::cin.get();

        }
        
        */


        std::cout << "\nComputing GCD of constants...\n";

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            std::cout << (*it)->id << ": " << (*it)->getText() << std::endl;

            antlr4::tree::ParseTree* left;
            antlr4::tree::ParseTree* right;
            bool new_interval = false;

            if ((*it)->type == FINALLY) {

                MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == ONCE) {

                MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == GLOBALLY) {

                MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == HISTORICALLY) {

                MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == UNTIL) {

                MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == SINCE) {

                MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == RELEASE) {

                MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == TRIGGER) {

                MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == PNUELIFN) {

                MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == PNUELION) {

                MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == PNUELIGN) {

                MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == PNUELIHN) {

                MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == COUNTFN) {

                MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == COUNTON) {

                MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == COUNTGN) {

                MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else if ((*it)->type == COUNTHN) {

                MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);

                if (phi->interval() != nullptr) {
                    left = (antlr4::tree::ParseTree*)phi->interval()->children[1];
                    right = (antlr4::tree::ParseTree*)phi->interval()->children[3];
                    new_interval = true;
                }

            } else {
                assert(false);
            }

            if (new_interval) {

                if (left->children[0]->getText() != "0" && left->children[0]->getText() != "infty") {

                    if (gcd == 0) {

                        gcd = std::stoi(left->children[0]->getText());

                    } else {

                        gcd = std::gcd(gcd, std::stoi(left->children[0]->getText()));

                    }

                }

                if (right->children[0]->getText() != "0" && right->children[0]->getText() != "infty") {

                    if (gcd == 0) {

                        gcd = std::stoi(right->children[0]->getText());

                    } else {

                        gcd = std::gcd(gcd, std::stoi(right->children[0]->getText()));

                    }

                }

            }

        }

        std::cout << "\n<<<<<< Converting into TAs... >>>>>>\n\n";

        // auto div = monitaal::TA::time_divergence_ta(get_letters(std::string(num_all_props + 1, 'X')), true);

        components_counter = 2;     // TA_0 and TA_div
        size_t components_count;

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIGN || (*it)->type == PNUELIHN) {
                
                // TODO: need change if we add options to disable seq

                if ((*it)->type == PNUELIFN) {
                    components_counter = components_counter + ((out_flatten || comp_flatten) ? 1 : ((MitlParser::AtomFnContext*)(*it))->atoms.size() + 2);
                } else if ((*it)->type == PNUELION) {
                    components_counter = components_counter + ((out_flatten || comp_flatten) ? 1 : ((MitlParser::AtomOnContext*)(*it))->atoms.size() + 2);
                } else if ((*it)->type == PNUELIGN) {
                    components_counter = components_counter + ((out_flatten || comp_flatten) ? 1 : ((MitlParser::AtomGnContext*)(*it))->atoms.size() + 2);
                } else if ((*it)->type == PNUELIHN) {
                    components_counter = components_counter + ((out_flatten || comp_flatten) ? 1 : ((MitlParser::AtomHnContext*)(*it))->atoms.size() + 2);
                }

            } else if ((*it)->type == COUNTFN || (*it)->type == COUNTON || (*it)->type == COUNTGN || (*it)->type == COUNTHN) {
                components_counter = components_counter + ((out_flatten || comp_flatten) ? 1 : (*it)->num_pairs + 3);     // TODO: need change if we add options to disable seq
            } else {
                components_counter = components_counter + 1;
            }

        }

        ++components_counter;       // M

        components_count = components_counter;
        components_counter = 0;


        std::stringstream out_str;

        if (out_format.has_value() && !out_flatten) {

            if (out_format.value()) {

                out_str << "# File generated by MightyPPL" << std::endl;
                out_str << "system:model_and_spec" << std::endl << std::endl << std::endl;

                out_str << "event:a" << std::endl << std::endl << std::endl;

                out_str << "clock:1:g" << std::endl;
                out_str << "clock:1:x" << std::endl;        // for pinwheel

                for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

                    if ((*it)->type == FINALLY) {
                        MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == ONCE) {
                        MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == GLOBALLY) {
                        MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == HISTORICALLY) {
                        MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == UNTIL) {
                        MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == SINCE) {
                        MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == RELEASE) {
                        MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == TRIGGER) {
                        MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "clock:1:x_" << phi->id << "_0" << std::endl;
                        }
                    } else if ((*it)->type == PNUELIFN) {
                        MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        }
                    } else if ((*it)->type == PNUELION) {
                        MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        }
                    } else if ((*it)->type == PNUELIGN) {
                        MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        }
                    } else if ((*it)->type == PNUELIHN) {
                        MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        }
                    } else if ((*it)->type == COUNTFN) {
                        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                            }

                        } else {

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                                out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                            }
                            out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;

                        }
                    } else if ((*it)->type == COUNTON) {
                        MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);
                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                            }

                        } else {

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                                out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                            }
                            out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;

                        }
                    } else if ((*it)->type == COUNTGN) {
                        MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);
                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                            }

                        } else {

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                                out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                            }
                            out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;

                        }
                    } else if ((*it)->type == COUNTHN) {
                        MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);
                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                            }

                        } else {

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                                out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                            }
                            out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;

                        }
                    } else {
                        assert(false);
                    }

                }

                out_str << "clock:1:x_div" << std::endl;
                out_str << std::endl << std::endl;


                for (auto i = 0; i < num_all_props; ++i) {
                    out_str << "int:1:0:2:2:p_" << i + 1 << std::endl;
                }

                out_str << "int:1:0:" << components_count - 1 << ":0:turn" << std::endl;

            } else {

                out_str << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
                out_str << "<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.1//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_2.dtd'>" << std::endl;

                out_str << "<nta>" << std::endl << std::endl;

                out_str << "\t<declaration>" << std::endl;


                out_str << "\t\tclock g;" << std::endl;

                for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

                    if ((*it)->type == FINALLY) {
                        MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == ONCE) {
                        MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == GLOBALLY) {
                        MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == HISTORICALLY) {
                        MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == UNTIL) {
                        MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == SINCE) {
                        MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == RELEASE) {
                        MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == TRIGGER) {
                        MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)(*it);
                        if (phi->interval() != nullptr) {
                            out_str << "\t\tclock x_" << phi->id << "_0;" << std::endl;
                        }
                    } else if ((*it)->type == PNUELIFN) {
                        MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                        // When comp_flatten we still have the same number of clocks, but we may
                        // lose the structure (e.g., x_i_0 is not necessarily the one used by the first
                        // component TA

                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                        }
                    } else if ((*it)->type == PNUELION) {
                        MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                        }
                    } else if ((*it)->type == PNUELIGN) {
                        MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                        }
                    } else if ((*it)->type == PNUELIHN) {
                        MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)(*it);
                        for (auto i = 0; i < phi->atoms.size(); ++i) {
                            out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                        }
                    } else if ((*it)->type == COUNTFN) {
                        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);
                        
                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                            }

                        } else {

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                                out_str << "\t\tclock y_" << phi->id << "_" << i << ";" << std::endl;
                            }
                            out_str << "\t\tclock x_" << phi->id << "_" << phi->num_pairs << ";" << std::endl;

                        }
                    } else if ((*it)->type == COUNTON) {
                        MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                            }

                        } else { 

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                                out_str << "\t\tclock y_" << phi->id << "_" << i << ";" << std::endl;
                            }
                            out_str << "\t\tclock x_" << phi->id << "_" << phi->num_pairs << ";" << std::endl;

                        }
                    } else if ((*it)->type == COUNTGN) {
                        MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);

                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                            }

                        } else { 

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                                out_str << "\t\tclock y_" << phi->id << "_" << i << ";" << std::endl;
                            }
                            out_str << "\t\tclock x_" << phi->id << "_" << phi->num_pairs << ";" << std::endl;

                        }
                    } else if ((*it)->type == COUNTHN) {
                        MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);

                        if (comp_flatten) {

                            for (auto i = 0; i < 2 * phi->num_pairs + 1; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                            }

                        } else {

                            for (auto i = 0; i < phi->num_pairs; ++i) {
                                out_str << "\t\tclock x_" << phi->id << "_" << i << ";" << std::endl;
                                out_str << "\t\tclock y_" << phi->id << "_" << i << ";" << std::endl;
                            }
                            out_str << "\t\tclock x_" << phi->id << "_" << phi->num_pairs << ";" << std::endl;

                        }
                    } else {
                        assert(false);
                    }

                }

                out_str << "\t\tclock x_div;" << std::endl;
                out_str << std::endl << std::endl;


                for (auto i = 0; i < num_all_props; ++i) {
                    out_str << "\t\tint[0, 2] p_" << i + 1 << " = 2;" << std::endl;
                }

                out_str << "\t\tint[0, " << components_count - 1 << "] turn = 0;" << std::endl;


                out_str << "\t</declaration>" << std::endl << std::endl;

            }

        }


        std::cout << "\nGenerating TA_0" << "...\n";

        std::string name = "TA_0";

        monitaal::clock_map_t clocks;
        clocks.insert({0, "x0"});        // clock 0 is needed anyway

        bdd label;

        monitaal::constraints_t empty_invariant;
        monitaal::locations_t locations;
        monitaal::locations_t locations_;

        locations.push_back(monitaal::location_t(false, 0, "s0", empty_invariant));
        locations.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

        locations_.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));
        locations_.push_back(monitaal::location_t(true, 1, "s1", empty_invariant));

        monitaal::bdd_edges_t bdd_edges;
        monitaal::constraints_t guard;
        monitaal::clocks_t reset;

        // 0 -> 1, varphi

        label = nnf_formula->formula()->hat;

        bdd_edges.push_back(monitaal::bdd_edge_t(0, 1, guard, reset, label));

        // 1 -> 1, *varphi

        label = nnf_formula->formula()->star;

        bdd_edges.push_back(monitaal::bdd_edge_t(1, 1, guard, reset, label));

        varphi = monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0);   // last arg: initial location id
        varphi_ = monitaal::TAwithBDDEdges(name, clocks, locations_, bdd_edges, 0);   // last arg: initial location id
        clocks.clear();
        locations.clear();
        locations_.clear();
        bdd_edges.clear();

        if (out_format.has_value() && !out_flatten) {

            if (out_format.value()) {

                out_str << std::endl << std::endl;
                out_str << "# " << "TA_0" << std::endl;
                out_str << "# " << nnf_formula->formula()->getText() << std::endl;
                out_str << "process:" << "TA_0" << std::endl;

                out_str << "location:" << "TA_0" << ":ell_0{initial: }" << std::endl;
                out_str << "location:" << "TA_0" << ":ell_1{labels: accept_0}" << std::endl;

                bdd label;

                std::stringstream p_constraint;
                std::stringstream p_assignments;

                // 0 -> 1, varphi

                label = nnf_formula->formula()->hat;

                bdd_allsat(label, *allsat_print_handler);

                for (auto i = 0; i < sat_paths.size(); ++i) {

                    int largest = 0;
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X' && j > largest) {
                            largest = j;
                        }
                    }
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X') {
                            p_constraint << "p_" << j << (sat_paths[i][j] == '0' ? " % 2 == 0" : " >= 1");
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " && ";
                                p_assignments << "; ";
                            }
                        }
                    }

                    out_str << "edge:" << "TA_0" << ":ell_0:ell_1:a{provided: turn == " << components_counter << (p_constraint.str().size() ? " && " + p_constraint.str() : "") << " : do: g = 0; turn = " << components_counter + 1 << (p_assignments.str().size() ? "; " + p_assignments.str() : "") << "}" << std::endl;

                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();

                // 1 -> 1, *varphi

                label = nnf_formula->formula()->star;

                bdd_allsat(label, *allsat_print_handler);

                for (auto i = 0; i < sat_paths.size(); ++i) {

                    int largest = 0;
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X' && j > largest) {
                            largest = j;
                        }
                    }
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X') {
                            p_constraint << "p_" << j << (sat_paths[i][j] == '0' ? " % 2 == 0" : " >= 1");
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " && ";
                                p_assignments << "; ";
                            }
                        }
                    }

                    out_str << "edge:" << "TA_0" << ":ell_1:ell_1:a{provided: turn == " << components_counter << (p_constraint.str().size() ? " && " + p_constraint.str() : "") << " : do: g = 0; turn = " << components_counter + 1 << (p_assignments.str().size() ? "; " + p_assignments.str() : "") << "}" << std::endl;

                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();

            } else {

                out_str << "\t<template>" << std::endl;
                out_str << "\t\t<name>" << "TA_0" << "</name>" << std::endl;

                out_str << "\t\t<declaration>" << std::endl;

                out_str << "\t\t\tint[0, 1] loc = 0;" << std::endl;

                out_str << "\t\t</declaration>" << std::endl;

                out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                out_str << "\t\t</location>" << std::endl;
                out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                bdd label;

                std::stringstream p_constraint;
                std::stringstream p_assignments;

                // 0 -> 1, varphi

                label = nnf_formula->formula()->hat;

                bdd_allsat(label, *allsat_print_handler);

                for (auto i = 0; i < sat_paths.size(); ++i) {

                    int largest = 0;
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X' && j > largest) {
                            largest = j;
                        }
                    }
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X') {
                            if (sat_paths[i][j] == '0') {
                                p_constraint << "(p_" << j << " == 2" << " || " << "p_" << j << " == 0)";
                            } else {
                                p_constraint << "p_" << j << " &gt;= 1";
                            }
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " &amp;&amp; ";
                                p_assignments << ", ";
                            }
                        }
                    }

                    out_str << "\t\t<transition>\n";
                    out_str << "\t\t\t<source ref=\"id0\"/>\n";
                    out_str << "\t\t\t<target ref=\"id0\"/>\n";

                    std::string provided_str;
                    provided_str = "turn == " + std::to_string(components_counter) + " &amp;&amp; loc == 0" + (p_constraint.str().size() ? " &amp;&amp; " + p_constraint.str() : "");

                    out_str << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                    std::string do_str;
                    do_str = "g = 0, turn = " + std::to_string(components_counter + 1) + ", loc = 1" + (p_assignments.str().size() ? ", " + p_assignments.str() : "");

                    out_str << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                    out_str << "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                    out_str << "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                    out_str << "\t\t</transition>\n";

                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();

                // 1 -> 1, *varphi

                label = nnf_formula->formula()->star;

                bdd_allsat(label, *allsat_print_handler);

                for (auto i = 0; i < sat_paths.size(); ++i) {

                    int largest = 0;
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X' && j > largest) {
                            largest = j;
                        }
                    }
                    for (auto j = 1; j < sat_paths[i].size(); ++j) {
                        if (sat_paths[i][j] != 'X') {
                            if (sat_paths[i][j] == '0') {
                                p_constraint << "(p_" << j << " == 2" << " || " << "p_" << j << " == 0)";
                            } else {
                                p_constraint << "p_" << j << " &gt;= 1";
                            }
                            p_assignments << "p_" << j << (sat_paths[i][j] == '0' ? " = 0" : " = 1");
                            if (j != largest) {
                                p_constraint << " &amp;&amp; ";
                                p_assignments << ", ";
                            }
                        }
                    }

                    out_str << "\t\t<transition>\n";
                    out_str << "\t\t\t<source ref=\"id0\"/>\n";
                    out_str << "\t\t\t<target ref=\"id0\"/>\n";

                    std::string provided_str;
                    provided_str = "turn == " + std::to_string(components_counter) + " &amp;&amp; loc == 1" + (p_constraint.str().size() ? " &amp;&amp; " + p_constraint.str() : "");

                    out_str << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                    std::string do_str;
                    do_str = "g = 0, turn = " + std::to_string(components_counter + 1) + ", loc = 1" + (p_assignments.str().size() ? ", " + p_assignments.str() : "");

                    out_str << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                    out_str << "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                    out_str << "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                    out_str << "\t\t</transition>\n";


                    std::stringstream().swap(p_constraint);
                    std::stringstream().swap(p_assignments);

                }

                sat_paths.clear();

                out_str << "\t</template>" << std::endl << std::endl;

            }

        }

        ++components_counter;

        std::cout << std::endl;

        std::cout << std::setw(20) << varphi.name() << std::endl;
        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << varphi.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << varphi.number_of_clocks() - 2 << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto& [k, v] : varphi.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << varphi.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << varphi.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }

        if (debug) {

            std::cout << "\nSee the component above\n";
            std::cout << "\nPress any key to continue . . .\n";
            std::cin.get();

        }


        std::cout << "\nGenerating TA_div" << "...\n";

        div = monitaal::TAwithBDDEdges::time_divergence_ta(bdd_true());

        if (out_format.has_value() && !out_flatten) {

            if (out_format.value()) {

                out_str << std::endl << std::endl;
                out_str << "# " << "TA_div" << std::endl;
                out_str << "process:" << "TA_div" << std::endl;

                out_str << "location:" << "TA_div" << ":ell_0{initial: : labels: accept_div}" << std::endl;
                out_str << "location:" << "TA_div" << ":ell_1{" << (out_fin ? "labels: accept_div" : "") << "}" << std::endl;

                out_str << "edge:" << "TA_div" << ":ell_0:ell_1:a{provided: g == 0 && turn == " << components_counter << " : do: turn = " << components_counter + 1 << "}" << std::endl;

                out_str << "edge:" << "TA_div" << ":ell_1:ell_1:a{provided: g == 0 && turn == " << components_counter << " && x_div < " << gcd << " : do: turn = " << components_counter + 1 << "}" << std::endl;

                out_str << "edge:" << "TA_div" << ":ell_1:ell_0:a{provided: g == 0 && turn == " << components_counter << " && x_div >= " << gcd << " : do: turn = " << components_counter + 1 << "; x_div = 0}" << std::endl;

            } else {

                out_str << "\t<template>" << std::endl;
                out_str << "\t\t<name>" << "TA_div" << "</name>" << std::endl;

                out_str << "\t\t<declaration>" << std::endl;

                out_str << "\t\t\tint[0, 1] loc = 0;" << std::endl;

                out_str << "\t\t</declaration>" << std::endl;

                out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                out_str << "\t\t</location>" << std::endl;
                out_str << "\t\t<init ref=\"id0\"/>" << std::endl;

                std::string provided_str;
                std::string do_str;

                out_str << "\t\t<transition>\n";
                out_str << "\t\t\t<source ref=\"id0\"/>\n";
                out_str << "\t\t\t<target ref=\"id0\"/>\n";

                provided_str = "g == 0 &amp;&amp; turn == " + std::to_string(components_counter) + " &amp;&amp; loc == 0";

                out_str << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                do_str = "turn = " + std::to_string(components_counter + 1) + ", loc = 1";

                out_str << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                out_str << "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                out_str << "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                out_str << "\t\t</transition>\n";


                out_str << "\t\t<transition>\n";
                out_str << "\t\t\t<source ref=\"id0\"/>\n";
                out_str << "\t\t\t<target ref=\"id0\"/>\n";

                provided_str = "g == 0 &amp;&amp; turn == " + std::to_string(components_counter) + " &amp;&amp; loc == 1 &amp;&amp; x_div &lt; " + std::to_string(gcd);

                out_str << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                do_str = "turn = " + std::to_string(components_counter + 1) + ", loc = 1";

                out_str << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                out_str << "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                out_str << "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                out_str << "\t\t</transition>\n";


                out_str << "\t\t<transition>\n";
                out_str << "\t\t\t<source ref=\"id0\"/>\n";
                out_str << "\t\t\t<target ref=\"id0\"/>\n";

                provided_str = "g == 0 &amp;&amp; turn == " + std::to_string(components_counter) + " &amp;&amp; loc == 1 &amp;&amp; x_div &gt;= " + std::to_string(gcd);

                out_str << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                do_str = "turn = " + std::to_string(components_counter + 1) + ", loc = 0, x_div = 0";

                out_str << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                out_str << "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                out_str << "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                out_str << "\t\t</transition>\n";

                out_str << "\t</template>" << std::endl << std::endl;

            }

        }

        ++components_counter;

        std::cout << std::endl;

        std::cout << std::setw(20) << div.name() << std::endl;
        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << div.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << div.number_of_clocks() - 2 << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto& [k, v] : div.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << div.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << div.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }

        if (debug) {

            std::cout << "\nSee the component above\n";
            std::cout << "\nPress any key to continue . . .\n";
            std::cin.get();

        }

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIGN || (*it)->type == PNUELIHN
                    || (*it)->type == COUNTFN || (*it)->type == COUNTON || (*it)->type == COUNTGN || (*it)->type == COUNTHN) {
                std::cout << "\nGenerating TA_" << (*it)->id << "_0 (and other components)...\n";
            } else {
                std::cout << "\nGenerating TA_" << (*it)->id << "_0...\n";
            }
            auto [ generated_components, component_str ] = build_ta_from_atom(*it);

            temporal_components.insert(temporal_components.end(), generated_components.begin(), generated_components.end());

            if (out_format.has_value() && !out_flatten) {

                out_str << component_str;

            }

            // components_counter = components_counter + generated_components.size();

            // if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIGN || (*it)->type == PNUELIHN
            //         || (*it)->type == COUNTFN || (*it)->type == COUNTON || (*it)->type == COUNTGN || (*it)->type == COUNTHN) {
            //     std::cout << "\nGenerated " << generated_components.size() << " components\n";
            // } else {
            //     std::cout << "\nGenerated TA_" << (*it)->id << "_0\n";
            // }

            for (auto it = generated_components.begin(); it != generated_components.end(); ++it) {

                std::cout << std::endl;

                std::cout << std::setw(20) << (*it).name() << std::endl;
                std::cout << std::setw(20) << "# of locations: " << std::setw(10) << (*it).locations().size() << std::setw(0) << std::endl;
                std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << (*it).number_of_clocks() - 2 << std::setw(0) << std::endl;

                // for (auto i = 0; i < temporal_components.back().number_of_clocks(); ++i) {
                //     std::cout << std::setw(20) << temporal_components.back().clock_name(i) << std::setw(0) << std::endl;
                // }

                std::cout << std::endl;

                for (const auto& [k, v] : (*it).locations()) {

                    std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
                    std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << (*it).bdd_edges_from(k).size() << std::setw(0) << std::endl;
                    // for (const auto& e : (*it).bdd_edges_from(k)) {
                    //     std::cout << e.from() << " -> " << e.to() << ": " << std::endl;
                    //     bdd_printset(e.bdd_label());
                    //     std::cout << std::endl;
                    // }
                    std::cout << std::setw(20) << "# incoming: " << std::setw(10) << (*it).bdd_edges_to(k).size() << std::setw(0) << std::endl;

                }

                if (debug) {

                    std::cout << "\nSee the component above\n";
                    std::cout << "\nPress any key to continue . . .\n";
                    std::cin.get();

                }

            }

        }


        std::cout << "\nGenerating M" << "...\n";

        name = "M";

        clocks.insert({0, "x0"});        // clock 0 is needed anyway
        clocks.insert({1, "x"});        // for pinwheel

        locations.push_back(monitaal::location_t(true, 0, "s0", empty_invariant));

        label = bdd_ithvar(nnf_formula->props.at("p1")) & !bdd_ithvar(nnf_formula->props.at("p2")) & !bdd_ithvar(nnf_formula->props.at("p3")) & !bdd_ithvar(nnf_formula->props.at("p4"));
        reset.push_back(1);
        guard.push_back(monitaal::constraint_t::lower_non_strict(1, 1));
        bdd_edges.push_back(monitaal::bdd_edge_t(0, 0, guard, reset, label));
        guard.clear();
        reset.clear();

        label = !bdd_ithvar(nnf_formula->props.at("p1")) & bdd_ithvar(nnf_formula->props.at("p2")) & !bdd_ithvar(nnf_formula->props.at("p3")) & !bdd_ithvar(nnf_formula->props.at("p4"));
        reset.push_back(1);
        guard.push_back(monitaal::constraint_t::lower_non_strict(1, 1));
        bdd_edges.push_back(monitaal::bdd_edge_t(0, 0, guard, reset, label));
        guard.clear();
        reset.clear();

        label = !bdd_ithvar(nnf_formula->props.at("p1")) & !bdd_ithvar(nnf_formula->props.at("p2")) & bdd_ithvar(nnf_formula->props.at("p3")) & !bdd_ithvar(nnf_formula->props.at("p4"));
        reset.push_back(1);
        guard.push_back(monitaal::constraint_t::lower_non_strict(1, 1));
        bdd_edges.push_back(monitaal::bdd_edge_t(0, 0, guard, reset, label));
        guard.clear();
        reset.clear();

        label = !bdd_ithvar(nnf_formula->props.at("p1")) & !bdd_ithvar(nnf_formula->props.at("p2")) & !bdd_ithvar(nnf_formula->props.at("p3")) & bdd_ithvar(nnf_formula->props.at("p4"));
        reset.push_back(1);
        guard.push_back(monitaal::constraint_t::lower_non_strict(1, 1));
        bdd_edges.push_back(monitaal::bdd_edge_t(0, 0, guard, reset, label));
        guard.clear();
        reset.clear();


        model = monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0);   // last arg: initial location id
        clocks.clear();
        locations.clear();
        bdd_edges.clear();


        if (out_format.has_value() && !out_flatten) {

            if (out_format.value()) {

                out_str << std::endl << std::endl;
                out_str << "# " << "M" << std::endl;
                out_str << "process:" << "M" << std::endl;

                out_str << "location:" << "M" << ":ell_0{initial: : labels: accept_M}" << std::endl;

                out_str << "edge:" << "M" << ":ell_0:ell_0:a{provided: g == 0 && turn == " << components_counter
                        << " && p_" << std::to_string(nnf_formula->props.at("p1")) << " >= 1"
                        << " && p_" << std::to_string(nnf_formula->props.at("p2")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p3")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p4")) << " % 2 == 0"
                        << " && x >= 1 : do: x = 0; turn = 0; ";
                for (auto i = 0; i < num_all_props; ++i) {
                    out_str << "p_" << i + 1 << " = 2" << (i == num_all_props - 1 ? "" : "; ");
                }
                out_str << "}" << std::endl;

                out_str << "edge:" << "M" << ":ell_0:ell_0:a{provided: g == 0 && turn == " << components_counter
                        << " && p_" << std::to_string(nnf_formula->props.at("p1")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p2")) << " >= 1"
                        << " && p_" << std::to_string(nnf_formula->props.at("p3")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p4")) << " % 2 == 0"
                        << " && x >= 1 : do: x = 0; turn = 0; ";
                for (auto i = 0; i < num_all_props; ++i) {
                    out_str << "p_" << i + 1 << " = 2" << (i == num_all_props - 1 ? "" : "; ");
                }
                out_str << "}" << std::endl;

                out_str << "edge:" << "M" << ":ell_0:ell_0:a{provided: g == 0 && turn == " << components_counter
                        << " && p_" << std::to_string(nnf_formula->props.at("p1")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p2")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p3")) << " >= 1"
                        << " && p_" << std::to_string(nnf_formula->props.at("p4")) << " % 2 == 0"
                        << " && x >= 1 : do: x = 0; turn = 0; ";
                for (auto i = 0; i < num_all_props; ++i) {
                    out_str << "p_" << i + 1 << " = 2" << (i == num_all_props - 1 ? "" : "; ");
                }
                out_str << "}" << std::endl;

                out_str << "edge:" << "M" << ":ell_0:ell_0:a{provided: g == 0 && turn == " << components_counter
                        << " && p_" << std::to_string(nnf_formula->props.at("p1")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p2")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p3")) << " % 2 == 0"
                        << " && p_" << std::to_string(nnf_formula->props.at("p4")) << " >= 1"
                        << " && x >= 1 : do: x = 0; turn = 0; ";
                for (auto i = 0; i < num_all_props; ++i) {
                    out_str << "p_" << i + 1 << " = 2" << (i == num_all_props - 1 ? "" : "; ");
                }
                out_str << "}" << std::endl;

            } else {

                out_str << "\t<template>" << std::endl;
                out_str << "\t\t<name>" << "M" << "</name>" << std::endl;

                out_str << "\t\t<declaration>" << std::endl;

                out_str << "\t\t\tint[0, 0] loc = 0;" << std::endl;

                out_str << "\t\t</declaration>" << std::endl;

                out_str << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                out_str << "\t\t</location>" << std::endl;
                out_str << "\t\t<init ref=\"id0\"/>" << std::endl;


                out_str << "\t\t<transition>\n";
                out_str << "\t\t\t<source ref=\"id0\"/>\n";
                out_str << "\t\t\t<target ref=\"id0\"/>\n";

                std::string provided_str;
                provided_str = "g == 0 &amp;&amp; turn == " + std::to_string(components_counter) + " &amp;&amp; loc == 0";

                out_str << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" + provided_str + "</label>\n";

                std::string do_str;
                do_str = "turn = 0, loc = 0, ";
                for (auto i = 0; i < num_all_props; ++i) {
                    do_str += "p_" + std::to_string(i + 1) + " = 2" + (i == num_all_props - 1 ? "" : ", ");
                }

                out_str << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" + do_str + "</label>\n";

                out_str << "\t\t\t<nail x=\"-102\" y=\"34\"/>\n";
                out_str << "\t\t\t<nail x=\"-102\" y=\"-34\"/>\n";
                out_str << "\t\t</transition>\n";

                out_str << "\t</template>" << std::endl << std::endl;


            }

        }

        ++components_counter;

        // std::cout << "components_counter == " << components_counter << std::endl;
        // std::cout << "components_count == " << components_count << std::endl;
        assert(components_counter == components_count);

        std::cout << std::endl;

        std::cout << std::setw(20) << model.name() << std::endl;
        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << model.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << model.number_of_clocks() - 2 << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto& [k, v] : model.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << model.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << model.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }

        if (debug) {

            std::cout << "\nSee the component above\n";
            std::cout << "\nPress any key to continue . . .\n";
            std::cin.get();

        }


        if (out_format.has_value() && out_format.value() && !out_flatten) {     // TA_sync is not needed for XML output
            
            if (out_fin) {

                std::cout << "\nGenerating TA_sync" << "...\n";

                out_str << std::endl << std::endl;
                out_str << "# " << "TA_sync" << std::endl;
                out_str << "process:" << "TA_sync" << std::endl;

                out_str << "location:" << "TA_sync" << ":ell_0{initial: : labels: accept_sync}" << std::endl;
                out_str << "location:" << "TA_sync" << ":ell_1{}" << std::endl;

                out_str << "edge:" << "TA_sync" << ":ell_0:ell_1:a{provided: turn != " << components_counter - 1 << "}" << std::endl;
                out_str << "edge:" << "TA_sync" << ":ell_1:ell_1:a{provided: turn != " << components_counter - 1 << "}" << std::endl;
                out_str << "edge:" << "TA_sync" << ":ell_1:ell_0:a{provided: turn == " << components_counter - 1 << "}" << std::endl;

                std::cout << "\nGenerating sync constraints" << "...\n";

                out_str << std::endl << std::endl;
                out_str << "# " << "sync constraints" << std::endl;
                out_str << "sync:TA_sync@a:TA_0@a" << std::endl;
                out_str << "sync:TA_sync@a:TA_div@a" << std::endl;

                for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {


                    if (comp_flatten) {

                        out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << "0" << "@a" << std::endl;

                    } else {

                        if ((*it)->type == PNUELIFN) {

                            MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                        } else if ((*it)->type == PNUELION) {

                            MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                        } else if ((*it)->type == PNUELIGN) {

                            MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                        } else if ((*it)->type == PNUELIHN) {

                            MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                        } else if ((*it)->type == COUNTFN) {

                            MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                            for (auto i = 0; i < (*it)->num_pairs; ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                            out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a" << std::endl;

                        } else if ((*it)->type == COUNTON) {

                            MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                            for (auto i = 0; i < (*it)->num_pairs; ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                            out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a" << std::endl;

                        } else if ((*it)->type == COUNTGN) {

                            MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);

                            for (auto i = 0; i < (*it)->num_pairs; ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                            out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a" << std::endl;

                        } else if ((*it)->type == COUNTHN) {

                            MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);

                            for (auto i = 0; i < (*it)->num_pairs; ++i) {

                                out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << i << "@a" << std::endl;

                            }

                            out_str << "sync:TA_sync@a:seq_in_" << (*it)->id << "@a" << std::endl;
                            out_str << "sync:TA_sync@a:seq_out_" << (*it)->id << "@a" << std::endl;

                            out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a" << std::endl;

                        } else {

                            out_str << "sync:TA_sync@a:TA_" << (*it)->id << "_" << "0" << "@a" << std::endl;

                        }

                    }

                }

                out_str << "sync:TA_sync@a:M@a" << std::endl;

                out_str << std::endl;

            }

        }


        if (!out_format.has_value() || (out_format.has_value() && out_flatten)) {

            std::cout << "\n<<<<<< Taking intersection... >>>>>>\n\n";

            back = false;
            last_intersection = true;

            std::vector<monitaal::TAwithBDDEdges> automata = temporal_components;
            automata.insert(automata.begin(), div);
            automata.insert(automata.begin(), varphi);
            automata.insert(automata.end(), model);

            monitaal::TAwithBDDEdges product = monitaal::TAwithBDDEdges::intersection(automata);

            // std::cout << product;
            
            std::cout << std::endl;

            std::cout << std::setw(20) << "# of locations: " << std::setw(10) << product.locations().size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << product.number_of_clocks() - 2 << std::setw(0) << std::endl;

            std::cout << std::endl;

            for (const auto& [k, v] : product.locations()) {

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

                out_str << "\n# " << "Please use the following command to check satisfiability:\n\n";
                std::cout << "\nPlease use the following command to check satisfiability:\n\n";
                if (out_fin) {
                    out_str << "# " << "tck-reach -a covreach -l ";
                    std::cout << "tck-reach -a covreach -l ";
                } else {
                    out_str << "# " << "tck-liveness -a couvscc -l ";
                    std::cout << "tck-liveness -a couvscc -l ";
                }

                out_str << "accept_0,accept_div,";
                std::cout << "accept_0,accept_div,";

                for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

                    if (comp_flatten) {

                        out_str << "accept_" << (*it)->id << ",";
                        std::cout << "accept_" << (*it)->id << ",";

                    } else {

                        if ((*it)->type == PNUELIFN) {

                            MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";

                        } else if ((*it)->type == PNUELION) {

                            MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";

                        } else if ((*it)->type == PNUELIGN) {

                            MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";

                        } else if ((*it)->type == PNUELIHN) {

                            MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)(*it);

                            for (auto i = 0; i < phi->atoms.size(); ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";

                        } else if ((*it)->type == COUNTFN) {

                            MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                            for (auto i = 0; i < phi->num_pairs; ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";
                            out_str << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";
                            std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                        } else if ((*it)->type == COUNTON) {

                            MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                            for (auto i = 0; i < phi->num_pairs; ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";
                            out_str << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";
                            std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                        } else if ((*it)->type == COUNTGN) {

                            MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);

                            for (auto i = 0; i < phi->num_pairs; ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";
                            out_str << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";
                            std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                        } else if ((*it)->type == COUNTHN) {

                            MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);

                            for (auto i = 0; i < phi->num_pairs; ++i) {

                                out_str << "accept_" << (*it)->id << "_" << i << ",";
                                std::cout << "accept_" << (*it)->id << "_" << i << ",";

                            }

                            out_str << "accept_seq_in_" << (*it)->id << ",";
                            std::cout << "accept_seq_in_" << (*it)->id << ",";
                            out_str << "accept_seq_out_" << (*it)->id << ",";
                            std::cout << "accept_seq_out_" << (*it)->id << ",";
                            out_str << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";
                            std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                        } else {

                            out_str << "accept_" << (*it)->id << ",";
                            std::cout << "accept_" << (*it)->id << ",";

                        }

                    }

                }

                out_str << "accept_M" << (out_fin ? ",accept_sync " : " ") << out_file << std::endl;
                std::cout << "accept_M" << (out_fin ? ",accept_sync " : " ") << out_file << std::endl;

                // return no TA, but the generated output for components

                return { monitaal::TA("dummy", {}, {}, {}, 0), out_str.str() };

            } else {
                
                // return no TA, but the generated output for components

                return { monitaal::TA("dummy", {}, {}, {}, 0), out_str.str() };

            }

        }


        assert(false);

    }

} // namespace mightypplcpp
