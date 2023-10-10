
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlAtomNumberingVisitor.h"


namespace mightylcpp {

    std::any MitlAtomNumberingVisitor::visitMain(MitlParser::MainContext *ctx) {

        root = ctx;
        int output;
        current_id = 0;
        output = std::any_cast<int>(visit(ctx->formula()));
        assert(output == current_id);
        current_id = 0;
        return output;

    }

    std::any MitlAtomNumberingVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {

        return visit(ctx->atom());

    }

    std::any MitlAtomNumberingVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {

        return std::any_cast<int>(visit(ctx->formula(0))) + std::any_cast<int>(visit(ctx->formula(1)));
        
    }

    std::any MitlAtomNumberingVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        assert(false);

    }

    std::any MitlAtomNumberingVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        assert(false);

    }

    std::any MitlAtomNumberingVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        return visit(ctx->formula());

    }

    std::any MitlAtomNumberingVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        return std::any_cast<int>(visit(ctx->formula(0))) + std::any_cast<int>(visit(ctx->formula(1)));

    }

    std::any MitlAtomNumberingVisitor::visitBound(MitlParser::BoundContext *ctx) {

        assert(false);

    }

    std::any MitlAtomNumberingVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        assert(false);

    }

    std::any MitlAtomNumberingVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        ctx->id = ++current_id;
        ctx->type = GLOBALLY;
        return std::any_cast<int>(visit(ctx->atom())) + 1;

    }

    std::any MitlAtomNumberingVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        ctx->id = ++current_id;
        ctx->type = FINALLY;
        return std::any_cast<int>(visit(ctx->atom())) + 1;

    }

    std::any MitlAtomNumberingVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        ctx->id = ++current_id;
        ctx->type = RELEASE;
        return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

    }

    std::any MitlAtomNumberingVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        ctx->id = ++current_id;
        ctx->type = UNTIL;
        return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

    }

    std::any MitlAtomNumberingVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {

        return visit(ctx->formula());

    }

    std::any MitlAtomNumberingVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {

        return 0;

    }

    std::any MitlAtomNumberingVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {

        if (root->props.count(ctx->Idfr()->getText()) == 0) {

            ctx->id = ++current_id;
            root->props.insert({ctx->Idfr()->getText(), current_id}); 
            return 1;

        } else {

            ctx->id = root->props.at(ctx->Idfr()->getText());
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {

        return 0;

    }


}  // namespace mightylcpp

