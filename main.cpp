#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"
#include <iostream>

void SanityChecks(GrammarFactory& factory, Grammar& gr) {
    std::cout << "Sanity check (Is Infinite?) : " << factory.IsInfinite(gr)
              << "\n";
    std::cout << "Sanity check (Has Unreachable Symbols?) : "
              << factory.HasUnreachableSymbols(gr) << "\n";
    std::cout << "Sanity check (Has Direct Left Recursion?) : "
              << factory.HasDirectLeftRecursion(gr) << "\n";
}

Grammar GenLL1Grammar(int level) {
    GrammarFactory factory;
    factory.Init();
    Grammar   gr = factory.PickOne(level);
    LL1Parser ll1(gr);
    while (factory.IsInfinite(gr) || factory.HasUnreachableSymbols(gr) ||
           factory.HasDirectLeftRecursion(gr) || !ll1.CreateLL1Table()) {
        factory.RemoveLeftRecursion(gr);
        ll1 = LL1Parser(gr);
        if (ll1.CreateLL1Table()) {
            break;
        }
        gr = factory.PickOne(level);
    }
    return gr;
}

Grammar GenSLR1Grammar(int level) {
    GrammarFactory factory;
    factory.Init();
    Grammar    gr = factory.PickOne(level);
    SLR1Parser slr1(gr);

    while (factory.IsInfinite(gr) || factory.HasUnreachableSymbols(gr) ||
           !slr1.MakeParser()) {
        gr   = factory.PickOne(level);
        slr1 = SLR1Parser(gr);
    }
    return gr;
}

/*int main() {
    GrammarFactory factory;
    factory.Init();
    Grammar    gr = factory.PickOne(3);
    LL1Parser  ll1(gr);
    SLR1Parser slr1(gr);
    gr.Debug();
    std::cout << "Is ll1? : " << ll1.CreateLL1Table() << "\n";
    std::cout << "Is slr1? : " << slr1.MakeParser() << "\n";
    SanityChecks(factory, gr);
    if (factory.HasDirectLeftRecursion(gr)) {
        std::cout << "-------------------------------------------\n";
        std::cout << "Direct left recursion elimination:\n";
        factory.RemoveLeftRecursion(gr);
        gr.Debug();
        ll1  = LL1Parser(gr);
        slr1 = SLR1Parser(gr);
        std::cout << "Is ll1? : " << ll1.CreateLL1Table() << "\n";
        std::cout << "Is slr1? : " << slr1.MakeParser() << "\n";
        SanityChecks(factory, gr);
    }
}*/

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

    if (level < 1 || level > 3) {
        std::cerr << "Error: Difficulty level must be 1, 2, or 3." << std::endl;
        return 1;
    }

    Grammar gr;
    if (analysis_type == "ll") {
        gr = GenLL1Grammar(level);
        LL1Parser ll1(gr);
        gr.Debug();
        std::cout << "Is ll1? : " << ll1.CreateLL1Table() << "\n";
    } else if (analysis_type == "slr") {
        gr = GenSLR1Grammar(level);
        SLR1Parser slr1(gr);
        gr.Debug();
        std::cout << "Is slr1? : " << slr1.MakeParser() << "\n";
    } else {
        std::cerr << "Error: Invalid analysis type. Use 'll' or 'slr'."
                  << std::endl;
        return 1;
    }
    return 0;
}