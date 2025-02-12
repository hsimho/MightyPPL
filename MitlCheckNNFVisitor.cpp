
// Generated from Mitl.g4 by ANTLR 4.13.0
//

#include "MitlCheckNNFVisitor.h"


namespace mightypplcpp {

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

        // TODO: less hacky way to identify the type of rule?

        if (ctx->formula(0)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == TRIGGER) {

                assert(("Confusing use of &&: Write (pUq)&&r or pU(q&&r) instead of pUq&&r", false));

            }

        }

        if (ctx->formula(1)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == TRIGGER) {

                assert(("Confusing use of &&: Write (pUq)&&r or pU(q&&r) instead of pUq&&r", false));

            }

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->formula(1)));
        return ret_0 && ret_1;

    }
    
    std::any MitlCheckNNFVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        if (ctx->formula(0)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == TRIGGER) {

                assert(("Confusing use of <->: Write (pUq)<->r or pU(q<->r) instead of pUq<->r", false));

            }

        }

        if (ctx->formula(1)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == TRIGGER) {

                assert(("Confusing use of <->: Write (pUq)<->r or pU(q<->r) instead of pUq<->r", false));

            }

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->formula(1)));
        return false;

    }
    
    std::any MitlCheckNNFVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        if (ctx->formula(0)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == TRIGGER) {

                assert(("Confusing use of ->: Write (pUq)->r or pU(q->r) instead of pUq->r", false));

            }

        }

        if (ctx->formula(1)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == TRIGGER) {

                assert(("Confusing use of ->: Write (pUq)->r or pU(q->r) instead of pUq->r", false));

            }

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->formula(1)));
        return false;

    }
    
    std::any MitlCheckNNFVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        // TODO: less hacky way to identify the type of rule?

        if (ctx->atom()->children.size() != 1) {

            if (ctx->atom()->type == UNTIL || ctx->atom()->type == SINCE || ctx->atom()->type == RELEASE || ctx->atom()->type == TRIGGER) {

                assert(("Confusing use of !: Write (!p)Uq or !(pUq) instead of !pUq", false));

            }

            bool ret = std::any_cast<bool>(visit(ctx->atom()));
            return false;

        } else {      // AtomTrue, AtomFalse, or AtomIdfr
            if (ctx->atom()->children[0]->getText() == "true" || ctx->atom()->children[0]->getText() == "false") {
                return false;
            }
        }

        return true;

    }
    
    std::any MitlCheckNNFVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        if (ctx->formula(0)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(0))->atom()->type == TRIGGER) {

                assert(("Confusing use of ||: Write (pUq)||r or pU(q||r) instead of pUq||r", false));

            }

        }
        if (ctx->formula(1)->children.size() == 1) {

            if (static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == UNTIL || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == SINCE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == RELEASE || static_cast<MitlParser::FormulaAtomContext*>(ctx->formula(1))->atom()->type == TRIGGER) {

                assert(("Confusing use of ||: Write (pUq)||r or pU(q||r) instead of pUq||r", false));

            }

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->formula(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->formula(1)));
        return ret_0 && ret_1;

    }
    
    std::any MitlCheckNNFVisitor::visitBound(MitlParser::BoundContext *ctx) {
        assert(("visitBound() should not be called", false));
    }
    
    std::any MitlCheckNNFVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        // This method, unlike others, returns whether the interval is uni 

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

    std::any MitlCheckNNFVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }
        return visit(ctx->atom());
    }

    std::any MitlCheckNNFVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }
        return visit(ctx->atom());
    }
    
    std::any MitlCheckNNFVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }
        return visit(ctx->atom());
    }

    std::any MitlCheckNNFVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }
        return visit(ctx->atom());
    }

    std::any MitlCheckNNFVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        if (ctx->atom(0)->type == UNTIL || ctx->atom(0)->type == SINCE || ctx->atom(0)->type == RELEASE || ctx->atom(0)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->atom(1)->type == UNTIL || ctx->atom(1)->type == SINCE || ctx->atom(1)->type == RELEASE || ctx->atom(1)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

    }

    std::any MitlCheckNNFVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        if (ctx->atom(0)->type == UNTIL || ctx->atom(0)->type == SINCE || ctx->atom(0)->type == RELEASE || ctx->atom(0)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->atom(1)->type == UNTIL || ctx->atom(1)->type == SINCE || ctx->atom(1)->type == RELEASE || ctx->atom(1)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        if (ctx->atom(0)->type == UNTIL || ctx->atom(0)->type == SINCE || ctx->atom(0)->type == RELEASE || ctx->atom(0)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->atom(1)->type == UNTIL || ctx->atom(1)->type == SINCE || ctx->atom(1)->type == RELEASE || ctx->atom(1)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        if (ctx->atom(0)->type == UNTIL || ctx->atom(0)->type == SINCE || ctx->atom(0)->type == RELEASE || ctx->atom(0)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->atom(1)->type == UNTIL || ctx->atom(1)->type == SINCE || ctx->atom(1)->type == RELEASE || ctx->atom(1)->type == TRIGGER) {

            assert(("Add some parentheses to indicate operator precedence", false));

        }

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

    }

    std::any MitlCheckNNFVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));
            assert(("Fn, On, Gn, Hn should be unilateral", ctx->uni));

        }

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));
            assert(("Fn, On, Gn, Hn should be unilateral", ctx->uni));

        }

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomGn(MitlParser::AtomGnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));
            assert(("Fn, On, Gn, Hn should be unilateral", ctx->uni));

        }

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomHn(MitlParser::AtomHnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));
            assert(("Fn, On, Gn, Hn should be unilateral", ctx->uni));

        }

        bool ret = true;
        for (auto i = 0; i < ctx->atoms.size(); ++i) {
            ret = ret && std::any_cast<bool>(visit(ctx->atoms[i]));
        }
        return ret;

    }

    std::any MitlCheckNNFVisitor::visitAtomCFn(MitlParser::AtomCFnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomCGn(MitlParser::AtomCGnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

    }
    
    std::any MitlCheckNNFVisitor::visitAtomCHn(MitlParser::AtomCHnContext *ctx) {

        if (ctx->interval() != nullptr) {

            ctx->uni = std::any_cast<bool>(visit(ctx->interval()));

        }

        bool ret_0 = std::any_cast<bool>(visit(ctx->atom(0)));
        bool ret_1 = std::any_cast<bool>(visit(ctx->atom(1)));
        return ret_0 && ret_1;

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



}  // namespace mightypplcpp

