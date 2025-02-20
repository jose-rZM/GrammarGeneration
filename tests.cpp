#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include <gtest/gtest.h>

TEST(GrammarTest, IsInfinite_WhenGrammarIsInfinite) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"a", "A"});

    bool result = factory.IsInfinite(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, IsInfinite_WhenGrammarIsNotInfinite) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    bool result = factory.IsInfinite(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest, HasUnreachableSymbols_WhenGrammarHasUnreachableSymbols) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"a", "b"});
    g.AddProduction("B", {"c"});

    bool result = factory.HasUnreachableSymbols(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, HasUnreachableSymbols_WhenGrammarHasNoUnreachableSymbols) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"a", "b", "B"});
    g.AddProduction("B", {"c"});

    bool result = factory.HasUnreachableSymbols(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest, HasLeftDirectRecursion_WhenGrammarHasLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"A", "a"});
    g.AddProduction("A", {"b"});

    bool result = factory.HasDirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, HasLeftDirectRecursion_WhenGrammarHasNoLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    bool result = factory.HasDirectLeftRecursion(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest, RemoveDirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});

    g.AddProduction("A", {"A", "a"});
    g.AddProduction("A", {"b"});

    Grammar original = g;
    factory.RemoveLeftRecursion(g);

    EXPECT_NE(original.g_, g.g_);
    EXPECT_FALSE(factory.HasDirectLeftRecursion(g));
    EXPECT_NE(g.g_.find("A'"), g.g_.end());
}

TEST(GrammarTest, RemoveDirectLeftRecursion_WhenThereIsNoLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});

    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    Grammar original = g;
    factory.RemoveLeftRecursion(g);

    EXPECT_EQ(original.g_, g.g_);
}

TEST(LL1__Test, FirstSet) {
    Grammar g;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();
    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"a", "b"};
    ll1.First({{"A", g.st_.EOL_}}, result);
    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithNullableSymbols) {
    Grammar g;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();
    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"a", g.st_.EPSILON_};
    ll1.First({{"A", g.st_.EOL_}}, result);
    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetMultipleSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"a", "b", g.st_.EPSILON_};
    ll1.First({{"A", "B"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetEndingWithNullable) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"a", g.st_.EPSILON_};
    ll1.First({{"A", g.st_.EOL_}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithAllSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"a", "B", "D"});
    g.AddProduction("A", {"C", "B"});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"d", "B", "c"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"a", "B"});
    g.AddProduction("D", {"d"});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"a", "b", "d"};
    ll1.First({{"A", "B", "D", "C", "D"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithOneSymbolAndEpsilon) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "$"});
    g.AddProduction("A", {"a", "B", "D"});
    g.AddProduction("A", {"C", "B"});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"d", "B", "c"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"a", "B"});
    g.AddProduction("D", {"d"});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"a", "b", "d", g.st_.EPSILON_};
    ll1.First({{"A"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithMultipleSymbols2) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E`"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"(", "n", "+", g.st_.EPSILON_};
    ll1.First({{"T", "E'"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithTerminalSymbolAtTheBeginning) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E`"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {"+"};
    ll1.First({{"+", "T", "E'"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithOnlyEpsilon) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E`"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {g.st_.EPSILON_};
    ll1.First({{g.st_.EPSILON_}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FollowTest1) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E`"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected {g.st_.EOL_, ")"};
    result = ll1.Follow("E");

    EXPECT_EQ(result, expected);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
