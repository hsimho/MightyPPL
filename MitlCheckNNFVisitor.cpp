
// Generated from Mitl.g4 by ANTLR 4.13.0
//

#include "MitlCheckNNFVisitor.h"


namespace mightylcpp {

//    void MitlCheckNNFVisitor::loadParser(const MitlParser& parser) {  //get parser
//        ruleNames = parser.getRuleNames(); //load parser rules from parser
//    }

    std::any MitlCheckNNFVisitor::visitMain(MitlParser::MainContext *ctx) {
        return visit(ctx->formula());
    }
    
    std::any MitlCheckNNFVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {
        return visit(ctx->atom());
    }
    
    std::any MitlCheckNNFVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {
        return std::any_cast<bool>(visit(ctx->formula(0))) && std::any_cast<bool>(visit(ctx->formula(1)));
    }
    
    std::any MitlCheckNNFVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        return false;
    }
    
    std::any MitlCheckNNFVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        return false;
    }
    
    std::any MitlCheckNNFVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        // TODO: less hacky way to identify the type of rule?

        if (ctx->formula()->children.size() != 1) {     // Check if ctx is FormulaAtom
            return false;
        }

        antlr4::RuleContext* child = (antlr4::RuleContext*)ctx->formula()->children[0];

        // assert(ruleNames[child->getRuleIndex()] == "atom");

        if (child->children.size() != 1) {
            return false;
        } else {      // AtomTrue, AtomFalse, or AtomIdfr
            if (child->children[0]->getText() == "true" || child->children[0]->getText() == "false") {
                return false;
            }
        }

        return true;

    }
    
    std::any MitlCheckNNFVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {
      return std::any_cast<bool>(visit(ctx->formula(0))) && std::any_cast<bool>(visit(ctx->formula(1)));
    }
    
    std::any MitlCheckNNFVisitor::visitBound(MitlParser::BoundContext *ctx) {
        return true;
    }
    
    std::any MitlCheckNNFVisitor::visitInterval(MitlParser::IntervalContext *ctx) {
        return true;
    }

    std::any MitlCheckNNFVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {
        return visit(ctx->atom());
    }

    std::any MitlCheckNNFVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {
        return visit(ctx->atom());
    }
    
    std::any MitlCheckNNFVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {
        return visit(ctx->atom());
    }

    std::any MitlCheckNNFVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {
        return visit(ctx->atom());
    }

    std::any MitlCheckNNFVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {
        return std::any_cast<bool>(visit(ctx->atom(0))) && std::any_cast<bool>(visit(ctx->atom(1)));
    }

    std::any MitlCheckNNFVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {
        return std::any_cast<bool>(visit(ctx->atom(0))) && std::any_cast<bool>(visit(ctx->atom(1)));
    }
    
    std::any MitlCheckNNFVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {
        return std::any_cast<bool>(visit(ctx->atom(0))) && std::any_cast<bool>(visit(ctx->atom(1)));
    }
    
    std::any MitlCheckNNFVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {
        return std::any_cast<bool>(visit(ctx->atom(0))) && std::any_cast<bool>(visit(ctx->atom(1)));
    }

    std::any MitlCheckNNFVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomFnDual(MitlParser::AtomFnDualContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomOnDual(MitlParser::AtomOnDualContext *ctx) {

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {
        return visit(ctx->formula());
    }
    
    std::any MitlCheckNNFVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {
        return true;
    }
    
    std::any MitlCheckNNFVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {
        return true;
    }
    
    std::any MitlCheckNNFVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {
        return true;
    }



}  // namespace mightylcpp

