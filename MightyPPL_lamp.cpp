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

    void allsat_print_handler(char* varset, int size) {

        std::string output;

        for (int v = 0; v < size; ++v) {
            output += (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
        }
        //std::cout << std::setw(20) << output << std::setw(0) << std::endl;

        sat_paths.push_back(output);

    }


    
    // TODO: refactoring to avoid overlap with build_edge() below

    void build_untimed_edge(monitaal::bdd_edges_t& bdd_edges, const std::map<std::string, monitaal::location_id_t>& name_id_map, std::stringstream& out_s, const std::string& automaton_name, const std::string& source, const std::string& target, bdd label) {

        bdd_edges.push_back(monitaal::bdd_edge_t(name_id_map.at(source), name_id_map.at(target), monitaal::constraints_t{}, monitaal::clocks_t{}, label));

        if (out_format.has_value() && out_format.value() && !out_flatten) {

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
                s = s + std::string("edge:") + automaton_name + ":" + source + ":" + target + ":a{provided: turn == 1"
                                    + (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                    + "}" 
                                    + "\n"; 

                std::stringstream().swap(p_constraint);

            }

            sat_paths.clear();
            out_s << s;

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


        if (out_format.has_value() && out_format.value() && !out_flatten) {

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
                s = s + std::string("edge:") + "TA_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + ":ell_" + source + ":ell_" + target + ":a{provided: turn == 1"
                                    + (p_constraint.str().size() ? " && " + p_constraint.str() : std::string{})
                                    + (guard_x.size() ? " && x_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " " + guard_x : std::string{})
                                    + (guard_y.size() ? " && y_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " " + guard_y : std::string{});
                std::string reset_clocks_str;

                if (reset == 1) {
                    reset_clocks_str = " : do: x_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0}";
                } else if (reset == 2) {
                    reset_clocks_str = " : do: y_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0}";
                } else if (reset == 3) {
                    reset_clocks_str = " : do: x_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0; " + "y_" + std::to_string(base_id) + "_" + std::to_string(offset_id) + " = 0}";
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
        for (const auto & [k, v] : nnf_formula->props) {
            std::cout << v << ": " << k << std::endl;
        }

        std::cout << std::endl;
        std::cout << "temporals:" << std::endl << std::endl;

        std::set<MitlParser::AtomContext*, atom_cmp> temporal_atoms;
        MitlCollectTemporalVisitor collect_temporal_visitor;
        temporal_atoms = std::any_cast<decltype(temporal_atoms)>(collect_temporal_visitor.visitMain(nnf_formula));
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

        std::cout << "\n<<<<<< Converting into TAs... >>>>>>\n\n";

        // auto div = monitaal::TA::time_divergence_ta(get_letters(std::string(num_all_props + 1, 'X')), true);

        auto div = monitaal::TAwithBDDEdges::time_divergence_ta(bdd_true());

        std::vector<monitaal::TAwithBDDEdges> temporal_components;
        std::stringstream out_str;

        if (out_format.has_value() && out_format.value() && !out_flatten) {

            out_str << "# File generated by MightyPPL" << std::endl;
            out_str << "system:model_and_spec" << std::endl << std::endl << std::endl;

            out_str << "event:a" << std::endl << std::endl << std::endl;

            out_str << "clock:1:x" << std::endl;        // for timed lamp
            out_str << "clock:1:y" << std::endl;

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
                    for (auto i = 0; i < phi->num_pairs; ++i) {
                        out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                    }
                    out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;
                } else if ((*it)->type == COUNTON) {
                    MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);
                    for (auto i = 0; i < phi->num_pairs; ++i) {
                        out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                    }
                    out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;
                } else if ((*it)->type == COUNTGN) {
                    MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);
                    for (auto i = 0; i < phi->num_pairs; ++i) {
                        out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                    }
                    out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;
                } else if ((*it)->type == COUNTHN) {
                    MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);
                    for (auto i = 0; i < phi->num_pairs; ++i) {
                        out_str << "clock:1:x_" << phi->id << "_" << i << std::endl;
                        out_str << "clock:1:y_" << phi->id << "_" << i << std::endl;
                    }
                    out_str << "clock:1:x_" << phi->id << "_" << phi->num_pairs << std::endl;
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

        label = nnf_formula->formula()->hat;

        bdd_edges.push_back(monitaal::bdd_edge_t(0, 1, guard, reset, label));

        // 1 -> 1, *varphi

        label = nnf_formula->formula()->star;

        bdd_edges.push_back(monitaal::bdd_edge_t(1, 1, guard, reset, label));

        varphi = monitaal::TAwithBDDEdges(name, clocks, locations, bdd_edges, 0);   // last arg: initial location id
        clocks.clear();
        locations.clear();
        bdd_edges.clear();

        if (out_format.has_value() && out_format.value() && !out_flatten) {

            out_str << std::endl << std::endl;
            out_str << "# " << "TA_0" << std::endl;
            out_str << "# " << nnf_formula->formula()->getText() << std::endl;
            out_str << "process:" << "TA_0" << std::endl;

            out_str << "location:" << "TA_0" << ":ell_0{initial: }" << std::endl;
            out_str << "location:" << "TA_0" << ":ell_1{labels: accept_0}" << std::endl;

            bdd label;

            // 0 -> 1, varphi

            label = nnf_formula->formula()->hat;

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

            label = nnf_formula->formula()->star;

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

        std::cout << std::endl;

        std::cout << std::setw(20) << "# of locations: " << std::setw(10) << varphi.locations().size() << std::setw(0) << std::endl;
        std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << varphi.number_of_clocks() << std::setw(0) << std::endl;

        std::cout << std::endl;

        for (const auto & [k, v] : varphi.locations()) {

            std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? " *ACCEPTING*" : "") << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << varphi.bdd_edges_from(k).size() << std::setw(0) << std::endl;
            std::cout << std::setw(20) << "# incoming: " << std::setw(10) << varphi.bdd_edges_to(k).size() << std::setw(0) << std::endl;

        }

        if (debug) {

            std::cout << "\nSee the component above\n";
            std::cout << "\nPress any key to continue . . .\n";
            std::cin.get();

        }

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIGN || (*it)->type == PNUELIHN
                    || (*it)->type == COUNTFN || (*it)->type == COUNTON || (*it)->type == COUNTGN || (*it)->type == COUNTHN) {
                std::cout << "\nGenerating TA_" << (*it)->id << " (and other sub-components)...\n";
            } else {
                std::cout << "\nGenerating TA_" << (*it)->id << "...\n";
            }
            auto [ generated_components, component_str ] = build_ta_from_atom(*it);
            temporal_components.insert(temporal_components.end(), generated_components.begin(), generated_components.end());

            if (out_format.has_value() && out_format.value() && !out_flatten) {

                out_str << component_str;

            }

            if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIGN || (*it)->type == PNUELIHN
                    || (*it)->type == COUNTFN || (*it)->type == COUNTON || (*it)->type == COUNTGN || (*it)->type == COUNTHN) {
                std::cout << "\nGenerated TA_" << (*it)->id << " (and other sub-components)\n";
            } else {
                std::cout << "\nGenerated TA_" << (*it)->id << "\n";
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

            if (debug) {

                std::cout << "\nSee the component above\n";
                std::cout << "\nPress any key to continue . . .\n";
                std::cin.get();

            }

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

        if (out_format.has_value() && out_format.value() && !out_flatten) {

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

        if (out_format.has_value() && out_format.value() && !out_flatten) {

            // std::cout << "\nGenerating sync constraints" << "...\n";

            out_str << std::endl << std::endl;
            out_str << "# " << "sync constraints" << std::endl;
            out_str << "sync:TA_div@a:TA_0@a" << std::endl;

            out_str << "sync:TA_div@a:M@a:";

            for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {


                if ((*it)->type == PNUELIFN) {

                    MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";

                } else if ((*it)->type == PNUELION) {

                    MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";

                } else if ((*it)->type == PNUELIGN) {

                    MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)(*it);

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";

                } else if ((*it)->type == PNUELIHN) {

                    MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)(*it);

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";

                } else if ((*it)->type == COUNTFN) {

                    MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                    for (auto i = 0; i < (*it)->num_pairs; ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a";

                } else if ((*it)->type == COUNTON) {

                    MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                    for (auto i = 0; i < (*it)->num_pairs; ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a";

                } else if ((*it)->type == COUNTGN) {

                    MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);

                    for (auto i = 0; i < (*it)->num_pairs; ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a";

                } else if ((*it)->type == COUNTHN) {

                    MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);

                    for (auto i = 0; i < (*it)->num_pairs; ++i) {

                        out_str << "TA_" << (*it)->id << "_" << i << "@a";
                        out_str << ":";

                    }

                    out_str << "seq_in_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "seq_out_" << (*it)->id << "@a";
                    out_str << ":";
                    out_str << "TA_" << (*it)->id << "_" << (*it)->num_pairs << "@a";

                } else {

                    out_str << "TA_" << (*it)->id << "_" << "0" << "@a";

                }

                if (std::next(it) != temporal_atoms.end()) {
                    out_str << ":";
                }


            }
            out_str << std::endl;

        }


        if (!out_format.has_value() || (out_format.has_value() && out_flatten)) {

            std::cout << "\n<<<<<< Taking intersection... >>>>>>\n\n";

            back = false;

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

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";

                    } else if ((*it)->type == PNUELION) {

                        MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";

                    } else if ((*it)->type == PNUELIGN) {

                        MitlParser::AtomGnContext* phi = (MitlParser::AtomGnContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";

                    } else if ((*it)->type == PNUELIHN) {

                        MitlParser::AtomHnContext* phi = (MitlParser::AtomHnContext*)(*it);

                        for (auto i = 0; i < phi->atoms.size(); ++i) {

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";

                    } else if ((*it)->type == COUNTFN) {

                        MitlParser::AtomCFnContext* phi = (MitlParser::AtomCFnContext*)(*it);

                        for (auto i = 0; i < phi->num_pairs; ++i) {

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";
                        std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                    } else if ((*it)->type == COUNTON) {

                        MitlParser::AtomCOnContext* phi = (MitlParser::AtomCOnContext*)(*it);

                        for (auto i = 0; i < phi->num_pairs; ++i) {

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";
                        std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                    } else if ((*it)->type == COUNTGN) {

                        MitlParser::AtomCGnContext* phi = (MitlParser::AtomCGnContext*)(*it);

                        for (auto i = 0; i < phi->num_pairs; ++i) {

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";
                        std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                    } else if ((*it)->type == COUNTHN) {

                        MitlParser::AtomCHnContext* phi = (MitlParser::AtomCHnContext*)(*it);

                        for (auto i = 0; i < phi->num_pairs; ++i) {

                            std::cout << "accept_" << (*it)->id << "_" << i << ",";

                        }

                        std::cout << "accept_seq_in_" << (*it)->id << ",";
                        std::cout << "accept_seq_out_" << (*it)->id << ",";
                        std::cout << "accept_" << (*it)->id << "_" << phi->num_pairs << ",";

                    } else {

                        std::cout << "accept_" << (*it)->id << ",";

                    }
                }

                std::cout << "accept_0,accept_M,accept_div " << out_file << std::endl;

                // return no TA, but the genereated output for components

                return { monitaal::TA("dummy", {}, {}, {}, 0), out_str.str() };

            } else {

                assert(("UPPAAL XML output is only supported for flattened (monolithic) automata", false));
                
                // // return nothing; out_str is empty at this point anyway

                // return { monitaal::TA("dummy", {}, {}, {}, 0), std::string{} };

            }

        }


        assert(false);

    }

} // namespace mightypplcpp
