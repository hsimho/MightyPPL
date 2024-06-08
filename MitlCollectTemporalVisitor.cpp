
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlCollectTemporalVisitor.h"


namespace mightylcpp {


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

    std::any MitlCollectTemporalVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom())));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom())));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom())));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom())));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(1))));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(1))));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(1))));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        temporal_atoms.insert(ctx);
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(0))));
        temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atom(1))));
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        temporal_atoms.insert(ctx);
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atoms[i])));
        }
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        temporal_atoms.insert(ctx);
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atoms[i])));
        }
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomFnDual(MitlParser::AtomFnDualContext *ctx) {

        temporal_atoms.insert(ctx);
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atoms[i])));
        }
        return temporal_atoms;

    }

    std::any MitlCollectTemporalVisitor::visitAtomOnDual(MitlParser::AtomOnDualContext *ctx) {

        temporal_atoms.insert(ctx);
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            temporal_atoms.merge(std::any_cast<decltype(temporal_atoms)>(visit(ctx->atoms[i])));
        }
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


}  // namespace mightylcpp

