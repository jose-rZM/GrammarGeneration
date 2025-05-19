#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " [ll|slr] [1|2|3]" << std::endl;
        return 1;
    }

    std::string analysis_type = argv[1];
    int         level;

    try {
        level = std::stoi(argv[2]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid difficulty level. Please use 1, 2, or 3."
                  << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Difficulty level out of range." << std::endl;
        return 1;
    }

    if (level < 1 || level > 7) {
        std::cerr << "Error: Difficulty level must be between 1 and 7."
                  << std::endl;
        return 1;
    }

    GrammarFactory factory;
    factory.Init();
    Grammar gr;
    if (analysis_type == "ll") {
        gr = factory.GenLL1Grammar(level);
        LL1Parser ll1(gr);
        gr.Debug();
        std::cout << "Is ll1? : " << ll1.CreateLL1Table() << "\n";
        ll1.PrintTable();
    } else if (analysis_type == "slr") {
        gr = factory.GenSLR1Grammar(level);
        gr.TransformToAugmentedGrammar();
        SLR1Parser slr1(gr);
        gr.Debug();
        std::cout << "Is slr1? : " << slr1.MakeParser() << "\n";
        slr1.DebugStates();
        slr1.DebugActions();
    } else {
        std::cerr << "Error: Invalid analysis type. Use 'll' or 'slr'."
                  << std::endl;
        return 1;
    }
    return 0;
}