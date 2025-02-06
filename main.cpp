#include <iostream>
#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"

int main() {
    GrammarFactory factory;
    factory.Init();
    Grammar gr = factory.PickOne();
    LL1Parser ll1(gr);
    gr.Debug();
    std::cout << "Is ll1? : " << ll1.CreateLL1Table();

}