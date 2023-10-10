#pragma once

#include "types.h"
#include "TAwithBDDEdges.h"
#include "StatewithBDDEdges.h"
#include "Fixpoint.h"

#include "bdd.h"

// namespace mightylcpp {
// 
//     extern void allsat_print_handler(char*, int);
//     extern std::vector<std::string> sat_paths;
//     extern std::vector<std::string> get_letters(const std::string&);
// 
// }

namespace monitaal {

    class FixpointwithBDDEdges : public Fixpoint {
    public:

        static bdd_symbolic_state_map_t reach(const bdd_symbolic_state_map_t& states, const TAwithBDDEdges& T);

        static bdd_symbolic_state_map_t accept_states(const TAwithBDDEdges& T);

        static bdd_symbolic_state_map_t buchi_accept_fixpoint(const TAwithBDDEdges& T);

    };

}

