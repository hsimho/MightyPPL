
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlAtomNumberingVisitor.h"

namespace mightypplcpp {

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

        assert(("The formula should be in NNF now", false));

    }

    std::any MitlAtomNumberingVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        assert(("The formula should be in NNF now", false));

    }

    std::any MitlAtomNumberingVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        return visit(ctx->atom());

    }

    std::any MitlAtomNumberingVisitor::visitFormulaOr(MitlParser::FormulaOrContext *ctx) {

        return std::any_cast<int>(visit(ctx->formula(0))) + std::any_cast<int>(visit(ctx->formula(1)));

    }

    std::any MitlAtomNumberingVisitor::visitBound(MitlParser::BoundContext *ctx) {

        assert(("visitBound() should not be called", false));

    }

    std::any MitlAtomNumberingVisitor::visitInterval(MitlParser::IntervalContext *ctx) {

        assert(("visitInterval() should not be called", false));

    }

    std::any MitlAtomNumberingVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = FINALLY;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = ONCE;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = GLOBALLY;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = HISTORICALLY;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = UNTIL;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = SINCE;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = RELEASE;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = TRIGGER;
            root->temporals.insert({ctx->getText(), current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        ctx->bits = 2 * std::ceil(std::log2(ctx->atoms.size() + 1)); // + 1: 0 for "all off"

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELIFN;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomOn(MitlParser::AtomOnContext *ctx) {

        ctx->bits = 2 * std::ceil(std::log2(ctx->atoms.size() + 1)); // + 1: 0 for "all off"

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELION;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomGn(MitlParser::AtomGnContext *ctx) {

        ctx->bits = 2 * std::ceil(std::log2(ctx->atoms.size() + 1)); // + 1: 0 for "all off"

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELIGN;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomHn(MitlParser::AtomHnContext *ctx) {

        ctx->bits = 2 * std::ceil(std::log2(ctx->atoms.size() + 1)); // + 1: 0 for "all off"

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELIHN;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            assert(ret == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomCFn(MitlParser::AtomCFnContext *ctx) {

        antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
        antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

        antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
        antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

        ctx->num_pairs = std::ceil(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + 1;
        ctx->bits = 2 * (std::ceil(std::log2(ctx->num_pairs + 1)) + 1); // + 1: 0 for "all off", another +1 for the other disjunct (both in and out, but only in really needed)

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = COUNTFN;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + (int)ctx->bits;


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
        antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

        antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
        antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

        ctx->num_pairs = std::ceil(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + 1;
        ctx->bits = 2 * (std::ceil(std::log2(ctx->num_pairs + 1)) + 1); // + 1: 0 for "all off", another +1 for the other disjunct (both in and out, but only in really needed)

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = COUNTON;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + (int)ctx->bits;


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomCGn(MitlParser::AtomCGnContext *ctx) {

        antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
        antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

        antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
        antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

        ctx->num_pairs = std::ceil(std::stoi(right->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())));
        ctx->num_pairs = std::ceil(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + (std::stoi(left->children[0]->getText()) % (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) ? 2 : 1);
        ctx->bits = 2 * (std::ceil(std::log2(ctx->num_pairs + 1)) + 1); // + 1: 0 for "all off", another +1 for the other disjunct (both in and out, but only in really needed)

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = COUNTGN;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + (int)ctx->bits;


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomCHn(MitlParser::AtomCHnContext *ctx) {

        antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
        antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

        antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
        antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

        ctx->num_pairs = std::ceil(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + (std::stoi(left->children[0]->getText()) % (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText())) ? 2 : 1);
        ctx->bits = 2 * (std::ceil(std::log2(ctx->num_pairs + 1)) + 1); // + 1: 0 for "all off", another +1 for the other disjunct (both in and out, but only in really needed)

        if (root->temporals.count(ctx->getText()) == 0) {

            ctx->id = ++current_id;
            ctx->type = COUNTHN;
            root->temporals.insert({ctx->getText(), current_id}); 
            current_id = current_id + ctx->bits - 1;
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + (int)ctx->bits;


        } else {

            ctx->id = root->temporals.at(ctx->getText());
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

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


}  // namespace mightypplcpp

