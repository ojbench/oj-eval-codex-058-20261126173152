#ifndef POKEDEX_SRC_HPP
#define POKEDEX_SRC_HPP

#include <cstring>
#include <fstream>
#include <map>
#include <string>

class BasicException {
protected:
    std::string message;
public:
    explicit BasicException(const char *text) : message(text) {}
    explicit BasicException(const std::string &text) : message(text) {}
    virtual ~BasicException() = default;
    virtual const char *what() const { return message.c_str(); }
};

class ArgumentException : public BasicException {
public:
    using BasicException::BasicException;
};

class IteratorException : public BasicException {
public:
    using BasicException::BasicException;
};

struct Pokemon {
    char name[12] = {};
    int id = 0;
    unsigned types = 0;
};

class Pokedex {
    std::string filename;
    std::map<int, Pokemon> records;

    static void checkId(int id) {
        if (id <= 0)
            throw ArgumentException("Argument Error: PM ID Invalid (" +
                                    std::to_string(id) + ")");
    }

    static int typeIndex(const std::string &type) {
        static const char *const names[] = {
            "fire", "water", "grass", "electric", "flying", "ground", "dragon"
        };
        for (int i = 0; i < 7; ++i)
            if (type == names[i]) return i;
        throw ArgumentException("Argument Error: PM Type Invalid (" + type + ")");
    }

    static unsigned parseTypes(const char *text) {
        const std::string types(text);
        unsigned mask = 0;
        std::size_t start = 0;
        while (true) {
            const std::size_t end = types.find('#', start);
            mask |= 1u << typeIndex(types.substr(start, end - start));
            if (end == std::string::npos) return mask;
            start = end + 1;
        }
    }

    static float damage(int type, unsigned mask) {
        static const float chart[7][7] = {
            {0.5f, 0.5f, 2, 1, 1, 1, 0.5f},
            {2, 0.5f, 0.5f, 1, 1, 2, 0.5f},
            {0.5f, 2, 0.5f, 1, 0.5f, 2, 0.5f},
            {1, 2, 0.5f, 1, 2, 0, 0.5f},
            {1, 1, 2, 0.5f, 1, 1, 1},
            {2, 1, 0.5f, 2, 0, 1, 1},
            {1, 1, 1, 1, 1, 1, 2}
        };
        float result = 1;
        for (int i = 0; i < 7; ++i)
            if (mask & (1u << i)) result *= chart[type][i];
        return result;
    }

public:
    explicit Pokedex(const char *fileName) : filename(fileName) {
        std::ifstream input(filename);
        Pokemon pokemon;
        std::string name;
        while (input >> pokemon.id >> name >> pokemon.types) {
            if (name.size() >= sizeof(pokemon.name)) continue;
            std::strcpy(pokemon.name, name.c_str());
            records[pokemon.id] = pokemon;
        }
        if (!input.is_open()) {
            std::ofstream create(filename);
        }
    }

    ~Pokedex() {
        std::ofstream output(filename, std::ios::trunc);
        for (const auto &entry : records)
            output << entry.first << ' ' << entry.second.name << ' '
                   << entry.second.types << '\n';
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        for (const char *p = name; *p; ++p)
            if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')))
                throw ArgumentException(std::string("Argument Error: PM Name Invalid (") + name + ")");
        checkId(id);
        const unsigned mask = parseTypes(types);
        Pokemon pokemon;
        std::strcpy(pokemon.name, name);
        pokemon.id = id;
        pokemon.types = mask;
        return records.emplace(id, pokemon).second;
    }

    bool pokeDel(int id) {
        checkId(id);
        return records.erase(id) != 0;
    }

    std::string pokeFind(int id) const {
        checkId(id);
        const auto it = records.find(id);
        return it == records.end() ? "None" : it->second.name;
    }

    std::string typeFind(const char *types) const {
        const unsigned mask = parseTypes(types);
        int count = 0;
        std::string names;
        for (const auto &entry : records) {
            if ((entry.second.types & mask) == mask) {
                ++count;
                names += '\n';
                names += entry.second.name;
            }
        }
        return count ? std::to_string(count) + names : "None";
    }

    float attack(const char *type, int id) const {
        checkId(id);
        const int index = typeIndex(type);
        const auto it = records.find(id);
        return it == records.end() ? -1 : damage(index, it->second.types);
    }

    int catchTry() const {
        if (records.empty()) return 0;
        int counts[128] = {};
        for (const auto &entry : records) ++counts[entry.second.types];
        unsigned available = records.begin()->second.types;
        --counts[available];
        int result = 1;
        unsigned processed = 0;
        // Each newly acquired attack type only needs to be processed once.
        while (available != processed) {
            for (int type = 0; type < 7; ++type) {
                const unsigned bit = 1u << type;
                if (!(available & bit) || (processed & bit)) continue;
                processed |= bit;
                for (unsigned mask = 1; mask < 128; ++mask) {
                    if (counts[mask] && damage(type, mask) >= 2) {
                        result += counts[mask];
                        counts[mask] = 0;
                        available |= mask;
                    }
                }
            }
        }
        return result;
    }

    struct iterator {
    private:
        friend class Pokedex;
        Pokedex *owner = nullptr;
        std::map<int, Pokemon>::iterator position;
        iterator(Pokedex *p, std::map<int, Pokemon>::iterator it)
            : owner(p), position(it) {}
    public:
        iterator() = default;
        iterator &operator++() {
            if (!owner || position == owner->records.end())
                throw IteratorException("Iterator Error: Invalid Iterator");
            ++position;
            return *this;
        }
        iterator &operator--() {
            if (!owner || position == owner->records.begin())
                throw IteratorException("Iterator Error: Invalid Iterator");
            --position;
            return *this;
        }
        iterator operator++(int) { iterator old = *this; ++*this; return old; }
        iterator operator--(int) { iterator old = *this; --*this; return old; }
        iterator &operator=(const iterator &) = default;
        bool operator==(const iterator &rhs) const {
            return owner == rhs.owner && (!owner || position == rhs.position);
        }
        bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
        Pokemon &operator*() const {
            if (!owner || position == owner->records.end())
                throw IteratorException("Iterator Error: Dereference Error");
            return position->second;
        }
        Pokemon *operator->() const { return &operator*(); }
    };

    iterator begin() { return iterator(this, records.begin()); }
    iterator end() { return iterator(this, records.end()); }
};

#endif
