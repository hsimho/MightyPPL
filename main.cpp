#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <stdexcept>
#include <optional>

#include "types.h"
// #include "StatewithBDDEdges.h"
#include "TAwithBDDEdges.h"
// #include "FixpointwithBDDEdges.h"

#include "MightyPPL.h"

#include "Monitor.h"
#include "Parser.h"

using namespace mightypplcpp;
using namespace antlr4;


namespace mightypplcpp {

    const char* spec_file = NULL;
    const char* out_file = NULL;
    std::optional<bool> out_format = std::nullopt;    // true: tck, false: xml
    bool out_flatten = true;
    bool comp_flatten = false;
    bool out_fin = false;
    bool debug = false;
    bool back = true;

    monitaal::TAwithBDDEdges varphi = monitaal::TAwithBDDEdges("dummy", {}, {}, {}, 0);
    monitaal::TAwithBDDEdges varphi_ = monitaal::TAwithBDDEdges("dummy", {}, {}, {}, 0);
    monitaal::TAwithBDDEdges div = monitaal::TAwithBDDEdges("dummy", {}, {}, {}, 0);
    std::vector<monitaal::TAwithBDDEdges> temporal_components;
    monitaal::TAwithBDDEdges model = monitaal::TAwithBDDEdges("dummy", {}, {}, {}, 0);   // last arg: initial location id

//    std::map<std::string, std::pair<std::set<size_t>, std::set<size_t>>> acc_map;

} // namespace mightypplcpp

namespace mightypplcpp {



    try {

        if (argc < 3) {

            throw std::invalid_argument("No spec file / acceptance type specified"); 

        } else {

            spec_file = argv[1];

            if (std::string_view(argv[2]) == "--fin") {

                out_fin = true;

            } else if (std::string_view(argv[2]) == "--inf") {

                out_fin = false;

            } else {

                throw std::invalid_argument("Wrong acceptance type specified"); 

            }

            if (argc > 3) {

                if (argc >= 5) {

                    if (argc > 5) {
                    
                        out_file = argv[3]; 

                        if (std::string_view(argv[4]) == "--tck") {

                            out_format = true;

                        } else if (std::string_view(argv[4]) == "--xml") {

                            out_format = false;

                        } else {

                            throw std::invalid_argument("Wrong output format specified (--tck or --xml?)"); 

                        }


                        if (argc > 8) {

                            throw std::invalid_argument("Too many arguments");

                        } else if (argc == 8) {

                            if (std::string_view(argv[5]) == "--noflatten") {

                                out_flatten = false;

                            } else if (std::string_view(argv[5]) == "--compflatten") {

                                out_flatten = false;
                                comp_flatten = true;

                            } else {

                                throw std::invalid_argument("One of the last 3 arguments was wrong");

                            }

                            if (std::string_view(argv[6]) == "--debug") {

                                debug = true;

                            } else {

                                throw std::invalid_argument("One of the last 3 arguments was wrong");

                            }

                            if (std::string_view(argv[7]) == "--noback") {

                                back = false;

                            } else {

                                throw std::invalid_argument("One of the last 3 arguments was wrong");

                            }

                        } else if (argc == 7) {

                            if (std::string_view(argv[5]) == "--noflatten") {

                                out_flatten = false;

                                if (std::string_view(argv[6]) == "--debug") {

                                    debug = true;

                                } else if (std::string_view(argv[6]) == "--noback") {

                                    back = false;

                                } else {

                                    throw std::invalid_argument("Last argument was wrong");

                                }

                            } else if (std::string_view(argv[5]) == "--compflatten") {

                                out_flatten = false;
                                comp_flatten = true;

                                if (std::string_view(argv[6]) == "--debug") {

                                    debug = true;

                                } else if (std::string_view(argv[6]) == "--noback") {

                                    back = false;

                                } else {

                                    throw std::invalid_argument("Last argument was wrong");

                                }
                                

                            } else {

                                if (std::string_view(argv[5]) == "--debug") {

                                    debug = true;

                                } else {

                                    throw std::invalid_argument("One of the last 2 arguments was wrong");

                                }

                                if (std::string_view(argv[6]) == "--noback") {

                                    back = false;

                                } else {

                                    throw std::invalid_argument("One of the last 2 arguments was wrong");

                                }

                            }


                        } else if (argc == 6) {

                            if (std::string_view(argv[5]) == "--noflatten") {

                                out_flatten = false;

                            } else if (std::string_view(argv[5]) == "--compflatten") {

                                out_flatten = false;
                                comp_flatten = true;

                            } else if (std::string_view(argv[5]) == "--debug") {

                                debug = true;

                            } else if (std::string_view(argv[5]) == "--noback") {

                                back = false;

                            } else {

                                throw std::invalid_argument("The last argument was wrong");

                            }

                        }

                    } else {    // argc == 5
                            

                        if (std::string_view(argv[3]) == "--debug") {

                            debug = true;

                            if (std::string_view(argv[4]) == "--noback") {

                                back = false;

                            } else {

                                throw std::invalid_argument("The last argument was wrong");

                            }

                        } else {

                            out_file = argv[3]; 

                            if (std::string_view(argv[4]) == "--tck") {

                                out_format = true;

                            } else if (std::string_view(argv[4]) == "--xml") {

                                out_format = false;

                            } else {

                                throw std::invalid_argument("Wrong output format specified (--tck or --xml?)"); 

                            }

                        }

                    }

                } else {    // argc == 4

                    if (std::string_view(argv[3]) == "--debug") {

                        debug = true;

                    } else if (std::string_view(argv[3]) == "--noback") {

                        back = false;

                    } else {

                        throw std::invalid_argument("No output format specified (--tck or --xml?)"); 

                    }

                }

            }
                    
        }

    } catch (const std::invalid_argument& e) {

        std::cerr << e.what() << std::endl;
        std::cerr << "Usage: mitppl <in_spec_file> --{fin|inf} [out_file --{tck|xml} [--{noflatten|compflatten}]] [--debug] [--noback]" << std::endl << std::endl;

        std::cerr << "--debug: pauses to see diagostic info" << std::endl;
        std::cerr << "--noback: disables symbolic backward analysis (for Pnueli modalities / MITL modalities with <l, u>; no effect if --noflatten is set)" << std::endl << std::endl;
        std::cerr << "If [out_file ...] is NOT specified, the built-in DBM fixpoint algorithm checks the (Buechi) emptiness of the" << std::endl;
        std::cerr << "(monolithic flattened) product automaton (i.e. the satisfiability of the input formula && TA_div && model M)." << std::endl;
        return 1;

    }

    std::ifstream spec_in(spec_file, std::ios::in);
    if (!spec_in) {

        std::cerr << "Error: Could not open in_spec_file" << std::endl;
        return 1;

    }

    bdd_init(1000, 100);

    std::cout << "\n<<<<<< Pre-processing input formula... >>>>>>\n\n";

    std::cout << "\nInput formula (as read from input):\n" << std::endl;

    std::stringstream buf;
    buf << spec_in.rdbuf();
    std::cout << buf.str();

    std::string nnf_in = buf.str();

    ANTLRInputStream input(nnf_in);
    MitlLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    MitlParser parser(&tokens);

    MitlParser::MainContext* original_formula = parser.main();

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto [pos, out_str] = build_ta_from_main(original_formula);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

   
    if (out_flatten) {
        std::cout << "Constructing TA took = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    }

    // else if (comp_flatten) {
    //     std::cout << "Constructing tester TAs (one for each temporal subformula) took = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    // } else {
    //     std::cout << "Constructing tester and component TAs took = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    // }


    if (!out_format.has_value()) {

        std::chrono::steady_clock::time_point begin2 = std::chrono::steady_clock::now();
        std::cout << "<<<<<< Calculating fixpoint >>>>>>" << std::endl;

        auto initial_state = monitaal::symbolic_state_t(pos.initial_location(), monitaal::Federation::zero(pos.number_of_clocks()));

        if (out_fin) {

            // Note that we assume the finite timed word accepted is of length >= 1 (also enforced by the acc. condition of TA_0)
            if (initial_state.is_included_in(monitaal::Fixpoint::reach(monitaal::Fixpoint::accept_states(pos), pos))) {

                std::cout << "\n\n\033[32mSATISFIABLE (by a \033[1mfinite\033[22m timed word)\033[0m\n\n" << std::endl;

            } else {

                std::cout << "\n\n\033[31mNOT SATISFIABLE (by \033[1mfinite\033[22m timed words)\033[0m\n\n" << std::endl;

            }

        } else {

            if (initial_state.is_included_in(monitaal::Fixpoint::buchi_accept_fixpoint(pos))) {

                std::cout << "\n\n\033[32mSATISFIABLE (by an \033[1minfinite\033[22m timed word)\033[0m\n\n" << std::endl;

            } else {

                std::cout << "\n\n\033[31mNOT SATISFIABLE (by \033[1minfinite\033[22m timed words)\033[0m\n\n" << std::endl;

            }

        }

        std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();
        std::cout << "Fixpoint took = " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin2).count() << "[ms]" << std::endl;

    } else {    // out_format.has_value()

        std::ofstream spec_out(out_file, std::ios::trunc);
        if (!spec_out) {

            std::cerr << "Error: Could not open out_file" << std::endl;
            return 1;

        }

        if (out_format.value()) {       // tck

            if (out_flatten) {

                std::stringstream tck;

                tck << "# File generated by MightyPPL" << std::endl;
                tck << "system:model_and_spec" << std::endl << std::endl << std::endl;

                tck << "event:a" << std::endl << std::endl << std::endl;

                for (auto i = 1; i < pos.number_of_clocks() - 1; ++i) {

                    // In MoniTAal there is always x0 (meant to be 0 at all times)
                    // And there is an extra "global" clock for monitoring in newer versions MoniTAal
                    // So if number_of_clocks() returns 5, only 3 clocks are useful for tck

                    tck << "clock:1:x_" << i - 1 << std::endl;

                }
                tck << std::endl << std::endl;

                tck << "# " << "TA" << std::endl;
                tck << "# " << "This is the (untimely-reachable part of the) product of component automata, TA_0, div, and M" << std::endl;
                tck << "process:" << "TA" << std::endl;

                for (const auto& [k, v] : pos.locations()) {

                    tck << "location:" << "TA" << ":ell_" << k << "{" << (k == pos.initial_location() ?  (v.is_accept() ? "initial: : " : "initial: ") : "")
                        << (v.is_accept() ? "labels: accept" : "") << "}" << std::endl;

                }
                tck << std::endl << std::endl;

                for (const auto& [k, v] : pos.locations()) {

                    for (const auto& e : pos.edges_from(k)) {

                        tck << "edge:TA:ell_" << e.from() << ":ell_" << e.to() << ":a{";

                        std::string provided_str;
                        for (const auto& g : e.guard()) {

                            if (!provided_str.empty()) {
                                provided_str += " && ";
                            }

                            if (g._i == 0) {

                                assert(g._j != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(g._j - 1) + " > " + std::to_string(-1 * g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._j - 1) + " >= " + std::to_string(-1 * g._bound.get_bound());
                                }

                            } else if (g._j == 0) {

                                assert(g._i != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(g._i - 1) + " < " + std::to_string(g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._i - 1) + " <= " + std::to_string(g._bound.get_bound());
                                }

                            } else {
                                assert(("Currently support only non-diagonal guards", false));
                            }

                        }

                        std::string do_str;
                        for (const auto& r : e.reset()) {
                    
                            if (!do_str.empty()) {
                                do_str += "; ";
                            }

                            do_str += "x_" + std::to_string(r - 1) + " = 0";

                        }

                        tck << (provided_str.empty() ? std::string{} : "provided: ") << provided_str;
                        tck << (provided_str.empty() || do_str.empty() ? std::string{} : " : ");
                        tck << (do_str.empty() ? std::string{} : "do: ") << do_str;
                        tck << "}" << std::endl;
                    }

                }
                tck << std::endl << std::endl;

                tck << "\n# " << "Please use the following command to check satisfiability:\n\n";
                std::cout << "\nPlease use the following command to check satisfiability:\n\n";

                if (out_fin) {
                    tck << "# " << "tck-reach -a covreach -l accept " << out_file << std::endl;
                    std::cout << "tck-reach -a covreach -l accept " << out_file << std::endl;

                } else {
                    tck << "# " << "tck-liveness -a couvscc -l accept " << out_file << std::endl;
                    std::cout << "tck-liveness -a couvscc -l accept " << out_file << std::endl;

                }

                spec_out << tck.str();

            } else {

                spec_out << out_str;

            }

        } else {        // xml

            if (out_flatten) {

                std::stringstream xml;

                xml << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
                xml << "<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.1//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_2.dtd'>" << std::endl;

                xml << "<nta>" << std::endl << std::endl;

                xml << "\t<declaration>" << std::endl;


                for (auto i = 1; i < pos.number_of_clocks() - 1; ++i) {

                    // In MoniTAal there is always x0 (meant to be 0 at all times)
                    // And there is an extra "global" clock for monitoring in newer versions MoniTAal
                    // So if number_of_clocks() returns 5, only 3 clocks are useful

                    xml << "\t\tclock x_" << i - 1 << ";" << std::endl;

                }


                xml << "\t</declaration>" << std::endl << std::endl;


                xml << "\t<template>" << std::endl;
                xml << "\t\t<name>TA</name>" << std::endl;

                xml << "\t\t<declaration>" << std::endl;


                monitaal::location_id_t largest_loc = 0;
                std::set<size_t> acc_set;
                monitaal::location_id_t last_acc_loc;

                for (const auto& [k, v] : pos.locations()) {
                    if (k > largest_loc) {
                        largest_loc = k;
                    }
                    if (v.is_accept()) {
                        acc_set.insert(k);
                        last_acc_loc = k;
                    }
                }

                xml << "\t\t\tint[0, " << largest_loc << "] loc = " << pos.initial_location() << ";" << std::endl;

                xml << "\t\t\tint[0, 1] acc = 0;" << std::endl;


                xml << "\t\t</declaration>" << std::endl;

                xml << "\t\t<location id=\"id0\" x=\"0\" y=\"0\">" << std::endl;
                xml << "\t\t</location>" << std::endl;
                xml << "\t\t<init ref=\"id0\"/>" << std::endl;

                for (const auto& [k, v] : pos.locations()) {

                    for (const auto& e : pos.edges_from(k)) {

                        xml << "\t\t<transition>" << std::endl;
                        xml << "\t\t\t<source ref=\"id0\"/>" << std::endl;
                        xml << "\t\t\t<target ref=\"id0\"/>" << std::endl;

                        std::string provided_str;
                        provided_str += "loc == " + std::to_string(e.from());
                        for (const auto& g : e.guard()) {

                            if (!provided_str.empty()) {
                                provided_str += " &amp;&amp; ";
                            }

                            if (g._i == 0) {

                                assert(g._j != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(g._j - 1) + " &gt; " + std::to_string(-1 * g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._j - 1) + " &gt;= " + std::to_string(-1 * g._bound.get_bound());
                                }

                            } else if (g._j == 0) {

                                assert(g._i != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(g._i - 1) + " &lt; " + std::to_string(g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._i - 1) + " &lt;= " + std::to_string(g._bound.get_bound());
                                }

                            } else {
                                assert(("Currently support only non-diagonal guards", false));
                            }

                        }

                        xml << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" << provided_str << "</label>" << std::endl;

                        std::string do_str;
                        do_str += "loc = " + std::to_string(e.to());
                        do_str += ", acc = " + (acc_set.count(e.to()) ? 1 : 0);
                        for (const auto& r : e.reset()) {
                    
                            if (!do_str.empty()) {
                                do_str += ", ";
                            }

                            do_str += "x_" + std::to_string(r - 1) + " = 0";

                        }

                        xml << "\t\t\t<label kind=\"assignment\" x=\"-246\" y=\"-34\">" << do_str << "</label>" << std::endl;

                        xml << "\t\t\t<nail x=\"-102\" y=\"34\"/>" << std::endl;
                        xml << "\t\t\t<nail x=\"-102\" y=\"-34\"/>" << std::endl;
                        xml << "\t\t</transition>" << std::endl;
                    }

                }

                xml << "\t</template>" << std::endl << std::endl;

	            xml << "\t<system>system TA;" << std::endl;
	            xml << "\t</system>" << std::endl << std::endl;
                xml << "</nta>" << std::endl;

                xml << std::endl << std::endl;

                spec_out << xml.str();


                std::cout << "\nPlease use the following command to check satisfiability:\n\n";

                if (out_fin) {

                    std::cout << "verifyta " << out_file << " " << (std::string(out_file) + ".q") << std::endl << std::endl;

                    std::ofstream query_out(std::string(out_file) + ".q", std::ios::trunc);
                    if (!query_out) {

                        std::cerr << "Error: Could not open " << (std::string(out_file) + ".q") << std::endl;
                        return 1;

                    }

                    query_out << "E<>(";

                    for (auto it = pos.locations().begin(); it != pos.locations().end(); ++it) {

                        if (it->second.is_accept()) {
                            query_out << "TA.loc == " << it->first;
                            if (it->first != last_acc_loc) {
                                query_out << " || ";
                            }
                        }

                    }

                    query_out << ")";

                    query_out.close();

                } else {

                    std::cout << "opaal_ltsmin " << out_file << " --ltl=" << (std::string(out_file) + ".ltl") << " -t 1" << std::endl;

                    std::ofstream ltl_out(std::string(out_file) + ".ltl", std::ios::trunc);
                    if (!ltl_out) {

                        std::cerr << "Error: Could not open " << (std::string(out_file) + ".ltl") << std::endl;
                        return 1;

                    }

                    ltl_out << "!([]<>(";


                    ltl_out << "TA_acc == 1";

                    /*
                    for (auto it = pos.locations().begin(); it != pos.locations().end(); ++it) {

                        if (it->second.is_accept()) {
                            ltl_out << "TA_loc == " << it->first;
                            if (it->first != last_acc_loc) {
                                ltl_out << " || ";
                            }
                        }

                    }
                    */

                    ltl_out << "))";

                    ltl_out.close();

                }

            } else {        // !out_flatten

	            out_str += "\t<system>system TA_0, TA_div, ";

                for (const auto& a : temporal_components) {

                    out_str += a.name().substr(0, a.name().find_first_of(' ')) + ", ";

                }

                out_str += "M;\n";
	            out_str += "\t</system>\n\n";
                out_str += "</nta>\n";

                out_str += "\n\n";

                spec_out << out_str;


                std::cout << "\nPlease use the following command to check satisfiability:\n\n";


                if (out_fin) {

                    std::cout << "verifyta " << out_file << " " << (std::string(out_file) + ".q") << std::endl << std::endl;

                    std::ofstream query_out(std::string(out_file) + ".q", std::ios::trunc);
                    if (!query_out) {

                        std::cerr << "Error: Could not open " << (std::string(out_file) + ".q") << std::endl;
                        return 1;

                    }

                    query_out << "E<>(";

                    query_out << "turn == 0 && "; 

                    for (const auto& a : temporal_components) {

                        monitaal::location_id_t last_acc_loc;
                        bool all_acc = true;
                        for (const auto& [k, v] : a.locations()) {
                            if (v.is_accept()) {
                                last_acc_loc = k;
                            } else {
                                all_acc = false;
                            }
                        }

                        if (!all_acc) {

                            query_out << "(";
                            for (auto it = a.locations().begin(); it != a.locations().end(); ++it) {

                                if (it->second.is_accept()) {
                                    query_out << a.name().substr(0, a.name().find_first_of(' ')) << ".loc == " << it->first;
                                    if (it->first != last_acc_loc) {
                                        query_out << " || ";
                                    }
                                }

                            }
                            query_out << ")";

                            query_out << " && ";

                        }

                    }

                    query_out << "TA_0.loc == 1";

                    // query_out << " && ";

                    // query_out << "M.loc == 0";

                    query_out << ")";

                    query_out.close();

                } else {

                    std::cout << "opaal_ltsmin " << out_file << " --ltl=" << (std::string(out_file) + ".ltl") << " -t 1" << std::endl;

                    std::ofstream ltl_out(std::string(out_file) + ".ltl", std::ios::trunc);
                    if (!ltl_out) {

                        std::cerr << "Error: Could not open " << (std::string(out_file) + ".ltl") << std::endl;
                        return 1;

                    }

                    ltl_out << "!(";

                    for (const auto& a : temporal_components) {

                        monitaal::location_id_t last_acc_loc;
                        bool all_acc = true;
                        for (const auto& [k, v] : a.locations()) {
                            if (v.is_accept()) {
                                last_acc_loc = k;
                            } else {
                                all_acc = false;
                            }
                        }

                        if (!all_acc) {

                            ltl_out << "([]<>(";

                            if (a.name().find(' ') != std::string::npos) {

                                ltl_out << a.name().substr(0, a.name().find_first_of(' ')) << "_acc == 1";

                            } else {

                                for (auto it = a.locations().begin(); it != a.locations().end(); ++it) {

                                    if (it->second.is_accept()) {
                                        ltl_out << a.name().substr(0, a.name().find_first_of(' ')) << "_loc == " << it->first;
                                        if (it->first != last_acc_loc) {
                                            ltl_out << " || ";
                                        }
                                    }

                                }

                            }

                            ltl_out << "))";

                            ltl_out << " && ";

                        }

                    }

                    ltl_out << "([]<>(TA_div_loc == 0))";

                    // ltl_out << " && ";

                    // ltl_out << "([]<>(M_loc == 0))";

                    ltl_out << ")";

                    ltl_out.close();

                }

            }

        }

        spec_out.close();

    }

    bdd_done();
    spec_in.close();

    return 0;

}
