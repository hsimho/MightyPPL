
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlToNNFVisitor.h"


namespace mightypplcpp {

    std::any MitlToNNFVisitor::visitMain(MitlParser::MainContext *ctx) {
        return visit(ctx->formula());
    }
    
    std::any MitlToNNFVisitor::visitFormulaAtom(MitlParser::FormulaAtomContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom()->negated = true;
            output += std::any_cast<std::string>(visit(ctx->atom()));
            ctx->atom()->negated = false;
            return output;

        } else {

            output += std::any_cast<std::string>(visit(ctx->atom()));
            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaAnd(MitlParser::FormulaAndContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = true;

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " || ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;
            return output;

        } else {

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " && ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitFormulaIff(MitlParser::FormulaIffContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = true;

            output += "(";

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " && ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            output += ")";

            output += " || ";

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = false;

            output += "(";

            output += std::any_cast<std::string>(visit(ctx->formula(1)));
            output += " && ";
            output += std::any_cast<std::string>(visit(ctx->formula(0)));

            output += ")";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;

            return output;

        } else {

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = false;

            output += "(";

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " || ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            output += ")";

            output += " && ";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = true;

            output += "(";

            output += std::any_cast<std::string>(visit(ctx->formula(1)));
            output += " || ";
            output += std::any_cast<std::string>(visit(ctx->formula(0)));

            output += ")";

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;

            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->formula(1)->negated = true;

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " && ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            ctx->formula(1)->negated = false;

            return output;

        } else {

            ctx->formula(0)->negated = true;

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " || ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            ctx->formula(0)->negated = false;
            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        std::string output;

        if (ctx->negated) {

            output += std::any_cast<std::string>(visit(ctx->atom()));

            return output;

        } else {

            ctx->atom()->negated = true;

            output += std::any_cast<std::string>(visit(ctx->atom()));

            ctx->atom()->negated = false;

            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->formula(0)->negated = true;
            ctx->formula(1)->negated = true;

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " && ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            ctx->formula(0)->negated = false;
            ctx->formula(1)->negated = false;

            return output;

        } else {

            output += std::any_cast<std::string>(visit(ctx->formula(0)));
            output += " || ";
            output += std::any_cast<std::string>(visit(ctx->formula(1)));

            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitBound(MitlParser::BoundContext *ctx) {

        assert(("visitBound() should not be called", false));

    }
    
    std::any MitlToNNFVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->children[0];
        antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->children[4];

        antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->children[1];
        antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->children[3];

        if (left_delim->getSymbol()->getType() == MitlParser::LBrack && right_delim->getSymbol()->getType() == MitlParser::RBrack) {

            return std::string("[" + left->getText() + ", " + right->getText() + "]");

        } else if (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RBrack) {

            return std::string("(" + left->getText() + ", " + right->getText() + "]");

        } else if (left_delim->getSymbol()->getType() == MitlParser::LBrack && right_delim->getSymbol()->getType() == MitlParser::RParen) {

            return std::string("[" + left->getText() + ", " + right->getText() + ")");

        } else if (left_delim->getSymbol()->getType() == MitlParser::LParen && right_delim->getSymbol()->getType() == MitlParser::RParen) {

            return std::string("(" + left->getText() + ", " + right->getText() + ")");

        } else {

            assert(false);

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom()->negated = true;

            output += ctx->uni ? "G" : "CGn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (false, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            ctx->atom()->negated = false;

            return output;


        } else {

            output += ctx->uni ? "F" : "CFn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (true, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom()->negated = true;

            output += ctx->uni ? "H" : "CHn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (false, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            ctx->atom()->negated = false;

            return output;


        } else {

            output += ctx->uni ? "O" : "COn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (true, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom()->negated = true;

            output += ctx->uni ? "F" : "CFn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (true, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            ctx->atom()->negated = false;

            return output;


        } else {

            output += ctx->uni ? "G" : "CGn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (false, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        std::string output;

        if (ctx->negated) {

            ctx->atom()->negated = true;

            output += ctx->uni ? "O" : "COn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (true, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            ctx->atom()->negated = false;

            return output;


        } else {

            output += ctx->uni ? "H" : "CHn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += ctx->uni ? " " : std::string(" (false, ");
            output += std::any_cast<std::string>(visit(ctx->atom()));
            output += ctx->uni ? "" : ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        std::string output;

        // TODO: less hacky way to identify the type of rule?

        antlr4::RuleContext* ancestor = static_cast<antlr4::RuleContext*>(ctx->parent); 
        bool paren = false;
        while (ancestor != nullptr) {

            if (ancestor->getRuleIndex() == MitlParser::RuleFormula && static_cast<MitlParser::FormulaContext*>(ancestor)->children.size() == 3) {

                paren = true;
                ancestor = nullptr;

            } else if (ancestor->getRuleIndex() == MitlParser::RuleAtom && !(static_cast<MitlParser::AtomContext*>(ancestor)->type == PAREN || static_cast<MitlParser::AtomContext*>(ancestor)->type == UNKNOWN)) {

                paren = true;
                ancestor = nullptr;

            } else {
                ancestor = static_cast<antlr4::RuleContext*>(ancestor->parent);
            }

        }

        if (ctx->negated) {

            ctx->atom(0)->negated = true;
            ctx->atom(1)->negated = true;

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " R" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "CGn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            ctx->atom(0)->negated = false;
            ctx->atom(1)->negated = false;

            return output;


        } else {

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " U" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "CFn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        std::string output;

        // TODO: less hacky way to identify the type of rule?

        antlr4::RuleContext* ancestor = static_cast<antlr4::RuleContext*>(ctx->parent); 
        bool paren = false;
        while (ancestor != nullptr) {

            if (ancestor->getRuleIndex() == MitlParser::RuleFormula && static_cast<MitlParser::FormulaContext*>(ancestor)->children.size() == 3) {

                paren = true;
                ancestor = nullptr;

            } else if (ancestor->getRuleIndex() == MitlParser::RuleAtom && !(static_cast<MitlParser::AtomContext*>(ancestor)->type == PAREN || static_cast<MitlParser::AtomContext*>(ancestor)->type == UNKNOWN)) {

                paren = true;
                ancestor = nullptr;

            } else {
                ancestor = static_cast<antlr4::RuleContext*>(ancestor->parent);
            }

        }

        if (ctx->negated) {

            ctx->atom(0)->negated = true;
            ctx->atom(1)->negated = true;

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " T" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "CHn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            ctx->atom(0)->negated = false;
            ctx->atom(1)->negated = false;

            return output;


        } else {

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " S" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "COn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        std::string output;

        // TODO: less hacky way to identify the type of rule?

        antlr4::RuleContext* ancestor = static_cast<antlr4::RuleContext*>(ctx->parent); 
        bool paren = false;
        while (ancestor != nullptr) {

            if (ancestor->getRuleIndex() == MitlParser::RuleFormula && static_cast<MitlParser::FormulaContext*>(ancestor)->children.size() == 3) {

                paren = true;
                ancestor = nullptr;

            } else if (ancestor->getRuleIndex() == MitlParser::RuleAtom && !(static_cast<MitlParser::AtomContext*>(ancestor)->type == PAREN || static_cast<MitlParser::AtomContext*>(ancestor)->type == UNKNOWN)) {

                paren = true;
                ancestor = nullptr;

            } else {
                ancestor = static_cast<antlr4::RuleContext*>(ancestor->parent);
            }

        }

        if (ctx->negated) {

            ctx->atom(0)->negated = true;
            ctx->atom(1)->negated = true;

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " U" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "CFn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            ctx->atom(0)->negated = false;
            ctx->atom(1)->negated = false;

            return output;


        } else {

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " R" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "CGn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        std::string output;

        // TODO: less hacky way to identify the type of rule?

        antlr4::RuleContext* ancestor = static_cast<antlr4::RuleContext*>(ctx->parent); 
        bool paren = false;
        while (ancestor != nullptr) {

            if (ancestor->getRuleIndex() == MitlParser::RuleFormula && static_cast<MitlParser::FormulaContext*>(ancestor)->children.size() == 3) {

                paren = true;
                ancestor = nullptr;

            } else if (ancestor->getRuleIndex() == MitlParser::RuleAtom && !(static_cast<MitlParser::AtomContext*>(ancestor)->type == PAREN || static_cast<MitlParser::AtomContext*>(ancestor)->type == UNKNOWN)) {

                paren = true;
                ancestor = nullptr;

            } else {
                ancestor = static_cast<antlr4::RuleContext*>(ancestor->parent);
            }

        }

        if (ctx->negated) {

            ctx->atom(0)->negated = true;
            ctx->atom(1)->negated = true;

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " S" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "COn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            ctx->atom(0)->negated = false;
            ctx->atom(1)->negated = false;

            return output;


        } else {

            if (ctx->uni) {

                output += paren ? "(" : "";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += " T" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " ";
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += paren ? ")" : "";

            } else {

                output += "CHn" + (ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "");
                output += " (";
                output += std::any_cast<std::string>(visit(ctx->atom(0)));
                output += ", "; 
                output += std::any_cast<std::string>(visit(ctx->atom(1)));
                output += ")";

            }

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        std::string output;

        if (ctx->negated) {

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = true;
            }

            output += "Gn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = false;
            }

            return output;


        } else {

            output += "Fn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        std::string output;

        if (ctx->negated) {

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = true;
            }

            output += "Hn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = false;
            }

            return output;


        } else {

            output += "On";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomGn(MitlParser::AtomGnContext *ctx) {

        std::string output;

        if (ctx->negated) {

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = true;
            }

            output += "Fn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = false;
            }

            return output;


        } else {

            output += "Gn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomHn(MitlParser::AtomHnContext *ctx) {

        std::string output;

        if (ctx->negated) {

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = true;
            }

            output += "On";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ctx->atoms[i]->negated = false;
            }

            return output;


        } else {

            output += "Hn";
            output += ctx->interval() ? std::any_cast<std::string>(visit(ctx->interval())) : "";
            output += std::string(" (");
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                output += std::any_cast<std::string>(visit(ctx->atoms[i])) + (i != ctx->atoms.size() - 1 ? ", " : "");
            }
            output += ")";

            return output;

        }

    }

    std::any MitlToNNFVisitor::visitAtomCFn(MitlParser::AtomCFnContext *ctx) {

        assert(("The original formula should contain no CFn, COn, CGn, CHn", false));

    }

    std::any MitlToNNFVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        assert(("The original formula should contain no CFn, COn, CGn, CHn", false));

    }

    std::any MitlToNNFVisitor::visitAtomCGn(MitlParser::AtomCGnContext *ctx) {

        assert(("The original formula should contain no CFn, COn, CGn, CHn", false));

    }

    std::any MitlToNNFVisitor::visitAtomCHn(MitlParser::AtomCHnContext *ctx) {

        assert(("The original formula should contain no CFn, COn, CGn, CHn", false));

    }


    
    std::any MitlToNNFVisitor::visitAtomParen(MitlParser::AtomParenContext *ctx) {

        std::string output;

        // TODO: less hacky way to identify the type of rule?
           
        antlr4::RuleContext* ancestor = static_cast<antlr4::RuleContext*>(ctx->parent); 
        bool paren = false;
        while (ancestor != nullptr) {

            if (ancestor->getRuleIndex() == MitlParser::RuleFormula && static_cast<MitlParser::FormulaContext*>(ancestor)->children.size() == 3) {

                paren = true;
                ancestor = nullptr;

            } else if (ancestor->getRuleIndex() == MitlParser::RuleAtom && !(static_cast<MitlParser::AtomContext*>(ancestor)->type == PAREN || static_cast<MitlParser::AtomContext*>(ancestor)->type == UNKNOWN)) {

                paren = true;
                ancestor = nullptr;

            } else {
                ancestor = static_cast<antlr4::RuleContext*>(ancestor->parent);
            }

        }

        if (ctx->negated) {

            ctx->formula()->negated = true;
            if (ctx->formula()->children.size() == 1) {
                output += std::any_cast<std::string>(visit(ctx->formula()));
            } else {
                if (paren) {
                    output += "(" + std::any_cast<std::string>(visit(ctx->formula())) + ")";
                } else {
                    output += std::any_cast<std::string>(visit(ctx->formula()));
                }
            }
            ctx->formula()->negated = false;

            return output;

        } else {

            if (ctx->formula()->children.size() == 1) {
                output += std::any_cast<std::string>(visit(ctx->formula()));
            } else {
                if (paren) {
                    output += "(" + std::any_cast<std::string>(visit(ctx->formula())) + ")";
                } else {
                    output += std::any_cast<std::string>(visit(ctx->formula()));
                }
            }

            return output;

        }
    }
    
    std::any MitlToNNFVisitor::visitAtomTrue(MitlParser::AtomTrueContext *ctx) {

        std::string output;

        if (ctx->negated) {

            output += "false";
            return output;

        } else {

            output += "true";
            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomIdfr(MitlParser::AtomIdfrContext *ctx) {

        std::string output;

        if (ctx->negated) {

            // TODO: less hacky way to identify the type of rule?
           
            // std::cout << "Trying to find the ancestor of " << "!" << ctx->Idfr()->getText() << std::endl << std::flush;
            antlr4::RuleContext* ancestor = static_cast<antlr4::RuleContext*>(ctx->parent); 
            bool paren = false;
            while (ancestor != nullptr) {

                // std::cout << "Trying " << ancestor->getText() << std::endl << std::flush;
                // if (ancestor->getRuleIndex() == MitlParser::RuleFormula) {
                //     
                //     std::cout << "(this is a Formula)" << std::endl << std::flush;

                // } else if (ancestor->getRuleIndex() == MitlParser::RuleAtom) {

                //     std::cout << "(this is an Atom)" << std::endl << std::flush;

                // } else {
                //     assert(false);
                // }

                if (ancestor->getRuleIndex() == MitlParser::RuleFormula && static_cast<MitlParser::FormulaContext*>(ancestor)->children.size() == 3) {
                //    std::cout << "(this is a Formula with two operands)" << std::endl << std::flush;
                    paren = false;
                    ancestor = nullptr;

                } else if (ancestor->getRuleIndex() == MitlParser::RuleAtom &&
                        (static_cast<MitlParser::AtomContext*>(ancestor)->type == FINALLY
                         || static_cast<MitlParser::AtomContext*>(ancestor)->type == ONCE
                         || static_cast<MitlParser::AtomContext*>(ancestor)->type == GLOBALLY
                         || static_cast<MitlParser::AtomContext*>(ancestor)->type == HISTORICALLY
                         || static_cast<MitlParser::AtomContext*>(ancestor)->type == PNUELIFN
                         || static_cast<MitlParser::AtomContext*>(ancestor)->type == PNUELION
                         || static_cast<MitlParser::AtomContext*>(ancestor)->type == PNUELIGN
                         || static_cast<MitlParser::AtomContext*>(ancestor)->type == PNUELIFN)) {
                //    std::cout << "(this is an Atom in F O G H Fn On Gn Hn)" << std::endl << std::flush;
                    paren = true;
                    ancestor = nullptr;

                } else {
                //    std::cout << "go further up" << std::endl << std::flush;
                    ancestor = static_cast<antlr4::RuleContext*>(ancestor->parent);
                }

            }

           
            if (paren) {
                output += "(!" + ctx->Idfr()->getText() + ")";
            } else {
                output += "!" + ctx->Idfr()->getText();
            }
            return output;

        } else {

            output += ctx->Idfr()->getText();
            return output;

        }

    }
    
    std::any MitlToNNFVisitor::visitAtomFalse(MitlParser::AtomFalseContext *ctx) {

        std::string output;

        if (ctx->negated) {

            output += "true";
            return output;

        } else {

            output += "false";
            return output;

        }

    }
    


}  // namespace mightypplcpp

