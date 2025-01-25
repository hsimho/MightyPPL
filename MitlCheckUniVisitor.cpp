
// Generated from Mitl.g4 by ANTLR 4.13.0
//

#include "MitlCheckUniVisitor.h"


namespace mightypplcpp {

//    void MitlCheckUniVisitor::loadParser(const MitlParser& parser) {  //get parser
//        ruleNames = parser.getRuleNames(); //load parser rules from parser
//    }

    std::any MitlCheckUniVisitor::visitMain(MitlParser::MainContext *ctx) {
        return visit(ctx->formula());
    }
    
    std::any MitlCheckUniVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {
        return visit(ctx->atom());
    }
    
    std::any MitlCheckUniVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {
        bool b1 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->formula(1)));
        return b1 && b2;
    }
    
    std::any MitlCheckUniVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {
        bool b1 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->formula(1)));
        return b1 && b2;
    }
    
    std::any MitlCheckUniVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {
        bool b1 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->formula(1)));
        return b1 && b2;
    }
    
    std::any MitlCheckUniVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {
        return visit(ctx->atom());
    }
    
    std::any MitlCheckUniVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {
        bool b1 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->formula(1)));
        return b1 && b2;
    }
    
    std::any MitlCheckUniVisitor::visitBound(MitlParser::BoundContext *ctx) {
        return true;
    }
    
    std::any MitlCheckUniVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->children[0];
        antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->children[4];

        antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->children[1];
        antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->children[3];

        if (
            (left_delim->getSymbol()->getType() == MitlParser::LBrack && left->children[0]->getText() == "0")
            || 
            (right_delim->getSymbol()->getType() == MitlParser::RParen && right->children[0]->getText() == "infty")
           ) {

            return true;

        } else {

            return false;

        }

    }

    std::any MitlCheckUniVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b = std::any_cast<bool>(visit(ctx->atom()));
        return ctx->uni && b;
    }

    std::any MitlCheckUniVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b = std::any_cast<bool>(visit(ctx->atom()));
        return ctx->uni && b;
    }
    
    std::any MitlCheckUniVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b = std::any_cast<bool>(visit(ctx->atom()));
        return ctx->uni && b;
    }

    std::any MitlCheckUniVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b = std::any_cast<bool>(visit(ctx->atom()));
        return ctx->uni && b;
    }

    std::any MitlCheckUniVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b1 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ctx->uni && b1 && b2;
    }

    std::any MitlCheckUniVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b1 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ctx->uni && b1 && b2;
    }
    
    std::any MitlCheckUniVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b1 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ctx->uni && b1 && b2;
    }
    
    std::any MitlCheckUniVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool b1 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool b2 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ctx->uni && b1 && b2;
    }

    std::any MitlCheckUniVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atoms[i]));
            ret = ret && b;
        }
        return ret;

    }
    
    std::any MitlCheckUniVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atoms[i]));
            ret = ret && b;
        }
        return ret;

    }
    
    std::any MitlCheckUniVisitor::visitAtomFnDual(MitlParser::AtomFnDualContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atoms[i]));
            ret = ret && b;
        }
        return ret;

    }
    
    std::any MitlCheckUniVisitor::visitAtomOnDual(MitlParser::AtomOnDualContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atoms[i]));
            ret = ret && b;
        }
        return ret;

    }

    std::any MitlCheckUniVisitor::visitAtomCFn(MitlParser::AtomCFnContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < 4; ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atom(i)));
            ret = ret && b;
        }
        return ret;

    }
    
    std::any MitlCheckUniVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < 4; ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atom(i)));
            ret = ret && b;
        }
        return ret;

    }
    
    std::any MitlCheckUniVisitor::visitAtomCFnDual(MitlParser::AtomCFnDualContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < 4; ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atom(i)));
            ret = ret && b;
        }
        return ret;

    }
    
    std::any MitlCheckUniVisitor::visitAtomCOnDual(MitlParser::AtomCOnDualContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < 4; ++i) {
            bool b = std::any_cast<bool>(visit(ctx->atom(i)));
            ret = ret && b;
        }
        return ret;

    }
    
    std::any MitlCheckUniVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {
        return visit(ctx->formula());
    }
    
    std::any MitlCheckUniVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {
        return true;
    }
    
    std::any MitlCheckUniVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {
        return true;
    }
    
    std::any MitlCheckUniVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {
        return true;
    }



}  // namespace mightypplcpp

