#include "grammar_factory.hpp"
#include <random>

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

Grammar GrammarFactory::PickOne()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return items.at(dist(gen));
}
