
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlToNNFVisitor.h"


namespace mightylcpp {

    std::any MitlToNNFVisitor::visitMain(MitlParser::MainContext *ctx) {
        return visit(ctx->formula());
    }
    
    std::any MitlToNNFVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {
        std::any output;

        if (ctx->negated) {

            ctx->atom()->negated = true;
            output = visit(ctx->atom());
            ctx->atom()->negated = false;
            return output;

        } else {

            output = visit(ctx->atom());
            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;
            return output;

        } else {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";
            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        std::string output;

        if (ctx->negated) {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = true;

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";

            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "&&" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = false;

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";

            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "&&" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";

            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;

            return output;

        } else {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = false;

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";

            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "||" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";

            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = true;

            output += std::string(ctx->depth() * 1 + 1, ' ') + "(" + "\n";

            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1 + 5, ' ') + "||" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";

            output += std::string(ctx->depth() * 1 + 1, ' ') + ")" + "\n";

            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;

            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->formula(1)->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->formula(1)->negated = false;

            return output;

        } else {

            ctx->formula(0)->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->formula(0)->negated = false;
            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        std::string output;

        if (ctx->negated) {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            return output;

        } else {

            ctx->formula()->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->formula()->negated = false;

            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "&&" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;

            return output;

        } else {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "||" + "\n";
            output += std::any_cast<std::string>(visit(ctx->formula(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";
            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitBound(MitlParser::BoundContext *ctx) {

        antlr4::tree::TerminalNode* child = (antlr4::tree::TerminalNode*)ctx->children[0];
        if (child->getSymbol()->getType() == MitlParser::IntLit) {

            return child->getText();

        } else if (child->getSymbol()->getType() == MitlParser::Infty) {

            return child->getText();

        } else {

            assert(false);

        }
    }
    
    std::any MitlToNNFVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

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
    
    std::any MitlToNNFVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom()->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "F ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->atom()->negated = false;

            return output;


        } else {

            output += std::string(ctx->depth() * 1, ' ') + "G ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom()->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "G ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->atom()->negated = false;

            return output;


        } else {

            output += std::string(ctx->depth() * 1, ' ') + "F ";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom())) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom(0)->negated = true;
            ctx->atom(1)->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "U "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->atom(0)->negated = false;
            ctx->atom(1)->negated = false;

            return output;


        } else {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "R "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";


            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom(0)->negated = true;
            ctx->atom(1)->negated = true;

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "R "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";

            ctx->atom(0)->negated = false;
            ctx->atom(1)->negated = false;

            return output;


        } else {

            output += std::string(ctx->depth() * 1, ' ') + "(" + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(0))) + "\n";
            output += std::string(ctx->depth() * 1 + 4, ' ') + "U "
                        + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "") + "\n";
            output += std::any_cast<std::string>(visit(ctx->atom(1))) + "\n";
            output += std::string(ctx->depth() * 1, ' ') + ")" + "\n";


            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {

        std::any output;

        if (ctx->negated) {

            ctx->formula()->negated = true;
            output = visit(ctx->formula());
            ctx->formula()->negated = false;

            return output;

        } else {

            output = visit(ctx->formula());
            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {

        if (ctx->negated) {

            return std::string(ctx->depth() * 1, ' ') + "false";

        } else {

            return std::string(ctx->depth() * 1, ' ') + "true";

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {

        std::string output;

        if (ctx->negated) {

            output = std::string(ctx->depth() * 1, ' ') + "(!" + ctx->Idfr()->getText() + ")";
            return output;

        } else {

            output = std::string(ctx->depth() * 1, ' ') + ctx->Idfr()->getText();
            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {

        if (ctx->negated) {

            return std::string(ctx->depth() * 1, ' ') + "true";

        } else {

            return std::string(ctx->depth() * 1, ' ') + "false";

        }

    }
    


}  // namespace mightylcpp

