
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlTypingVisitor.h"


namespace mightypplcpp {


    std::any MitlTypingVisitor::visitMain(MitlParser::MainContext *ctx) {

        visit(ctx->formula());
        return nullptr;

    }

    std::any MitlTypingVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {

        visit(ctx->atom());
        return nullptr;

    }

    std::any MitlTypingVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {

        visit(ctx->formula(0));
        visit(ctx->formula(1));
        return nullptr;

    }

    std::any MitlTypingVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        visit(ctx->formula(0));
        visit(ctx->formula(1));
        return nullptr;

    }

    std::any MitlTypingVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        visit(ctx->formula(0));
        visit(ctx->formula(1));
        return nullptr;

    }

    std::any MitlTypingVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        visit(ctx->atom());
        return nullptr;

    }

    std::any MitlTypingVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        visit(ctx->formula(0));
        visit(ctx->formula(1));
        return nullptr;

    }

    std::any MitlTypingVisitor::visitBound(MitlParser::BoundContext *ctx) {
        assert(("visitBound() should not be called", false));
    }

    std::any MitlTypingVisitor::visitInterval(MitlParser::IntervalContext *ctx) {
        assert(("visitInterval() should not be called", false));
    }

    std::any MitlTypingVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        visit(ctx->atom());
        ctx->type = FINALLY;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        visit(ctx->atom());
        ctx->type = ONCE;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        visit(ctx->atom());
        ctx->type = GLOBALLY;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        visit(ctx->atom());
        ctx->type = HISTORICALLY;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = UNTIL;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = SINCE;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = RELEASE;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = TRIGGER;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }
        ctx->type = PNUELIFN;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }
        ctx->type = PNUELION;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomGn(MitlParser::AtomGnContext *ctx) {

        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }
        ctx->type = PNUELIGN;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomHn(MitlParser::AtomHnContext *ctx) {

        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }
        ctx->type = PNUELIHN;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomCFn(MitlParser::AtomCFnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = COUNTFN;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = COUNTON;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomCGn(MitlParser::AtomCGnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = COUNTGN;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomCHn(MitlParser::AtomCHnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        ctx->type = COUNTHN;
        return nullptr; 

    }

    std::any MitlTypingVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {

        visit(ctx->formula());
        ctx->type = PAREN;
        return nullptr;

    }

    std::any MitlTypingVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {
        return nullptr; 
    }

    std::any MitlTypingVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {
        return nullptr; 
    }

    std::any MitlTypingVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {
        return nullptr; 
    }


}  // namespace mightypplcpp

