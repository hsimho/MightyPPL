#pragma once

namespace mightypplcpp {

struct atom_cmp {

    bool operator() (MitlParser::AtomContext* lhs, MitlParser::AtomContext* rhs) const {
        return lhs->id < rhs->id;
    }

};

}  // namespace mightypplcpp
