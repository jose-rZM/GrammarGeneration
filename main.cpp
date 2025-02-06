#include <iostream>
#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"

int main() {
    GrammarFactory factory;
    factory.Init();
    Grammar gr = factory.PickOne();
    LL1Parser ll1(gr);
    SLR1Parser slr1(gr);
    gr.Debug();
    std::cout << "Is ll1? : " << ll1.CreateLL1Table() << "\n";
    std::cout << "Is slr1? : \n" << slr1.MakeParser() << "\n";
    slr1.DebugStates();
    slr1.DebugTable();
    slr1.DebugActions();

}