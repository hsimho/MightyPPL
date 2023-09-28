
// Generated from Mitl.g4 by ANTLR 4.13.0

#pragma once

#include "antlr4-runtime.h"
#include "bdd.h"

#include "MitlVisitor.h"


namespace antlrcpptest {

class  MitlGetBDDVisitor : public MitlVisitor {

private:

    MitlParser::MainContext* root;

public:

    std::any visitMain(MitlParser::MainContext *ctx) override;

    std::any visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) override;

    std::any visitFormulaAnd(MitlParser::FormulaAndContext *ctx) override;

    std::any visitFormulaIff(MitlParser::FormulaIffContext *ctx) override;

    std::any visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) override;

    std::any visitFormulaNot(MitlParser::FormulaNotContext *ctx) override;

    std::any visitFormulaOr(MitlParser::FormulaOrContext *ctx) override;

    std::any visitBound(MitlParser::BoundContext *ctx) override;

    std::any visitInterval(MitlParser::IntervalContext *ctx) override;

    std::any visitAtomG(MitlParser::AtomGContext *ctx) override;

    std::any visitAtomF(MitlParser::AtomFContext *ctx) override;

    std::any visitAtomR(MitlParser::AtomRContext *ctx) override;

    std::any visitAtomU(MitlParser::AtomUContext *ctx) override;

    std::any visitAtomParen(MitlParser::AtomParenContext *ctx) override;

    std::any visitAtomTrue(MitlParser::AtomTrueContext *ctx) override;

    std::any visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) override;

    std::any visitAtomFalse(MitlParser::AtomFalseContext *ctx) override;


};

}  // namespace antlrcpptest
