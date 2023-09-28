#pragma once

namespace antlrcpptest {

struct atom_cmp {

    bool operator() (MitlParser::AtomContext* lhs, MitlParser::AtomContext* rhs) const {
        return lhs->depth() < rhs->depth();
    }

};

}  // namespace antlrcpptest
