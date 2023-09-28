
// Generated from Mitl.g4 by ANTLR 4.13.0
//

#include "antlr4-runtime.h"
#include "MitlCheckNNFVisitor.h"


namespace antlrcpptest {

    void MitlCheckNNFVisitor::loadParser(MitlParser *parser) {  //get parser
        ruleNames = parser->getRuleNames(); //load parser rules from parser
    }

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

        if (ctx->formula()->children.size() != 1) {     // Check if ctx is FormulaAtom
            return false;
        }

        antlr4::RuleContext* child = (antlr4::RuleContext*)ctx->formula()->children[0];

        assert(ruleNames[child->getRuleIndex()] == "atom");

        if (child->children.size() == 1) {
            return true;
        } else {
            return false;   
        }

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
    
    std::any MitlCheckNNFVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {
        return visit(ctx->atom());
    }
    
    std::any MitlCheckNNFVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {
        return visit(ctx->atom());
    }
    
    std::any MitlCheckNNFVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {
        return std::any_cast<bool>(visit(ctx->atom(0))) && std::any_cast<bool>(visit(ctx->atom(1)));
    }
    
    std::any MitlCheckNNFVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {
        return std::any_cast<bool>(visit(ctx->atom(0))) && std::any_cast<bool>(visit(ctx->atom(1)));
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



}  // namespace antlrcpptest

