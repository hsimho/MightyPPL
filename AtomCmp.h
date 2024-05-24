#pragma once

namespace mightypplcpp {

struct atom_cmp {

    bool operator() (MitlParser::AtomContext* lhs, MitlParser::AtomContext* rhs) const {
        if (lhs->depth() != rhs->depth()) {
            return lhs->depth() < rhs->depth();
        } else {
            return lhs->id < rhs->id;
        }
    }

};

}  // namespace mightypplcpp
