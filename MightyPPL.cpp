#include "MightyPPL.h"

namespace mightypplcpp {

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

        std::cout << std::setw(12);     // only affects the first char below, actually
        for (int v = 0; v < size; ++v) {
            output += (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
            std::cout << (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
        }
        std::cout << std::endl;
        std::cout << std::setw(0);

        sat_paths.push_back(output);

    }

    std::string build_edge(int id, const std::string& source, const std::string& target, const std::string& guard, bool reset, bdd label) {

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
                                + (guard.size() ? " && x_" + std::to_string(id) + " " + guard : std::string{})
                                + (reset? " : do: x_" + std::to_string(id) + " = 0}" : "}") 
                                + "\n"; 

            std::stringstream().swap(p_constraint);

        }

        sat_paths.clear();
        return s;

    }

    std::string build_ta_from_atom(const MitlParser::AtomContext* phi_) {

        std::stringstream tck;


        if (phi_->type == FINALLY) {

            MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomFContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Finally
                 "untimed" case
                *****/
                
                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0 -> 1, r && *p && *q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ^p && ~q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_true() & phi->atom()->tilde);

                // 2 -> 1, ^p && ~q

                tck << build_edge(phi->id, "2", "1", std::string{}, false, bdd_true() & phi->atom()->tilde);

                // 1 -> 2, r && *p && ^q

                tck << build_edge(phi->id, "1", "2", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 2 -> 2, r && *p && ^q

                tck << build_edge(phi->id, "2", "2", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 1 -> 0, !r && *p && ^q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 2 -> 0, !r && *p && ^q

                tck << build_edge(phi->id, "2", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, ^p && ~q, x <= a

                    tck << build_edge(phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_true() & phi->atom()->tilde);

                    // 1 -> 2, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Finally
                     [l, infty) or (l, infty)
                    *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;
                    // tck << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ^p && ~q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, !r && ^p && *q, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ~q, x := 0

                    tck << build_edge(phi->id, "1", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 1, !r && ^p && ~q

                    tck << build_edge(phi->id, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, !r && ^p && *q, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 1, r && ^p && ~q, x := 0

                    tck << build_edge(phi->id, "2", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 2, !r && ^p && ^q

                    tck << build_edge(phi->id, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 2, r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "1", "2", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, !r && ^p && ^q

                    tck << build_edge(phi->id, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "2", "2", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // // 1 -> 3, r && ^p && ~q, x := 0

                    // tck << build_edge(phi->id, "1", "3", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // // 1 -> 3, r && ^p && *q, x := 0, x < a

                    // tck << build_edge(phi->id, "1", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 3 -> 3, ^p && ~q

                    // tck << build_edge(phi->id, "3", "3", std::string{}, false, bdd_true() & phi->atom()->tilde);

                    // // 3 -> 3, ^p && *q, x < a

                    // tck << build_edge(phi->id, "3", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_true() & phi->atom()->star);

                    // // 3 -> 2, ^p && ^q, x := 0, x >= a

                    // tck << build_edge(phi->id, "3", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_true() & phi->atom()->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == ONCE) {

            MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomOContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Once
                 "untimed" case
                *****/
                
                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 0, r && *p && *q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ^p && ~q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_true() & phi->atom()->tilde);

                // 1 -> 1, r && *p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                // 0 -> 1, !r && *p && ^q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a 

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_true() & phi->atom()->tilde);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Once
                     [l, infty) or (l, infty)
                    *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;
                    // tck << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ^p && *q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 2 -> 1, !r && ^p && *q

                    // tck << build_edge(phi->id, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 1 -> 2, !r && ^p && *q

                    // tck << build_edge(phi->id, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, !r && ^p && *q

                    tck << build_edge(phi->id, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ~q, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 1, r && ^p && ~q, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // // 1 -> 2, r && ^p && ~q, x >= a

                    // tck << build_edge(phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 2, r && ^p && ~q, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 1, r && *p && ^q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == GLOBALLY) {

            MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomGContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Globally
                 "untimed" case
                *****/

                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 0 -> 1, r && *p && *q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ~p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                // 1 -> 1, r && ^p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 1 -> 0, !r && ^p && ^q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && ^q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 0, !r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Globally
                     [l, infty) or (l, infty)
                    *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && *q, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, !r && ~p && ^q, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ~p && *q, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 1, r && ~p && ^q, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 0, !r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == HISTORICALLY) {

            MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomHContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Historically
                 "untimed" case
                *****/

                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{initial: : labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 0, r && *p && *q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                // 1 -> 1, ~p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                // 1 -> 1, r && ^p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                // 0 -> 1, !r && ^p && ^q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{initial: : labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && ~p && ^q, x <= a

                    tck << build_edge(phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ~p && ^q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, r && ~p && ^q, x > a

                    tck << build_edge(phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);
                    
                    // 1 -> 1, !r && ~p && ^q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 2 -> 2, !r && ~p && ^q

                    tck << build_edge(phi->id, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x > a

                    tck << build_edge(phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 1 -> 2, r && *p && *q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0 -> 2, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Historically
                     [l, infty) or (l, infty)
                    *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{initial: : labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 1 -> 1, ~p && ^q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_true() & phi->atom()->hat);

                    // 2 -> 2, ~p && *q, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_true() & phi->atom()->star);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);

                    // 0 -> 2, !r && ^p && *q, x := 0

                    tck << build_edge(phi->id, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & bdd_false() & phi->atom()->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == UNTIL) {

            MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomUContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Until
                 "untimed" case
                *****/

                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0 -> 1, r && *p && *q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ^p && ~q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 2 -> 1, ^p && ~q

                tck << build_edge(phi->id, "2", "1", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 1 -> 2, r && *p && ^q

                tck << build_edge(phi->id, "1", "2", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 2 -> 2, r && *p && ^q

                tck << build_edge(phi->id, "2", "2", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 1 -> 0, !r && *p && ^q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 2 -> 0, !r && *p && ^q

                tck << build_edge(phi->id, "2", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, , x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, ^p && ~q, x <= a

                    tck << build_edge(phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 2, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Until
                     [l, infty) or (l, infty)
                     *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;
                    // tck << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ^p && ~q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, !r && ^p && *q, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ~q, x := 0

                    tck << build_edge(phi->id, "1", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 1, !r && ^p && ~q

                    tck << build_edge(phi->id, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, !r && ^p && *q, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 1, r && ^p && ~q, x := 0

                    tck << build_edge(phi->id, "2", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 2, r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 2, !r && ^p && ^q

                    tck << build_edge(phi->id, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 2, r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "1", "2", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 2, r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 2, !r && ^p && ^q

                    tck << build_edge(phi->id, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 2, r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "2", "2", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 0, !r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // // 1 -> 3, r && ^p && ~q, x := 0

                    // tck << build_edge(phi->id, "1", "3", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 1 -> 3, r && ^p && *q, x := 0, x < a

                    // tck << build_edge(phi->id, "1", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // // 3 -> 3, ^p && ~q

                    // tck << build_edge(phi->id, "3", "3", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 3 -> 3, ^p && *q, x < a

                    // tck << build_edge(phi->id, "3", "3", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->star);

                    // // 3 -> 2, ^p && ^q, x := 0, x >= a

                    // tck << build_edge(phi->id, "3", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, phi->atom(0)->hat & phi->atom(1)->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }


        } else if (phi_->type == SINCE) {

            MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomSContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Since
                 "untimed" case
                *****/

                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ^p && ~q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, phi->atom(0)->hat & phi->atom(1)->tilde);

                // 1 -> 1, r && *p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                // 0 -> 1, !r && *p && ^q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a 

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ^p && ~q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Since
                     [l, infty) or (l, infty)
                    *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{labels: accept_" << phi->id << "}" << std::endl;
                    // tck << "location:" << "TA_" << phi->id << ":ell_3{}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ^p && *q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // // 2 -> 1, !r && ^p && *q

                    // tck << build_edge(phi->id, "2", "1", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // // 1 -> 2, !r && ^p && *q

                    // tck << build_edge(phi->id, "1", "2", std::string{}, false, !bdd_ithvar(phi->id) & bdd_true() & phi->atom()->star);

                    // 2 -> 2, !r && ^p && *q

                    tck << build_edge(phi->id, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ~q, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 2 -> 1, r && ^p && ~q, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // // 1 -> 2, r && ^p && ~q, x >= a

                    // tck << build_edge(phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & bdd_true() & phi->atom()->tilde);

                    // 2 -> 2, r && ^p && ~q, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->tilde);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && *p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 2 -> 1, r && *p && ^q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 0 -> 1, !r && *p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }


        } else if (phi_->type == RELEASE) {

            MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomRContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Release
                 "untimed" case
                *****/

                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 0 -> 1, r && *p && *q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ~p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, r && ^p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 1 -> 0, !r && ^p && ^q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && ^q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && *p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->hat);

                    // 1 -> 1, r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Release
                     [l, infty) or (l, infty)
                    *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && *q, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 1, !r && ~p && ^q, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ~p && *q, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ~p && ^q, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 0, !r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 0, !r && ^p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == TRIGGER) {

            MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)phi_;

            tck << std::endl << std::endl;
            tck << "# " << "TA_" << phi->id << std::endl;
            tck << "# " << const_cast<MitlParser::AtomTContext*>(phi)->getText() << std::endl;
            tck << "process:" << "TA_" << phi->id << std::endl;

            if (phi->interval() == nullptr) {

                /***** Trigger
                 "untimed" case
                *****/

                tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                tck << "location:" << "TA_" << phi->id << ":ell_1{initial: : labels: accept_" << phi->id << "}" << std::endl;

                // 0 -> 0, !r && *p && *q

                tck << build_edge(phi->id, "0", "0", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 0, r && *p && *q

                tck << build_edge(phi->id, "1", "0", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                // 1 -> 1, ~p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                // 1 -> 1, r && ^p && ^q

                tck << build_edge(phi->id, "1", "1", std::string{}, false, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                // 0 -> 1, !r && ^p && ^q

                tck << build_edge(phi->id, "0", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


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

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{initial: : labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "2", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 2, r && ~p && ^q, x <= a

                    tck << build_edge(phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ~p && ^q, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);
                    
                    // 2 -> 2, r && ~p && ^q, x > a

                    tck << build_edge(phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);                    

                    // 1 -> 1, !r && ~p && ^q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 2 -> 2, !r && ~p && ^q

                    tck << build_edge(phi->id, "2", "2", std::string{}, false, !bdd_ithvar(phi->id) & phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 1 -> 1, r && ^p && ^q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x > a

                    tck << build_edge(phi->id, "2", "1", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 1 -> 2, r && *p && *q, x := 0, x <= a

                    tck << build_edge(phi->id, "1", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 2, r && *p && *q, x := 0, x > a

                    tck << build_edge(phi->id, "2", "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0 -> 2, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);


                } else if (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty") {

                    /***** Trigger
                     [l, infty) or (l, infty)
                    *****/

                    tck << "location:" << "TA_" << phi->id << ":ell_0{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_1{initial: : labels: accept_" << phi->id << "}" << std::endl;
                    tck << "location:" << "TA_" << phi->id << ":ell_2{initial: : labels: accept_" << phi->id << "}" << std::endl;

                    // 0 -> 0, !r && *p && *q, x := 0

                    tck << build_edge(phi->id, "0", "0", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 0, r && *p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 2 -> 0, r && *p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "0", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->star & phi->atom(1)->star);

                    // 1 -> 1, ~p && ^q

                    tck << build_edge(phi->id, "1", "1", std::string{}, false, phi->atom(0)->tilde & phi->atom(1)->hat);

                    // 2 -> 2, ~p && *q, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), false, phi->atom(0)->tilde & phi->atom(1)->star);

                    // 1 -> 2, r && ^p && *q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 2 -> 2, r && ^p && *q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "2", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 1 -> 1, r && ^p && ^q, x := 0, x >= a

                    tck << build_edge(phi->id, "1", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? ">= " : "> ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 2 -> 1, r && ^p && ^q, x := 0, x < a

                    tck << build_edge(phi->id, "2", "1", (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "< " : "<= ") + left->children[0]->getText(), true, bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);

                    // 0 -> 2, !r && ^p && *q, x := 0

                    tck << build_edge(phi->id, "0", "2", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->star);

                    // 0 -> 1, !r && ^p && ^q, x := 0

                    tck << build_edge(phi->id, "0", "1", std::string{}, true, !bdd_ithvar(phi->id) & phi->atom(0)->hat & phi->atom(1)->hat);


                } else {

                    assert(("The current version only supports unilateral intervals", false));

                }

            }

        } else if (phi_->type == PNUELIFN) {

            MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)phi_;

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

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        tck << std::endl << std::endl;
                        tck << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                        tck << "# " << const_cast<MitlParser::AtomFnContext*>(phi)->getText() << std::endl;
                        tck << "process:" << "TA_" << phi->id + i << std::endl;

                        tck << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                        for (auto j = 0; j < phi->atoms.size(); ++j) {
                            tck << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{}" << std::endl;
                        }

                        tck << "location:" << "TA_" << phi->id + i << ":ell_2{labels: accept_" << phi->id + i << "}" << std::endl;

                        // 0 -> 0, !r, x := 0

                        tck << build_edge(phi->id + i, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 0 -> 1_0, r, x := 0

                        tck << build_edge(phi->id + i, "0", "1_0", std::string{}, true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ~p_j

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            // 1_j -> 1_j+1, !r && ^p_j (x := 0, x <= a)

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), (j + 1 == phi->atoms.size() ? "0" : "1_" + std::to_string(j + 1)), (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), (j + 1 == phi->atoms.size() ? true : false), !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_j -> 1_0, r && *p_j (r && ~p_j, x <= a)

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_0", (j + 1 == phi->atoms.size() ? (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText() : std::string{}), false, bdd_ithvar(phi->id + i) & (j + 1 == phi->atoms.size() ? phi->atoms[phi->atoms.size() - 1]->tilde : phi->atoms[j]->star));

                        }

                        // 1_n-1 -> 2, r && ^p_j, x := 0, x <= a

                        tck << build_edge(phi->id + i, "1_" + std::to_string(phi->atoms.size() - 1), "2", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[phi->atoms.size() - 1]->hat);

                        // 2 -> 1_0, !r && ~p_0

                        tck << build_edge(phi->id + i, "2", "1_0", std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[0]->tilde);

                        // 2 -> 1_1, !r && ^p_0

                        tck << build_edge(phi->id + i, "2", "1_1", std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[0]->hat);

                        // 2 -> 1_0, r

                        tck << build_edge(phi->id + i, "2", "1_0", std::string{}, false, bdd_ithvar(phi->id + i));

                    }

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else if (phi_->type == PNUELION) {

            MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)phi_;

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

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        tck << std::endl << std::endl;
                        tck << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                        tck << "# " << const_cast<MitlParser::AtomOnContext*>(phi)->getText() << std::endl;
                        tck << "process:" << "TA_" << phi->id + i << std::endl;

                        tck << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                        for (auto j = 0; j < phi->atoms.size(); ++j) {
                            tck << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{labels: accept_" << phi->id + i << "}" << std::endl;
                        }

                        // 0 -> 0, !r, x := 0

                        tck << build_edge(phi->id + i, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 1_0 -> 0, r, x := 0, x <= a

                        tck << build_edge(phi->id + i, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ~p_j

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            // 1_j+1 -> 1_j, !r && ^p_j (x := 0)

                            tck << build_edge(phi->id + i, (j + 1 == phi->atoms.size() ? "0" : "1_" + std::to_string(j + 1)), "1_" + std::to_string(j), std::string{}, (j + 1 == phi->atoms.size() ? true : false), !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_0 -> 1_j, r && *p_j (r && ~p_j)

                            tck << build_edge(phi->id + i, "1_0", "1_" + std::to_string(j), std::string{}, false, bdd_ithvar(phi->id + i) & (j + 1 == phi->atoms.size() ? phi->atoms[phi->atoms.size() - 1]->tilde : phi->atoms[j]->star));

                        }

                        // 1_0 -> 1_n-1, r && ^p_j, x := 0, x <= a

                        tck << build_edge(phi->id + i, "1_0", "1_" + std::to_string(phi->atoms.size() - 1), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "<= " : "< ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[phi->atoms.size() - 1]->hat);

                    }

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else if (phi_->type == PNUELIFNDUAL) {

            MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)phi_;

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

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        tck << std::endl << std::endl;
                        tck << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                        tck << "# " << const_cast<MitlParser::AtomFnDualContext*>(phi)->getText() << std::endl;
                        tck << "process:" << "TA_" << phi->id + i << std::endl;

                        tck << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                        for (auto j = 0; j < phi->atoms.size(); ++j) {
                            tck << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{labels: accept_" << phi->id + i << "}" << std::endl;
                        }

                        // 0 -> 0, !r, x := 0

                        tck << build_edge(phi->id + i, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 0 -> 1_0, r, x := 0

                        tck << build_edge(phi->id + i, "0", "1_0", std::string{}, true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ^p_j

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_j -> 1_j, r && ^p_j, x := 0

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, true, bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            if (j + 1 != phi->atoms.size()) {

                                // 1_j -> 1_j+1, !r && ~p_j

                                tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                                // 1_j -> 1_j+1, r && ~p_j, x := 0

                                tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j + 1), std::string{}, true, bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            }

                            // 1_j -> 0, !r && *p_j, x := 0, x > a

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, !bdd_ithvar(phi->id + i) & phi->atoms[j]->star);

                            // 1_j -> 1_0, r && *p_j, x := 0, x > a

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[j]->star);
                        }

                    }

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else if (phi_->type == PNUELIONDUAL) {

            MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)phi_;

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

                    for (auto i = 0; i < phi->atoms.size(); ++i) {

                        tck << std::endl << std::endl;
                        tck << "# " << "TA_" << phi->id + i << " (" << i + 1 << " / " << phi->atoms.size() << ")" << std::endl;
                        tck << "# " << const_cast<MitlParser::AtomOnDualContext*>(phi)->getText() << std::endl;
                        tck << "process:" << "TA_" << phi->id + i << std::endl;

                        tck << "location:" << "TA_" << phi->id + i << ":ell_0{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                        for (auto j = 0; j < phi->atoms.size(); ++j) {
                            tck << "location:" << "TA_" << phi->id + i << ":ell_1_" << j << "{initial: : labels: accept_" << phi->id + i << "}" << std::endl;
                        }

                        // 0 -> 0, !r, x := 0

                        tck << build_edge(phi->id + i, "0", "0", std::string{}, true, !bdd_ithvar(phi->id + i));

                        // 1_0 -> 0, r, x := 0, x > a

                        tck << build_edge(phi->id + i, "1_0", "0", (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i));

                        for (auto j = 0; j < phi->atoms.size(); ++j) {

                            // 1_j -> 1_j, !r && ^p_j

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            // 1_j -> 1_j, r && ^p_j, x > a

                            tck << build_edge(phi->id + i, "1_" + std::to_string(j), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id + i) & phi->atoms[j]->hat);

                            if (j + 1 != phi->atoms.size()) {

                                // 1_j+1 -> 1_j, !r && ~p_j

                                tck << build_edge(phi->id + i, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), std::string{}, false, !bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                                // 1_j+1 -> 1_j, r && ~p_j, x > a

                                tck << build_edge(phi->id + i, "1_" + std::to_string(j + 1), "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), false, bdd_ithvar(phi->id + i) & phi->atoms[j]->tilde);

                            }

                            // 0 -> 1_j, !r && *p_j, x := 0

                            tck << build_edge(phi->id + i, "0", "1_" + std::to_string(j), std::string{}, true, !bdd_ithvar(phi->id + i) & phi->atoms[j]->star);

                            // 1_0 -> 1_j, r && *p_j, x := 0, x > a

                            tck << build_edge(phi->id + i, "1_0", "1_" + std::to_string(j), (right_delim->getSymbol()->getType() == MitlParser::RBrack ? "> " : ">= ") + right->children[0]->getText(), true, bdd_ithvar(phi->id + i) & phi->atoms[j]->star);
                        }

                    }

                } else {

                    assert(("Pnueli modalities must be decorated with [0, u] or [0, u)", false));

                }

            }

        } else {

            assert(("Unsupported atom type", false));

        }

        return tck.str();

    }

    std::string build_ta_from_main(MitlParser::MainContext* phi_) {

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


        std::stringstream tck;

        tck << "# Model generated by MightyPPL" << std::endl;
        tck << "system:model_and_spec" << std::endl << std::endl << std::endl;

        tck << "event:a" << std::endl << std::endl << std::endl;

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == FINALLY) {
                MitlParser::AtomFContext* phi = (MitlParser::AtomFContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == ONCE) {
                MitlParser::AtomOContext* phi = (MitlParser::AtomOContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == GLOBALLY) {
                MitlParser::AtomGContext* phi = (MitlParser::AtomGContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == HISTORICALLY) {
                MitlParser::AtomHContext* phi = (MitlParser::AtomHContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == UNTIL) {
                MitlParser::AtomUContext* phi = (MitlParser::AtomUContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == SINCE) {
                MitlParser::AtomSContext* phi = (MitlParser::AtomSContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == RELEASE) {
                MitlParser::AtomRContext* phi = (MitlParser::AtomRContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == TRIGGER) {
                MitlParser::AtomTContext* phi = (MitlParser::AtomTContext*)(*it);
                if (phi->interval() != nullptr) {
                    tck << "clock:1:x_" << phi->id << std::endl;
                }
            } else if ((*it)->type == PNUELIFN) {
                MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);
                for (auto i = 0; i < phi->atoms.size(); ++i) {
                    tck << "clock:1:x_" << phi->id + i << std::endl;
                }
            } else if ((*it)->type == PNUELION) {
                MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);
                for (auto i = 0; i < phi->atoms.size(); ++i) {
                    tck << "clock:1:x_" << phi->id + i << std::endl;
                }
            } else if ((*it)->type == PNUELIFNDUAL) {
                MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)(*it);
                for (auto i = 0; i < phi->atoms.size(); ++i) {
                    tck << "clock:1:x_" << phi->id + i << std::endl;
                }
            } else if ((*it)->type == PNUELIONDUAL) {
                MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)(*it);
                for (auto i = 0; i < phi->atoms.size(); ++i) {
                    tck << "clock:1:x_" << phi->id + i << std::endl;
                }
            } else {
                assert(false);
            }

        }

        tck << "clock:1:x_div" << std::endl;
        tck << std::endl << std::endl;


        for (auto i = 0; i < num_all_props; ++i) {
            tck << "int:1:0:1:0:p_" << i + 1 << std::endl;
        }

        tck << "int:1:0:1:0:turn" << std::endl;

        std::cout << "\n<<<<<< Converting into TAs... >>>>>>\n\n";

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == PNUELIFN || (*it)->type == PNUELION || (*it)->type == PNUELIFNDUAL || (*it)->type == PNUELIONDUAL) {
                std::cout << "\nGenerating TA_" << (*it)->id << " (and other sub-components)...\n";
            } else {
                std::cout << "\nGenerating TA_" << (*it)->id << "...\n";
            }
            tck << build_ta_from_atom(*it);


        }

        std::cout << "\nGenerating TA_0" << "...\n";

        tck << std::endl << std::endl;
        tck << "# " << "TA_0" << std::endl;
        tck << "# " << nnf_formula->formula()->getText() << std::endl;
        tck << "process:" << "TA_0" << std::endl;

        tck << "location:" << "TA_0" << ":ell_0{initial: }" << std::endl;
        tck << "location:" << "TA_0" << ":ell_1{labels: accept_0}" << std::endl;

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
                        tck << "edge:" << "TA_0" << ":ell_0:ell_1:a{provided: turn == 0 : do: " << p_assignments.str() << "}" << std::endl;
                    } else {
                        tck << "edge:" << "TA_0" << ":ell_0:ell_1:a{provided: turn == 0}" << std::endl;
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
                        tck << "edge:" << "TA_0" << ":ell_1:ell_1:a{provided: turn == 0 : do: " << p_assignments.str() << "}" << std::endl;
                    } else {
                        tck << "edge:" << "TA_0" << ":ell_1:ell_1:a{provided: turn == 0}" << std::endl;
                    }
                }
            }
        }

        sat_paths.clear();

        std::cout << "\nGenerating TA_div" << "...\n";

        tck << std::endl << std::endl;
        tck << "# " << "TA_div" << std::endl;
        tck << "process:" << "TA_div" << std::endl;

        tck << "location:" << "TA_div" << ":ell_0{initial: : labels: accept_div}" << std::endl;
        tck << "location:" << "TA_div" << ":ell_1{}" << std::endl;

        tck << "edge:" << "TA_div" << ":ell_0:ell_1:a{provided: turn == 0 : do: turn = 1}" << std::endl;
        tck << "edge:" << "TA_div" << ":ell_0:ell_1:a{provided: turn == 1 : do: turn = 0}" << std::endl;

        tck << "edge:" << "TA_div" << ":ell_1:ell_1:a{provided: turn == 0 && x_div < 1 : do: turn = 1}" << std::endl;
        tck << "edge:" << "TA_div" << ":ell_1:ell_1:a{provided: turn == 1 && x_div < 1 : do: turn = 0}" << std::endl;

        tck << "edge:" << "TA_div" << ":ell_1:ell_0:a{provided: turn == 0 && x_div >= 1 : do: turn = 1; x_div = 0}" << std::endl;
        tck << "edge:" << "TA_div" << ":ell_1:ell_0:a{provided: turn == 1 && x_div >= 1 : do: turn = 0; x_div = 0}" << std::endl;

        std::cout << "\nGenerating M" << "...\n";

        tck << std::endl << std::endl;
        tck << "# " << "M" << std::endl;
        tck << "process:" << "M" << std::endl;

        tck << "location:" << "M" << ":ell_0{initial: : labels: accept_M}" << std::endl;

        tck << "edge:" << "M" << ":ell_0:ell_0:a{provided: turn == 1}" << std::endl;

        std::cout << "\nGenerating sync constraints" << "...\n";

        tck << std::endl << std::endl;
        tck << "# " << "sync constraints" << std::endl;
        tck << "sync:TA_div@a:TA_0@a" << std::endl;

        tck << "sync:TA_div@a:M@a:";

        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {


            if ((*it)->type == PNUELIFN) {

                MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    tck << "TA_" << (*it)->id + i << "@a";
                    if (i != phi->atoms.size() - 1) {
                        tck << ":";
                    }

                }

            } else if ((*it)->type == PNUELION) {

                MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    tck << "TA_" << (*it)->id + i << "@a";
                    if (i != phi->atoms.size() - 1) {
                        tck << ":";
                    }

                }

            } else if ((*it)->type == PNUELIFNDUAL) {

                MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    tck << "TA_" << (*it)->id + i << "@a";
                    if (i != phi->atoms.size() - 1) {
                        tck << ":";
                    }

                }

            } else if ((*it)->type == PNUELIONDUAL) {

                MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    tck << "TA_" << (*it)->id + i << "@a";
                    if (i != phi->atoms.size() - 1) {
                        tck << ":";
                    }

                }

            } else {

                tck << "TA_" << (*it)->id << "@a";

            }

            if (std::next(it) != temporal_atoms.end()) {
                tck << ":";
            }


        }
        tck << std::endl;


        std::cout << "\n<<<<<< Outputting TChecker model... >>>>>>\n\n";

        std::cout << "\nPlease use the following command to check satisfiability:\n\n";
        std::cout << "tck-liveness -a couvscc -l ";
        for (auto it = temporal_atoms.begin(); it != temporal_atoms.end(); ++it) {

            if ((*it)->type == PNUELIFN) {

                MitlParser::AtomFnContext* phi = (MitlParser::AtomFnContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    std::cout << "accept_" << (*it)->id + i << ",";

                }

            } else if ((*it)->type == PNUELION) {

                MitlParser::AtomOnContext* phi = (MitlParser::AtomOnContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    std::cout << "accept_" << (*it)->id + i << ",";

                }

            } else if ((*it)->type == PNUELIFNDUAL) {

                MitlParser::AtomFnDualContext* phi = (MitlParser::AtomFnDualContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    std::cout << "accept_" << (*it)->id + i << ",";

                }

            } else if ((*it)->type == PNUELIONDUAL) {

                MitlParser::AtomOnDualContext* phi = (MitlParser::AtomOnDualContext*)(*it);

                for (auto i = 0; i < phi->atoms.size(); ++i) {

                    std::cout << "accept_" << (*it)->id + i << ",";

                }

            } else {

                std::cout << "accept_" << (*it)->id << ",";

            }
        }

        std::cout << "accept_0,accept_M,accept_div out.tck" << std::endl;
        
        return tck.str();

    }

} // namespace mightypplcpp
