#include "grammar.hpp"
#include "grammar_factory.hpp"
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
