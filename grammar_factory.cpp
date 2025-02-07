#include "grammar_factory.hpp"
#include <random>
#include <algorithm>

void GrammarFactory::Init()
{
    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "b", "A"}, {"a"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "b", "A"}, {"a", "b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A", "b"}, {"EPSILON"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"A", "a"}, {"EPSILON"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A"}, {"EPSILON"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A", "c"}, {"b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"a", "A", "a"}, {"b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"A", "a"}, {"b"}}}});

    items.emplace_back(std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
        {"A", {{"b", "A"}, {"a"}}}});
}

Grammar GrammarFactory::PickOne(int level)
{
    switch (level)
    {
    case 1:
        return Lv1();
        break;
    case 2:
        return Lv2();
        break;
    case 3:
        return Lv3();
        break;
    default:
        return Grammar();
        break;
    }
}

Grammar GrammarFactory::Lv1()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

Grammar GrammarFactory::Lv2()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

Grammar GrammarFactory::Lv3()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

GrammarFactory::FactoryItem::FactoryItem(const std::unordered_map<std::string, std::vector<production>> &grammar)
{
    for (const auto &[nt, prods] : grammar)
    {
        st_.PutSymbol(nt, false);
        for (const auto &prod : prods)
        {
            for (const std::string &symbol : prod)
            {
                if (symbol == "EPSILON")
                {
                    continue;
                }
                else if (std::islower(symbol[0]))
                {
                    st_.PutSymbol(symbol, true);
                }
            }
        }
    }
    g_ = (grammar);
}

bool GrammarFactory::FactoryItem::HasEmptyProduction(const std::string &antecedent)
{
    auto &rules = g_.at(antecedent);
    return std::find_if(rules.cbegin(), rules.cend(), [&](const auto &rule)
                        { return rule[0] == "EPSILON"; }) != rules.cend();
}