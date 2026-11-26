#include "../src.hpp"
#include <cassert>
#include <cstdio>
#include <random>
#include <vector>

template<class Exception, class Function>
void expectError(Function function, const std::string &message) {
    try { function(); assert(false); }
    catch (const Exception &error) { assert(error.what() == message); }
}

int main() {
    const char *path = "/tmp/pokedex-test.dat";
    std::remove(path);
    {
        Pokedex p(path);
        assert(p.begin() == p.end());
        assert(p.catchTry() == 0);
        p.pokeAdd("Water", 3, "water");
        p.pokeAdd("Fire", 1, "fire");
        p.pokeAdd("Grass", 2, "grass");
        assert(!p.pokeAdd("Other", 1, "dragon"));
        assert(p.catchTry() == 3);
        assert(p.typeFind("water") == "1\nWater");
        auto it = p.begin();
        assert((it++)->id == 1);
        assert(it->id == 2);
        assert((--p.end())->id == 3);
        expectError<IteratorException>([&] { --p.begin(); }, "Iterator Error: Invalid Iterator");
        expectError<IteratorException>([&] { ++p.end(); }, "Iterator Error: Invalid Iterator");
        expectError<IteratorException>([&] { *p.end(); }, "Iterator Error: Dereference Error");
        expectError<ArgumentException>([&] { p.pokeAdd("pm1", 1, "water"); }, "Argument Error: PM Name Invalid (pm1)");
        expectError<ArgumentException>([&] { p.typeFind("water#ice#block"); }, "Argument Error: PM Type Invalid (ice)");
        expectError<ArgumentException>([&] { p.attack("ice", 99); }, "Argument Error: PM Type Invalid (ice)");
        expectError<ArgumentException>([&] { p.pokeDel(0); }, "Argument Error: PM ID Invalid (0)");
        p.pokeAdd("Dual", 4, "water#dragon");
        assert(p.attack("fire", 4) == 0.25f);
        assert(p.pokeDel(2));
    }
    {
        Pokedex p(path);
        assert(p.pokeFind(1) == "Fire");
        assert(p.pokeFind(2) == "None");
        assert(p.typeFind("dragon#water") == "1\nDual");
    }
    std::remove(path);
    const char *names[] = {"fire", "water", "grass", "electric", "flying", "ground", "dragon"};
    std::mt19937 random(1277);
    for (int trial = 0; trial < 300; ++trial) {
        {
            Pokedex p(path);
            const int n = 1 + random() % 100;
            std::vector<unsigned> masks(n);
            for (int i = 0; i < n; ++i) {
                masks[i] = 1 + random() % 127;
                std::string types;
                for (int t = 0; t < 7; ++t) if (masks[i] & (1u << t)) {
                    if (!types.empty()) types += '#';
                    types += names[t];
                }
                std::string name = "P";
                name += char('A' + i / 26);
                name += char('a' + i % 26);
                p.pokeAdd(name.c_str(), i + 1, types.c_str());
            }
            std::vector<bool> caught(n, false);
            caught[0] = true;
            int count = 1;
            bool changed = true;
            while (changed) {
                changed = false;
                for (int i = 0; i < n; ++i) if (!caught[i]) {
                    bool canCatch = false;
                    for (int j = 0; j < n; ++j) if (caught[j])
                        for (int t = 0; t < 7; ++t)
                            if ((masks[j] & (1u << t)) && p.attack(names[t], i + 1) >= 2)
                                canCatch = true;
                    if (canCatch) { caught[i] = true; ++count; changed = true; }
                }
            }
            assert(p.catchTry() == count);
            assert(p.catchTry() == count);
        }
        std::remove(path);
    }
}
