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

int main() {
    GrammarFactory factory;
    factory.Init();
    Grammar    gr = factory.PickOne(2);
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
}