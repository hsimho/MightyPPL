
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MightyPPL.h"
#include "MitlGetBDDVisitor.h"


namespace mightypplcpp {


    std::any MitlGetBDDVisitor::visitMain(MitlParser::MainContext *ctx) {

        root = ctx;
        visit(ctx->formula());

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {

        visit(ctx->atom());

        ctx->overline = ctx->atom()->overline;
        ctx->star = ctx->atom()->star;
        ctx->tilde = ctx->atom()->tilde;
        ctx->hat = ctx->atom()->hat;

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {

        visit(ctx->formula(0));
        visit(ctx->formula(1));

        ctx->overline = ctx->formula(0)->overline & ctx->formula(1)->overline;
        ctx->star = ctx->formula(0)->star & ctx->formula(1)->star;
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = ctx->formula(0)->hat & ctx->formula(1)->hat;

        return nullptr;
        
    }

    std::any MitlGetBDDVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        assert(("The formula should be in NNF now", false));

    }

    std::any MitlGetBDDVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        assert(("The formula should be in NNF now", false));

    }

    std::any MitlGetBDDVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        visit(ctx->atom());

       // indeed as the whole formula is in NNF, ctx->atom() is always propositional
       // (exactly 1 original proposition).

        ctx->overline = !ctx->atom()->overline;
        ctx->star = ctx->atom()->star;
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = !ctx->atom()->hat;

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        visit(ctx->formula(0));
        visit(ctx->formula(1));

        ctx->overline = ctx->formula(0)->overline | ctx->formula(1)->overline;
        ctx->star = ctx->formula(0)->star & ctx->formula(1)->star;
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = (ctx->formula(0)->hat & ctx->formula(1)->tilde) | (ctx->formula(1)->hat & ctx->formula(0)->tilde) | (ctx->formula(0)->overline & ctx->formula(1)->overline & ctx->formula(0)->star & ctx->formula(1)->star);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitBound(MitlParser::BoundContext *ctx) {

        assert(("visitBound() should not be called", false));

    }

    std::any MitlGetBDDVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        assert(("visitInterval() should not be called", false));

    }


    std::any MitlGetBDDVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {
        

        visit(ctx->atom());

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {
        

        visit(ctx->atom());

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {
        

        visit(ctx->atom());

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {
        

        visit(ctx->atom());

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {
        
        visit(ctx->atom(0));
        visit(ctx->atom(1));

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {
        
        visit(ctx->atom(0));
        visit(ctx->atom(1));

        ctx->overline = bdd_ithvar(ctx->id);
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id));
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(ctx->id);

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {


        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }

        ctx->overline = bdd_false(); 
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ctx->overline = ctx->overline | encode(i + 1, ctx->id, ctx->bits / 2 - 1);
        }

        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id, ctx->bits / 2 - 1));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        ctx->hat = ctx->overline; 
        ctx->comp_hat = ctx->hat;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);

        }

        return nullptr;

    }
    
    std::any MitlGetBDDVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }

        ctx->overline = bdd_false(); 
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ctx->overline = ctx->overline | encode(i + 1, ctx->id + ctx->bits / 2, ctx->bits / 2 - 1);
        }

        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id + ctx->bits / 2, ctx->bits / 2 - 1));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        ctx->hat = ctx->overline; 
        ctx->comp_hat = ctx->hat;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);

        }
        
        return nullptr;

    }
    
    std::any MitlGetBDDVisitor::visitAtomGn(MitlParser::AtomGnContext *ctx) {

        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }

        ctx->overline = bdd_false(); 
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ctx->overline = ctx->overline | encode(i + 1, ctx->id, ctx->bits / 2 - 1);
        }

        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id, ctx->bits / 2 - 1));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        ctx->hat = ctx->overline; 
        ctx->comp_hat = ctx->hat;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);

        }

        return nullptr;

    }
    
    std::any MitlGetBDDVisitor::visitAtomHn(MitlParser::AtomHnContext *ctx) {

        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            visit(ctx->atoms[i]);
        }

        ctx->overline = bdd_false(); 
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ctx->overline = ctx->overline | encode(i + 1, ctx->id + ctx->bits / 2, ctx->bits / 2 - 1);
        }

        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id + ctx->bits / 2, ctx->bits / 2 - 1));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        ctx->hat = ctx->overline; 
        ctx->comp_hat = ctx->hat;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);

        }

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomCFn(MitlParser::AtomCFnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));

        ctx->overline = bdd_false(); 
        ctx->hat = bdd_false();

        for (auto i = 0; i < ctx->num_pairs; ++i) {
            ctx->overline = ctx->overline | (encode(i + 1, ctx->id, ctx->bits / 2 - 2) & bdd_ithvar(ctx->id + ctx->bits / 2 - 2));
            ctx->hat = ctx->hat | (encode(i + 1, ctx->id, ctx->bits / 2 - 2) & bdd_ithvar(ctx->id + ctx->bits / 2 - 2));
        }

        ctx->comp_hat = ctx->hat;
        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id, ctx->bits / 2 - 2) & !bdd_ithvar(ctx->id + ctx->bits / 2 - 2));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);

        }

        return nullptr;

    }
    
    std::any MitlGetBDDVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));

        ctx->overline = bdd_false(); 
        ctx->hat = bdd_false();

        for (auto i = 0; i < ctx->num_pairs; ++i) {
            ctx->overline = ctx->overline | (encode(i + 1, ctx->id + ctx->bits / 2, ctx->bits / 2 - 2) & bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 2));
            ctx->hat = ctx->hat | (encode(i + 1, ctx->id + ctx->bits / 2, ctx->bits / 2 - 2) & bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 2));
        }
        ctx->comp_hat = ctx->hat;

        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id + ctx->bits / 2, ctx->bits / 2 - 2) & !bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 2));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);

        }

        return nullptr;

    }
    
    std::any MitlGetBDDVisitor::visitAtomCGn(MitlParser::AtomCGnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));

        ctx->overline = bdd_false(); 
        ctx->hat = bdd_false();

        for (auto i = 0; i < ctx->num_pairs; ++i) {
            ctx->overline = ctx->overline | encode(i + 1, ctx->id, ctx->bits / 2 - 2);
            ctx->hat = ctx->hat | (encode(i + 1, ctx->id, ctx->bits / 2 - 2) & !bdd_ithvar(ctx->id + ctx->bits / 2 - 2));
        }

        ctx->overline = ctx->overline | bdd_ithvar(ctx->id + ctx->bits / 2 - 2);
        ctx->hat = ctx->hat | (encode(0, ctx->id, ctx->bits / 2 - 2) & bdd_ithvar(ctx->id + ctx->bits / 2 - 2));
        ctx->comp_hat = ctx->hat;

        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id, ctx->bits / 2 - 2) & !bdd_ithvar(ctx->id + ctx->bits / 2 - 2));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 - 1);

        }

        return nullptr;

    }
    
    std::any MitlGetBDDVisitor::visitAtomCHn(MitlParser::AtomCHnContext *ctx) {

        visit(ctx->atom(0));
        visit(ctx->atom(1));
        
        ctx->overline = bdd_false(); 
        ctx->hat = bdd_false();

        for (auto i = 0; i < ctx->num_pairs; ++i) {
            ctx->overline = ctx->overline | encode(i + 1, ctx->id + ctx->bits / 2, ctx->bits / 2 - 2);
            ctx->hat = ctx->hat | (encode(i + 1, ctx->id + ctx->bits / 2, ctx->bits / 2 - 2) & !bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 2));
        }

        ctx->overline = ctx->overline | bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 2);
        ctx->hat = ctx->hat | (encode(0, ctx->id + ctx->bits / 2, ctx->bits / 2 - 2) & bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 2));
        ctx->comp_hat = ctx->hat;

        ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : encode(0, ctx->id + ctx->bits / 2, ctx->bits / 2 - 2) & !bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 2));
        ctx->comp_star = ctx->star;

        ctx->tilde = !ctx->overline & ctx->star;

        if (comp_flatten) {

            ctx->overline = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);
            ctx->star = (root->repeats.count(ctx->id) ? bdd_true() : !bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1));
            ctx->tilde = !ctx->overline & ctx->star;
            ctx->hat = bdd_ithvar(ctx->id + ctx->bits / 2 + ctx->bits / 2 - 1);

        }

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {

        visit(ctx->formula());

        ctx->overline = ctx->formula()->overline;
        ctx->star = ctx->formula()->star;
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = ctx->formula()->hat;

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {

        ctx->overline = bdd_true();
        ctx->star = bdd_true();
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_true();

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {

        ctx->overline = bdd_ithvar(root->props.at(ctx->Idfr()->getText()));
        ctx->star = bdd_true();
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_ithvar(root->props.at(ctx->Idfr()->getText()));

        return nullptr;

    }

    std::any MitlGetBDDVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {

        ctx->overline = bdd_false();
        ctx->star = bdd_true();
        ctx->tilde = !ctx->overline & ctx->star;
        ctx->hat = bdd_false();

        return nullptr;

    }


}  // namespace mightypplcpp

