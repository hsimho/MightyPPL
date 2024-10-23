
// Generated from Mitl.g4 by ANTLR 4.13.0

#pragma once

#include <vector>
#include <string>

#include "antlr4-runtime.h"

#include "MitlVisitor.h"


namespace mightypplcpp {

/**
 * This class provides an empty implementation of MitlVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  MitlCheckNNFVisitor : public MitlVisitor {

//private:
//
//    std::vector<std::string> ruleNames;

public:

//    void loadParser(const MitlParser& parser);

    std::any visitMain(MitlParser::MainContext *ctx) override;

    std::any visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) override;

    std::any visitFormulaAnd(MitlParser::FormulaAndContext *ctx) override;

    std::any visitFormulaIff(MitlParser::FormulaIffContext *ctx) override;

    std::any visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) override;

    std::any visitFormulaNot(MitlParser::FormulaNotContext *ctx) override;

    std::any visitFormulaOr(MitlParser::FormulaOrContext *ctx) override;

    std::any visitBound(MitlParser::BoundContext *ctx) override;

    std::any visitInterval(MitlParser::IntervalContext *ctx) override;

    std::any visitAtomF(MitlParser::AtomFContext *ctx) override;

    std::any visitAtomO(MitlParser::AtomOContext *ctx) override;

    std::any visitAtomG(MitlParser::AtomGContext *ctx) override;

    std::any visitAtomH(MitlParser::AtomHContext *ctx) override;

    std::any visitAtomU(MitlParser::AtomUContext *ctx) override;

    std::any visitAtomS(MitlParser::AtomSContext *ctx) override;

    std::any visitAtomR(MitlParser::AtomRContext *ctx) override;

    std::any visitAtomT(MitlParser::AtomTContext *ctx) override;

    std::any visitAtomFn(MitlParser::AtomFnContext *ctx) override;

    std::any visitAtomOn(MitlParser::AtomOnContext *ctx) override;

    std::any visitAtomGn(MitlParser::AtomGnContext *ctx) override;

    std::any visitAtomHn(MitlParser::AtomHnContext *ctx) override;

    std::any visitAtomCFn(MitlParser::AtomCFnContext *ctx) override;

    std::any visitAtomCOn(MitlParser::AtomCOnContext *ctx) override;

    std::any visitAtomCGn(MitlParser::AtomCGnContext *ctx) override;

    std::any visitAtomCHn(MitlParser::AtomCHnContext *ctx) override;

    std::any visitAtomParen(MitlParser::AtomParenContext *ctx) override;

    std::any visitAtomTrue(MitlParser::AtomTrueContext *ctx) override;

    std::any visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) override;

    std::any visitAtomFalse(MitlParser::AtomFalseContext *ctx) override;


};

}  // namespace mightypplcpp
