
// Generated from Mitl.g4 by ANTLR 4.13.0

#include "MitlAtomNumberingVisitor.h"
#include "MitlToNNFVisitor.h"


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

        assert(false);

    }

    std::any MitlAtomNumberingVisitor::visitFormulaImplies(MitlParser::FormulaImpliesContext *ctx) {

        assert(false);

    }

    std::any MitlAtomNumberingVisitor::visitFormulaNot(MitlParser::FormulaNotContext *ctx) {

        return visit(ctx->atom());

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



    std::any MitlAtomNumberingVisitor::visitAtomF(MitlParser::AtomFContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = FINALLY;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomO(MitlParser::AtomOContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = ONCE;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomG(MitlParser::AtomGContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());


        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = GLOBALLY;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomH(MitlParser::AtomHContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = HISTORICALLY;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom())) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom())) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomU(MitlParser::AtomUContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = UNTIL;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomS(MitlParser::AtomSContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = SINCE;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomR(MitlParser::AtomRContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = RELEASE;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomT(MitlParser::AtomTContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->id = ++current_id;
            ctx->type = TRIGGER;
            root->temporals.insert({atom_clean, current_id}); 
            return std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) + 1;

        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            assert(std::any_cast<int>(visit(ctx->atom(0))) + std::any_cast<int>(visit(ctx->atom(1))) == 0);
            return 0;

        }

    }

    std::any MitlAtomNumberingVisitor::visitAtomFn(MitlParser::AtomFnContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->bits = std::ceil(std::log2(ctx->atoms.size() + 1)); // +1: for 0, which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELIFN;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
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

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->bits = std::ceil(std::log2(ctx->atoms.size() + 1)); // +1: for 0, which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELION;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomFnDual(MitlParser::AtomFnDualContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->bits = std::ceil(std::log2(ctx->atoms.size() + 1)); // +1: for 0, which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELIFNDUAL;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomOnDual(MitlParser::AtomOnDualContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            ctx->bits = std::ceil(std::log2(ctx->atoms.size() + 1)); // +1: for 0, which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = PNUELIONDUAL;
            size_t ret = 0;
            for (auto i = 0; i < ctx->atoms.size(); ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atoms[i]));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
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

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            ctx->max_l = std::floor(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + 1;
            ctx->bits = std::ceil(std::log2(ctx->max_l + 2)); // +2: Pn / \neg Pn with l + 1 arguments, and "0" which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = COUNTFN;
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomCOn(MitlParser::AtomCOnContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            ctx->max_l = std::floor(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + 1;
            ctx->bits = std::ceil(std::log2(ctx->max_l + 2)); // +2: Pn / \neg Pn with l + 1 arguments, and "0" which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = COUNTON;
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomCFnDual(MitlParser::AtomCFnDualContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            ctx->max_l = std::floor(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + 1;
            ctx->bits = std::ceil(std::log2(ctx->max_l + 2)); // +2: Pn / \neg Pn with l + 1 arguments, and "0" which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = COUNTFNDUAL;
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            assert(ret == 0);
            return 0;

        }

    }
    
    std::any MitlAtomNumberingVisitor::visitAtomCOnDual(MitlParser::AtomCOnDualContext *ctx) {

        MitlToNNFVisitor to_nnf_visitor;
        std::string atom_clean = std::any_cast<std::string>(to_nnf_visitor.visit(ctx));
        atom_clean.erase(std::remove_if(atom_clean.begin(), atom_clean.end(), [](unsigned char x) { return std::isspace(x); }), atom_clean.end());

        if (root->temporals.count(atom_clean) == 0) {

            antlr4::tree::TerminalNode* left_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[0];
            antlr4::tree::TerminalNode* right_delim = (antlr4::tree::TerminalNode*)ctx->interval()->children[4];

            antlr4::tree::ParseTree* left = (antlr4::tree::ParseTree*)ctx->interval()->children[1];
            antlr4::tree::ParseTree* right = (antlr4::tree::ParseTree*)ctx->interval()->children[3];

            ctx->max_l = std::floor(std::stoi(left->children[0]->getText()) / (std::stoi(right->children[0]->getText()) - std::stoi(left->children[0]->getText()))) + 1;
            ctx->bits = std::ceil(std::log2(ctx->max_l + 2)); // +2: Pn / \neg Pn with l + 1 arguments, and "0" which represents "all off"

            ctx->id = ++current_id;
            root->temporals.insert({atom_clean, current_id}); 
            current_id = current_id + ctx->bits - 1;
            ctx->type = COUNTONDUAL;
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            return (int)(ret + ctx->bits);


        } else {

            ctx->id = root->temporals.at(atom_clean);
            root->repeats.insert(ctx->id);
            size_t ret = 0;
            for (auto i = 0; i < 4; ++i) {
                ret = ret + std::any_cast<int>(visit(ctx->atom(i)));
            }
            assert(ret == 0);
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

