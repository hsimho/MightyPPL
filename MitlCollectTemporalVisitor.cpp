
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "antlr4-runtime.h"
#include "MitlCollectTemporalVisitor.h"


namespace antlrcpptest {


    std::any MitlCollectTemporalVisitor::visitMain(MitlParser::MainContext *ctx) {

        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->formula())));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {

        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom())));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {

        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->formula(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->formula(1))));
        return temporal_atoms; 
        
    }

    std::any MitlCollectTemporalVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        assert(false);

    }

    std::any MitlCollectTemporalVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        assert(false);

    }

    std::any MitlCollectTemporalVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->formula())));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->formula(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->formula(1))));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitBound(MitlParser::BoundContext *ctx) {

        assert(false);

    }

    std::any MitlCollectTemporalVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        assert(false);

    }

    std::any MitlCollectTemporalVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom())));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom())));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(1))));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(1))));
        return temporal_atoms; 

    }

    std::any MitlCollectTemporalVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {

        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->formula())));
        return temporal_atoms; 
    }

    std::any MitlCollectTemporalVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {

        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {

        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {

        return temporal_atoms;

    }


}  // namespace antlrcpptest

