#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"
#include <iostream>

int main() {
    GrammarFactory factory;
    factory.Init();
    Grammar    gr = factory.PickOne(3);
    LL1Parser  ll1(gr);
    SLR1Parser slr1(gr);
    gr.Debug();
    std::cout << "Is ll1? : " << ll1.CreateLL1Table() << "\n";
    std::cout << "Is slr1? : " << slr1.MakeParser() << "\n";
    std::cout << "Sanity check (Is Infinite?) : " << factory.IsInfinite(gr)
              << "\n";
    std::cout << "Sanity check (Has Unreachable Symbols?) : "
              << factory.HasUnreachableSymbols(gr) << "\n";
    std::cout << "Sanity check (Has Direct Left Recursion?) : "
              << factory.HasDirectLeftRecursion(gr) << "\n";
}