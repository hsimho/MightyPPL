#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iomanip>

#include "MightyPPL.h"

using namespace mightypplcpp;
using namespace antlr4;


int main(int argc, const char ** argv) {

    const char* spec_file;
    const char* tck_file;

    if (argc >= 3) {

        spec_file = argv[1];
        tck_file = argv[2];

    } else {

        std::cerr << "Usage: demo <in_spec_file> <out_tck_file>" << std::endl;
        return 1;

    }

    std::ifstream spec_in(spec_file, std::ios::in);
    if (!spec_in) {

        std::cerr << "Error: Could not open in_spec_file" << std::endl;
        return 1;

    }

    std::ofstream tck_out(tck_file, std::ios::trunc);
    if (!tck_out) {

        std::cerr << "Error: Could not open out_tck_file" << std::endl;
        return 1;

    }


    bdd_init(1000, 100);

    std::cout << "\n<<<<<< Pre-processing input formula... >>>>>>\n\n";

    std::cout << "\nInput formula (as read from input):\n";

    std::stringstream buf;
    buf << spec_in.rdbuf();
    std::cout << buf.str();

    std::string nnf_in = buf.str();

    ANTLRInputStream input(nnf_in);
    MitlLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    MitlParser parser(&tokens);

    MitlParser::MainContext* original_formula = parser.main();

    tck_out << build_ta_from_main(original_formula);

    /*
    nnf_in = "!(" + nnf_in + ")";

    input.load(nnf_in);
    lexer.setInputStream(&input);
    tokens.setTokenSource(&lexer);
    parser.setTokenStream(&tokens);
    original_formula = parser.main();

    monitaal::TA neg = build_ta_from_main(original_formula);
    */

    /*
    std::cout << "\npos:\n" << std::endl;
    std::cout << pos;

    std::cout << "\nneg:\n" << std::endl;
    std::cout << neg;
    */

    // std::cout << std::endl;

    // std::cout << std::setw(20) << "# of locations: " << std::setw(10) << projected.locations().size() << std::setw(0) << std::endl;
    // std::cout << std::setw(20) << "# of clocks: " << std::setw(10) << projected.locations().size() << std::setw(0) << std::endl;

    // std::cout << std::endl;

    // for (const auto & [k, v] : projected.locations()) {

    //     std::cout << std::setw(12) << "location: " << std::setw(10) << v.id() << " (" << v.name() << ")" << (v.is_accept() ? "(acc)" : "") << std::setw(0) << std::endl;
    //     std::cout << std::setw(20) << "# outgoing: " << std::setw(10) << projected.edges_from(k).size() << std::setw(0) << std::endl;
    //     std::cout << std::setw(20) << "# incoming: " << std::setw(10) << projected.edges_to(k).size() << std::setw(0) << std::endl;

    // }

    // std::cout << projected;



    bdd_done();

    spec_in.close();
    tck_out.close();

    return 0;

}
