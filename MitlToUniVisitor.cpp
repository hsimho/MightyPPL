
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlToUniVisitor.h"


namespace mightypplcpp {

    std::any MitlToUniVisitor::visitMain(MitlParser::MainContext *ctx) {

        return visit(ctx->formula());

    }
    
    std::any MitlToUniVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {

        std::any output;

        output = visit(ctx->atom());

        return output;

    }
    
    std::any MitlToUniVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
        output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }
    
    std::any MitlToUniVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
        output += std::string(ctx->depth() * 1 + 4, ' ') + "<->" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }
    
    std::any MitlToUniVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
        output += std::string(ctx->depth() * 1 + 4, ' ') + "->" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }
    
    std::any MitlToUniVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "(!";
        output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
        output += std::string(ctx->depth() * 1, ' ') + ")";

        return output;

    }
    
    std::any MitlToUniVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
        output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";
        output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }
    
    std::any MitlToUniVisitor::visitBound(MitlParser::BoundContext *ctx) {

        antlr4::tree::TerminalNode* child = (antlr4::tree::TerminalNode*)ctx->children[0];
        if (child->getSymbol()->getType() == MitlParser::IntLit) {

            return child->getText();

        } else if (child->getSymbol()->getType() == MitlParser::Infty) {

            return child->getText();

        } else {

            assert(false);

        }
    }
    
    std::any MitlToUniVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->children[0];
        antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->children[4];

        antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->children[1];
        antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->children[3];

        if (left_delim->getSymbol()->getType() == MitlParser::LBrack && right_delim->getSymbol()->getType() == MitlParser::RBrack) {

            return std::string("[" + left->getText() + ", " + right->getText() + "]");

        } else if (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RBrack) {

            return std::string("(" + left->getText() + ", " + right->getText() + "]");

        } else if (left_delim->getSymbol()->getType() == MitlParser::LBrack && right_delim->getSymbol()->getType() == MitlParser::RParen) {

            return std::string("[" + left->getText() + ", " + right->getText() + ")");

        } else if (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RParen) {

            return std::string("(" + left->getText() + ", " + right->getText() + ")");

        } else {

            assert(false);

        }

    }
    
    std::any MitlToUniVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        std::string output;

        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "F ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "F ";
            output += (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "CFn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom()));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = "("
                                + atom_clean
                                + " && "
                                + "("
                                + "(!" + atom_clean + ")"
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + atom_clean
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        std::string output;
        
        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "O ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";
        
        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "O ";
            output += (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "COn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom()));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = "("
                                + atom_clean
                                + " && "
                                + "("
                                + "(!" + atom_clean + ")"
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + atom_clean
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        std::string output;

        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "G ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "G ";
            output += (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "CGn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom()));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = std::string("(")
                                + "(!" + atom_clean + ")"
                                + " && "
                                + "("
                                + atom_clean
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + "(!" + atom_clean + ")"
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        std::string output;

        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "H ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "H ";
            output += (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "CHn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom()));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = std::string("(")
                                + "(!" + atom_clean + ")"
                                + " && "
                                + "("
                                + atom_clean
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + "(!" + atom_clean + ")"
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        std::string output;

        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "U "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "U "
                   + (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "CFn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom(1)));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = "("
                                + atom_clean
                                + " && "
                                + "("
                                + "(!" + atom_clean + ")"
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + atom_clean
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        std::string output;

        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "S "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "S "
                   + (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "COn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom(1)));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = "("
                                + atom_clean
                                + " && "
                                + "("
                                + "(!" + atom_clean + ")"
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + atom_clean
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        std::string output;

        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "R "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "R "
                   + (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "CGn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom(1)));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = std::string("(")
                                + "(!" + atom_clean + ")"
                                + " && "
                                + "("
                                + atom_clean
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + "(!" + atom_clean + ")"
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        std::string output;

        if (ctx->uni) {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "T "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        } else {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "T "
                   + (left_delim->getSymbol()->getType() == MitlParser::LBrack ? "[" : "(")
                   + left->children[0]->getText() + ", infty)" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";


            output += std::string(ctx->depth() * 1 + 1, ' ') + "CHn ";
            output += std::any_cast<std::string>(visit(ctx->interval()));
            output += std::string(" (") + "\n";

            std::string atom_clean = std::any_cast<std::string>(visit(ctx->atom(1)));
            atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());
            // atom_clean = std::regex_replace(std::regex_replace(atom_clean, std::regex("^ +| +$|(\S+)"), "$1"), std::regex(" {3,}"), "  ");

            std::string phi_geq1 = std::string("(")
                                + "(!" + atom_clean + ")"
                                + " && "
                                + "("
                                + atom_clean
                                + " U [" + std::to_string(std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) + ", infty) "
                                + "(!" + atom_clean + ")"
                                + ")"
                                + ")";

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + phi_geq1 + ")" + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + phi_geq1 + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + atom_clean + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + ", " + "\n";
            output += std::string(ctx->depth() * 1 + 1, ' ') + "(!" + atom_clean + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";


            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        }

        return output;

    }

    std::any MitlToUniVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "Fn ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }

    std::any MitlToUniVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "On ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }

    std::any MitlToUniVisitor::visitAtomFnDual(MitlParser::AtomFnDualContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "Gn ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }

    std::any MitlToUniVisitor::visitAtomOnDual(MitlParser::AtomOnDualContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "Hn ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }

    std::any MitlToUniVisitor::visitAtomCFn(MitlParser::AtomCFnContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "CFn ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < 4; ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atom(i))) + (i != 3 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }

    std::any MitlToUniVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "COn ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < 4; ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atom(i))) + (i != 3 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }

    std::any MitlToUniVisitor::visitAtomCFnDual(MitlParser::AtomCFnDualContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "CGn ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < 4; ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atom(i))) + (i != 3 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }

    std::any MitlToUniVisitor::visitAtomCOnDual(MitlParser::AtomCOnDualContext *ctx) {

        std::string output;

        output += std::string(ctx->depth() * 1, ' ') + "CHn ";
        output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
        output += std::string(" (") + "\n";
        for (auto i = 0; i < 4; ++i) {
            output += std::string(ctx->depth() * 1 + 4, ' ') + "\n" + std::any_cast<std::string>(visit(ctx->atom(i))) + (i != 3 ? std::string(ctx->depth() * 1 + 4, ' ') + "," : "") + "\n";
        }
        output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

        return output;

    }
    
    std::any MitlToUniVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {

        std::any output;

        output = visit(ctx->formula());

        return output;

    }
    
    std::any MitlToUniVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {

        return std::string(ctx->depth() * 1, ' ') + "true";

    }
    
    std::any MitlToUniVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {

        std::string output;


        output = std::string(ctx->depth() * 1, ' ') + "(" + ctx->Idfr()->getText() + ")";

        return output;

    }
    
    std::any MitlToUniVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {

        return std::string(ctx->depth() * 1, ' ') + "false";

    }
    


}  // namespace mightypplcpp

