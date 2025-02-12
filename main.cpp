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
    bool out_fin = false;

} // namespace mightypplcpp

int main(int argc, const char ** argv) {



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

                    out_file = argv[3]; 

                    if (std::string_view(argv[4]) == "--tck") {

                        out_format = true;

                    } else if (std::string_view(argv[4]) == "--xml") {

                        out_format = false;

                    } else {

                        throw std::invalid_argument("Wrong output format specified"); 

                    }

                    if (argc >= 6) {

                        if (std::string_view(argv[5]) == "--noflatten") {

                            out_flatten = false;

                        } else {

                            throw std::invalid_argument("Wrong output type specified"); 

                        }

                    }

                } else {

                    throw std::invalid_argument("No output format specified"); 

                }

            }
                    
        }

    } catch (const std::invalid_argument& e) {

        std::cerr << e.what() << std::endl;
        std::cerr << "Usage: demo <in_spec_file> --{fin|inf} [out_file --{tck|xml} [--noflatten]]" << std::endl;
        std::cerr << "If [out_file ...] unspecified, the built-in fixpoint algorithm based on DBMs\n"
                  << "(PARDIBAAL) checks the (Buechi) emptiness of a flattened automaton (i.e. the\n"
                  << "satisfiability of the input formula && TA_div && model M)." << std::endl;
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

   
    std::cout << "Constructing TA (with BDD transitions) took = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    // std::cout << pos << std::endl;


    if (!out_format.has_value()) {

        std::chrono::steady_clock::time_point begin2 = std::chrono::steady_clock::now();
        std::cout << "<<<<<< Calculating fixpoint >>>>>>" << std::endl;

        auto initial_state = monitaal::symbolic_state_t(pos.initial_location(), monitaal::Federation::zero(pos.number_of_clocks()));

        if (out_fin) {

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

    } else {

        std::ofstream spec_out(out_file, std::ios::trunc);
        if (!spec_out) {

            std::cerr << "Error: Could not open out_file" << std::endl;
            return 1;

        }

        if (out_format.value()) {

            if (out_flatten) {

                std::stringstream tck;

                tck << "# File generated by MightyPPL" << std::endl;
                tck << "system:model_and_spec" << std::endl << std::endl << std::endl;

                tck << "event:a" << std::endl << std::endl << std::endl;

                for (auto i = 1; i < pos.number_of_clocks() - 1; ++i) {

                    // In MoniTAal there is always x0 (meant to be 0 at all times)
                    // And there is an extra "global" clock for monitoring in newer versions MoniTAal
                    // So if number_of_clocks() returns 5, only 3 clocks are useful for tck

                    tck << "clock:1:x_" << i << std::endl;

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
                                    provided_str += "x_" + std::to_string(g._j) + " > " + std::to_string(-1 * g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._j) + " >= " + std::to_string(-1 * g._bound.get_bound());
                                }

                            } else if (g._j == 0) {

                                assert(g._i != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(g._i) + " < " + std::to_string(g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._i) + " <= " + std::to_string(g._bound.get_bound());
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

                            do_str += "x_" + std::to_string(r) + " = 0";

                        }

                        tck << (provided_str.empty() ? std::string{} : "provided: ") << provided_str;
                        tck << (provided_str.empty() || do_str.empty() ? std::string{} : " : ");
                        tck << (do_str.empty() ? std::string{} : "do: ") << do_str;
                        tck << "}" << std::endl;
                    }

                }
                tck << std::endl << std::endl;

                spec_out << tck.str();

                std::cout << "\nPlease use the following command to check satisfiability:\n\n";

                if (out_fin) {
                    std::cout << "tck-reach -a covreach -l accept " << out_file << std::endl;
                } else {
                    std::cout << "tck-liveness -a couvscc -l accept " << out_file << std::endl;
                }

            } else {

                spec_out << out_str;

            }

        } else {

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

                    xml << "\t\tclock x_" << i << ";" << std::endl;

                }


                xml << "\t</declaration>" << std::endl << std::endl;


                xml << "\t<template>" << std::endl;
                xml << "\t\t<name>TA</name>" << std::endl;

                xml << "\t\t<declaration>" << std::endl;


                monitaal::location_id_t largest_loc = 0;
                for (const auto& [k, v] : pos.locations()) {
                    if (k > largest_loc) {
                        largest_loc = k;
                    }
                }

                xml << "\t\t\tint[0, " << largest_loc << "] loc = " << pos.initial_location() << ";" << std::endl;


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
                                    provided_str += "x_" + std::to_string(g._j) + " &gt; " + std::to_string(-1 * g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._j) + " &gt;= " + std::to_string(-1 * g._bound.get_bound());
                                }

                            } else if (g._j == 0) {

                                assert(g._i != 0);

                                if (g._bound.is_strict()) {
                                    provided_str += "x_" + std::to_string(g._i) + " &lt; " + std::to_string(g._bound.get_bound());
                                } else {
                                    provided_str += "x_" + std::to_string(g._i) + " &lt;= " + std::to_string(g._bound.get_bound());
                                }

                            } else {
                                assert(("Currently support only non-diagonal guards", false));
                            }

                        }

                        xml << "\t\t\t<label kind=\"guard\" x=\"-357\" y=\"-68\">" << provided_str << "</label>" << std::endl;

                        std::string do_str;
                        do_str += "loc = " + std::to_string(e.to());
                        for (const auto& r : e.reset()) {
                    
                            if (!do_str.empty()) {
                                do_str += ", ";
                            }

                            do_str += "x_" + std::to_string(r) + " = 0";

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

                monitaal::location_id_t last_acc_loc;
                for (const auto& [k, v] : pos.locations()) {
                    if (v.is_accept()) {
                        last_acc_loc = k;
                    }
                }

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

                    for (auto it = pos.locations().begin(); it != pos.locations().end(); ++it) {

                        if (it->second.is_accept()) {
                            ltl_out << "TA_loc == " << it->first;
                            if (it->first != last_acc_loc) {
                                ltl_out << " || ";
                            }
                        }

                    }

                    ltl_out << "))";

                    ltl_out.close();

                }



            } else {

                assert(("UPPAAL XML output is only supported for flattened (monolithic) automata", false));

            }

        }

        spec_out.close();

    }

    bdd_done();
    spec_in.close();

    return 0;

}
