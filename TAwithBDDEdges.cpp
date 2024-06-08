#include "TAwithBDDEdges.h"

namespace monitaal {

    bdd_edge_t::bdd_edge_t(location_id_t from, location_id_t to, const constraints_t& guard, const clocks_t& reset, const bdd_label_t& bdd_label) :
            edge_t(from, to, guard, reset, label_t{}), _bdd_label(bdd_label) {}

    bdd_label_t bdd_edge_t::bdd_label() const { return _bdd_label; }

    TAwithBDDEdges::TAwithBDDEdges(std::string name, clock_map_t clocks, const locations_t &locations, const bdd_edges_t &bdd_edges, location_id_t initial) :
            TA(name, clocks, locations, edges_t{}, initial) {

        bdd_edge_map_t backward_bdd_edges, forward_bdd_edges;

        for (const auto &l : locations) {

            bdd_edges_t to, from;
            for (const auto &e : bdd_edges) {
                if (e.to() == l.id())
                    to.push_back(e);
                if (e.from() == l.id())
                    from.push_back(e);
            }
            backward_bdd_edges.insert({l.id(), to});
            forward_bdd_edges.insert({l.id(), from});

        }

        _backward_bdd_edges = std::move(backward_bdd_edges);
        _forward_bdd_edges = std::move(forward_bdd_edges);
    }

    const bdd_edges_t& TAwithBDDEdges::bdd_edges_to(location_id_t id) const { return _backward_bdd_edges.at(id); }

    const bdd_edges_t& TAwithBDDEdges::bdd_edges_from(location_id_t id) const { return _forward_bdd_edges.at(id); }


    void TAwithBDDEdges::intersection(const TAwithBDDEdges &other) {

        // TODO: This is a temporary adjustment in response to the off-by-one error
        // introduced by https://github.com/DEIS-Tools/MoniTAal/commit/2207cb9
        
        auto clock_size = this->number_of_clocks() - 2;

        std::map<std::pair<location_id_t, location_id_t>, std::pair<location_id_t, location_id_t>> new_loc_indir;
        location_id_t tmp_id = 0;
        for (const auto& [id1, loc1] : this->locations()) {
            for (const auto& [id2, loc2] : other.locations()) {
                new_loc_indir.insert({{loc1.id(), loc2.id()}, {tmp_id, tmp_id+1}});
                tmp_id += 2;
            }
        }

        bdd_edges_t new_bdd_edges;
        for (const auto& [l1, vec1] : this->_forward_bdd_edges)
            for (const auto& e1 : vec1)
                for (const auto& [l2, vec2] : other._forward_bdd_edges)
                    for (const auto& e2 : vec2) {

 //                       std::cout << "(" << l1 << ", " << l2 << ")" << std::endl;
 //                       std::cout << "bdd_satcount(e1.bdd_label()) == " << bdd_satcount(e1.bdd_label()) << std::endl;
 //                       std::cout << "bdd_satcount(e2.bdd_label()) == " << bdd_satcount(e2.bdd_label()) << std::endl;
                        if (bdd_satcount(e1.bdd_label() & e2.bdd_label()) > 0) {

//                            std::cout << "(" << l1 << ", " << l2 << ") syncs" << std::endl;
                            constraints_t guard(e1.guard());
                            for (const auto& c : e2.guard()) {
                                guard.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_size),
                                                              (c._j == 0 ? 0 : c._j + clock_size), c._bound));
                            }

                            clocks_t reset(e1.reset());
                            for (const auto& r : e2.reset())
                                reset.push_back(r == 0 ? 0 : r + clock_size);

                            bdd new_bdd_label = e1.bdd_label() & e2.bdd_label();

                            const auto& [to_l1, to_l2] = new_loc_indir.at({e1.to(), e2.to()});
                            const auto& [from_l1, from_l2] = new_loc_indir.at({e1.from(), e2.from()});

                            bdd_edge_t new_e1(from_l1, (this->locations().at(e1.from()).is_accept() 
                                                     ? to_l2 : to_l1), guard, reset, new_bdd_label);

                            bdd_edge_t new_e2(from_l2, (other.locations().at(e2.from()).is_accept() 
                                                     ? to_l1 : to_l2), guard, reset, new_bdd_label);

                            new_bdd_edges.push_back(new_e1);
                            new_bdd_edges.push_back(new_e2);
                        }
                    }


        ((TA*)this)->intersection((TA)other);

        bdd_edge_map_t backward_bdd_edges, forward_bdd_edges;

        for (const auto& [_, l] : this->locations()) {

            bdd_edges_t to, from;
            for (const auto &e : new_bdd_edges) {
                if (e.to() == l.id())
                    to.push_back(e);
                if (e.from() == l.id())
                    from.push_back(e);
            }
            backward_bdd_edges.insert({l.id(), to});
            forward_bdd_edges.insert({l.id(), from});

        }

        this->_backward_bdd_edges = std::move(backward_bdd_edges);
        this->_forward_bdd_edges = std::move(forward_bdd_edges);

    }

    TAwithBDDEdges TAwithBDDEdges::intersection(const std::vector<TAwithBDDEdges>& components) {
        
        assert(components.size() > 0);

        if (components.size() == 1) {
            return components[0];
        }

        std::cout << "Doing intersection for " << components.size() << " components..." << std::endl;

        clock_map_t new_clocks;
        std::vector<size_t> clock_offsets(components.size(), 0);

        size_t index = 0;
        new_clocks.insert({0, "x0"});

        for (size_t i = 0; i < components.size(); ++i) {

            // TODO: This is a temporary adjustment in response to the off-by-one error
            // introduced by https://github.com/DEIS-Tools/MoniTAal/commit/2207cb9

            for (size_t j = 0; j < components[i].number_of_clocks() - 1; ++j) {
                if (j == 0) {
                    clock_offsets[i] = index;
                } else {
                    new_clocks.insert({++index, components[i].clock_name(j) + "_" + std::to_string(i)});
                }
            }
        }

        // std::cout << "Total: " << new_clocks.size() << " clocks." << std::endl;
        
        std::vector<location_id_t> location_ids(components.size(), 0);
        for (size_t i = 0; i < components.size(); ++i) {

            location_ids[i] = components[i].initial_location();

        }

        location_id_t tmp_id = 0;

        std::map<std::vector<location_id_t>, std::map<size_t, location_id_t>> new_loc_indir;
        std::map<location_id_t, std::vector<location_id_t>> id_location_ids_map;
        std::map<location_id_t, size_t> id_i_map;

        locations_t new_locations_reachable;
        bdd_edges_t new_bdd_edges_reachable;

        std::set<location_id_t> new_location_ids_expanded;
        
        // location_map_t new_loc_map;
        // bdd_edge_map_t new_bdd_edge_map;

        // for (const auto &l : new_locations) {
        //     new_loc_map.insert({l.id(), l});
        // }

        // for (const auto &e : new_bdd_edges) {
        //     new_bdd_edge_map[e.from()].push_back(e);
        // }

        constraints_t constr;
        for (size_t i = 0; i < components.size(); ++i) {

            for (const auto& c : components[i].locations().at(location_ids[i]).invariant()) {
                constr.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_offsets[i]),
                                              (c._j == 0 ? 0 : c._j + clock_offsets[i]), c._bound));
            }

        }

        std::string name;
        for (size_t i = 0; i < components.size(); ++i) {
            name += components[i].locations().at(location_ids[i]).name() + (components[i].locations().at(location_ids[i]).is_accept() ? "*" : "") + "_";
        }

        new_locations_reachable.push_back(location_t(components[0].locations().at(location_ids[0]).is_accept(), tmp_id, name + std::to_string(0), constr));
        std::cout << "Adding location " << tmp_id << (components[0].locations().at(location_ids[0]).is_accept() ?  " *ACCEPTING*" : std::string{}) << std::endl;

        new_loc_indir.insert({location_ids, {}});
        new_loc_indir.at(location_ids).insert({0, tmp_id});
        id_location_ids_map.insert({tmp_id, location_ids});
        id_i_map.insert({tmp_id++, 0});

        std::set<location_id_t> fringe;

        fringe.insert(new_loc_indir.at(location_ids).at(0));

        size_t curr_i;

        while (!fringe.empty()) {

            auto lid = fringe.begin();
            fringe.erase(lid);

            location_ids = id_location_ids_map.at(*lid);
            curr_i = id_i_map.at(*lid);
            new_location_ids_expanded.insert(*lid);

            bool stucked = false;
            for (size_t i = 0; i < components.size(); ++i) {

                if (components[i].bdd_edges_from(location_ids[i]).size() == 0) {
                    stucked = true;
                }

            }

            // We don't currently remove "dead-end" locations

            if (!stucked) {

                std::vector<size_t> bdd_edge_indices(components.size(), 0);
                std::vector<location_id_t> dest_location_ids(components.size(), 0);

                bool incremented = false;

                do {

                    bdd new_bdd_label = bdd_true();
                    bool contradiction = false;
                    size_t last_index; 
                    for (size_t i = 0; i < components.size() && !contradiction; ++i) {
                        new_bdd_label = new_bdd_label & components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).bdd_label();
                        if (new_bdd_label == bdd_false()) {
                            contradiction = true;
                            last_index = i;
                        }
                    }


                    if (!contradiction) {

                        for (size_t i = 0; i < components.size(); ++i) {

                            dest_location_ids[i] = components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).to();

                        }

                        constraints_t guard;
                        for (size_t i = 0; i < components.size(); ++i) {

                            for (const auto& c : components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).guard()) {
                                guard.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_offsets[i]),
                                                              (c._j == 0 ? 0 : c._j + clock_offsets[i]), c._bound));
                            }

                        }


                        clocks_t reset;
                        for (size_t i = 0; i < components.size(); ++i) {

                            for (const auto& r : components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).reset()) {

                                assert(r != 0);
                                reset.push_back(r + clock_offsets[i]);

                            }

                        }

                        constraints_t constr;
                        for (size_t i = 0; i < components.size(); ++i) {

                            for (const auto& c : components[i].locations().at(dest_location_ids[i]).invariant()) {
                                constr.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_offsets[i]),
                                                              (c._j == 0 ? 0 : c._j + clock_offsets[i]), c._bound));
                            }

                        }

                        std::string name;
                        for (size_t i = 0; i < components.size(); ++i) {
                            name += components[i].locations().at(dest_location_ids[i]).name() + (components[i].locations().at(dest_location_ids[i]).is_accept() ? "*" : "") + "_";
                        }

                        // "Jumping" over counter values
                        // Here we already enforced that components.size >= 2
                        //
                        int new_i = curr_i;
                        
                        if (components[curr_i].locations().at(location_ids[curr_i]).is_accept()) {

                            bool full_circle = false;
                            while (!full_circle && components[new_i].locations().at(location_ids[new_i]).is_accept()) { 

                                if (new_i == components.size() - 1) {
                                    new_i = 0;
                                    full_circle = true;
                                } else {
                                    ++new_i; 
                                }

                            }

                        }


                        if (new_loc_indir.count(dest_location_ids)) {
                            
                            if (!new_loc_indir.at(dest_location_ids).count(new_i)) {

                                new_locations_reachable.push_back(location_t((new_i == 0 ? components[new_i].locations().at(dest_location_ids[new_i]).is_accept() : false), tmp_id, name + std::to_string(new_i), constr));
                                std::cout << "Adding location " << tmp_id << ((new_i == 0 ? components[new_i].locations().at(dest_location_ids[new_i]).is_accept() : false) ?  " *ACCEPTING*" : std::string{}) << std::endl;
                                new_loc_indir.at(dest_location_ids).insert({new_i, tmp_id});
                                id_location_ids_map.insert({tmp_id, dest_location_ids});
                                id_i_map.insert({tmp_id++, new_i});

                            }

                        } else {

                            new_locations_reachable.push_back(location_t((new_i == 0 ? components[new_i].locations().at(dest_location_ids[new_i]).is_accept() : false), tmp_id, name + std::to_string(new_i), constr));
                            std::cout << "Adding location " << tmp_id << ((new_i == 0 ? components[new_i].locations().at(dest_location_ids[new_i]).is_accept() : false) ?  " *ACCEPTING*" : std::string{}) << std::endl;
                            new_loc_indir.insert({dest_location_ids, {}});
                            new_loc_indir.at(dest_location_ids).insert({new_i, tmp_id});
                            id_location_ids_map.insert({tmp_id, dest_location_ids});
                            id_i_map.insert({tmp_id++, new_i});

                        }



                        std::vector<location_id_t> old_location_ids = id_location_ids_map.at(new_loc_indir.at(location_ids).at(curr_i));

                        std::string old_name;
                        for (size_t i = 0; i < components.size(); ++i) {
                            old_name += components[i].locations().at(old_location_ids[i]).name() + (components[i].locations().at(old_location_ids[i]).is_accept() ? "*" : "") + "_";
                        }

                        new_bdd_edges_reachable.push_back(bdd_edge_t(new_loc_indir.at(location_ids).at(curr_i), new_loc_indir.at(dest_location_ids).at(new_i), guard, reset, new_bdd_label));
                        std::cout << "Adding edge from " << new_loc_indir.at(location_ids).at(curr_i) << " (" << old_name + std::to_string(curr_i) << ")" << " -> "  << new_loc_indir.at(dest_location_ids).at(new_i) << " (" << name + std::to_string(new_i) << ")" << std::endl;


                        size_t dest_id;

                        dest_id = new_loc_indir.at(dest_location_ids).at(new_i);

                        if (!new_location_ids_expanded.count(dest_id)) {
                            fringe.insert(dest_id);
                        }

                    }

                    if (!contradiction) {

                        incremented = false;
                        for (int i = components.size() - 1; !incremented && i >= 0; --i) {

                            if (bdd_edge_indices[i] < components[i].bdd_edges_from(location_ids[i]).size() - 1) {

                                ++bdd_edge_indices[i];
                                incremented = true;

                            } else {

                                bdd_edge_indices[i] = 0;

                            }

                        }

                    } else {

                        incremented = false;
                        for (int i = last_index; !incremented && i >= 0; --i) {

                            if (bdd_edge_indices[i] < components[i].bdd_edges_from(location_ids[i]).size() - 1) {

                                ++bdd_edge_indices[i];
                                incremented = true;

                            } else {

                                bdd_edge_indices[i] = 0;

                            }

                        }

                    }

                } while (incremented);
                
            }

        }


        std::cout << "new_clocks.size() == " << new_clocks.size() << std::endl;
        std::cout << "new_locations_reachable.size() == " << new_locations_reachable.size() << std::endl;
        std::cout << "new_bdd_edges_reachable.size() == " << new_bdd_edges_reachable.size() << std::endl;

        for (size_t i = 0; i < components.size(); ++i) {

            location_ids[i] = components[i].initial_location();

        }

        return TAwithBDDEdges("product", new_clocks, new_locations_reachable, new_bdd_edges_reachable, new_loc_indir.at(location_ids).at(0));

    }

    TA TAwithBDDEdges::projection(const std::set<int>& props_to_remove) {

        edges_t edges;
        bdd projected_e;
        bdd new_props = bdd_true();
        for (const auto& i : props_to_remove) {
            new_props = new_props & bdd_ithvar(i);
        }
            
        std::string s;
        for (const auto& [id, l] : this->locations()) {

            for (const auto& e : this->bdd_edges_from(id)) {

                // std::cout << "Before projection: " << e.bdd_label() << std::endl;

                bdd_allsat(e.bdd_label(), *mightylcpp::allsat_print_handler);
                mightylcpp::sat_paths.clear();

                projected_e = bdd_exist(e.bdd_label(), new_props);

                // std::cout << "After projection: " << projected_e << std::endl;

                bdd_allsat(projected_e, *mightylcpp::allsat_print_handler);

                for (const auto& p : mightylcpp::sat_paths) {

                    if (!s.empty()) {
                        s += " || ";
                    }

                    s += p;

                }

                edges.push_back(monitaal::edge_t(e.from(), e.to(), e.guard(), e.reset(), s));     // from, to, guard, reset, label
                mightylcpp::sat_paths.clear();
                s.clear();

                // std::set<std::string> labels_set;

                // for (const auto& p : mightylcpp::sat_paths) {
                //     for (const auto& s : mightylcpp::get_letters(p)) {
                //         std::string projected_s;
                //         for (const auto& i : props_to_keep) {
                //             projected_s += s[i];
                //         }
                //         labels_set.insert(projected_s);
                //     }
                // }

                // mightylcpp::sat_paths.clear();

                // for (const auto& label : labels_set) {
                //     edges.push_back(monitaal::edge_t(e.from(), e.to(), e.guard(), e.reset(), label));     // from, to, guard, reset, label
                // }

            }

        }

        clock_map_t clocks;

        // TODO: This is a temporary adjustment in response to the off-by-one error
        // introduced by https://github.com/DEIS-Tools/MoniTAal/commit/2207cb9

        for (clock_index_t i = 0; i < this->number_of_clocks() - 1; ++i) {
            clocks.insert({i, this->clock_name(i)});
        }

        locations_t locations;
        for (const auto& [id, l] : this->locations()) {
            locations.push_back(l);
        }

        std::cout << "\nProjected TA: " << std::endl;
        std::cout << "clocks.size() == " << clocks.size() << std::endl;
        std::cout << "locations.size() == " << locations.size() << std::endl;
        std::cout << "bdd_edges.size() == " << edges.size() << std::endl;

        return TA("projected", clocks, locations, edges, this->initial_location());

    }

    TAwithBDDEdges TAwithBDDEdges::time_divergence_ta(const bdd& any) {

        clock_map_t clocks;
        clocks.insert({0, "0"});
        clocks.insert({1, "div_clock"});

        locations_t locations;
        locations.push_back(location_t(true, 0, "time_div1", constraints_t{}));
        locations.push_back(location_t(false, 1, "time_div2", constraints_t{}));

        bdd_edges_t bdd_edges;

        bdd_edges.push_back(bdd_edge_t(1, 1, constraints_t{constraint_t::upper_strict(1, 1)}, clocks_t{}, any));
        bdd_edges.push_back(bdd_edge_t(0, 1, constraints_t{}, clocks_t{1}, any));
        bdd_edges.push_back(bdd_edge_t(1, 0, constraints_t{constraint_t::lower_non_strict(1, 1)}, clocks_t{}, any));

        return TAwithBDDEdges("time_divergence", clocks, locations, bdd_edges, 0);

    }

    std::ostream& operator<<(std::ostream& out, const TAwithBDDEdges& T) {

        out << (const TA&)T; 

        out << "\n  BDD Edges:\n";
        for (const auto& es : T._backward_bdd_edges) {
            for (const auto& e : es.second) {
                out << " label: " << '\n';
                bdd_printset(e.bdd_label());
            }
        }

        return out;
    }
}
