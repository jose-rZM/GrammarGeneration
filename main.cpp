#include <iostream>
#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"

int main() {
    GrammarFactory factory;
    factory.Init();
    Grammar gr = factory.PickOne(2);
    LL1Parser ll1(gr);
    SLR1Parser slr1(gr);
    gr.Debug();
    std::cout << "Is ll1? : " << ll1.CreateLL1Table() << "\n";
    std::cout << "Is slr1? : " << slr1.MakeParser() << "\n";
}