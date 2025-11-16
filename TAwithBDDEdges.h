#pragma once

#include <map>
#include <set>
#include <vector>
#include <cassert>
#include <optional>
#include <numeric>

#include "types.h"
#include "TA.h"

#include "bdd.h"

namespace mightypplcpp {

    extern int gcd;
    extern bool last_intersection;

    extern void allsat_print_handler(char*, int);
    extern std::vector<std::string> sat_paths;
    extern std::vector<std::string> get_letters(const std::string&);

    extern std::optional<bool> out_format;
    extern bool out_fin;
    extern bool debug;
    extern bool back;

}

namespace monitaal {

//    struct location_cmp {
//
//        bool operator() (location_t lhs, location_t rhs) const {
//            return lhs.id() < rhs.id();
//        }
//
//    };
    
    using bdd_label_t = bdd;

    struct bdd_edge_t : public edge_t {

    private:

        const bdd_label_t _bdd_label;

    public:

        bdd_edge_t(location_id_t from, location_id_t to, const constraints_t& guard, const clocks_t& reset, const bdd_label_t& bdd_label);
        [[nodiscard]] bdd_label_t bdd_label() const;

    
    };


    using bdd_edges_t    = std::vector<bdd_edge_t>;
    using bdd_edge_map_t = std::map<location_id_t, bdd_edges_t>;


    class TAwithBDDEdges : public TA {

    private:

        std::string __name;
        bdd_edge_map_t _backward_bdd_edges;
        bdd_edge_map_t _forward_bdd_edges;


    public:


        TAwithBDDEdges(std::string name, clock_map_t clocks, const locations_t &locations, const bdd_edges_t &bdd_edges, location_id_t initial);

        [[nodiscard]] std::string name() const;

        [[nodiscard]] const bdd_edges_t &bdd_edges_to(location_id_t id) const;

        [[nodiscard]] const bdd_edges_t &bdd_edges_from(location_id_t id) const;

        void intersection(const TAwithBDDEdges& other);

        static TAwithBDDEdges intersection(const std::vector<TAwithBDDEdges>& components);

        TA projection(const std::set<int>& props_to_keep);

        TAwithBDDEdges projection_bdd(const std::set<int>& props_to_keep);

        static TAwithBDDEdges time_divergence_ta(const bdd& any);

        friend std::ostream& operator<<(std::ostream& out, const TAwithBDDEdges& T);

//        TA projection(int pos, int len);


    };

}
