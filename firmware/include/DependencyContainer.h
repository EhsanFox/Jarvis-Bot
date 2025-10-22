#pragma once
#include <unordered_map>
#include <Arduino.h>

struct StringHash {
    std::size_t operator()(const String& s) const noexcept {
        return std::hash<std::string>()(s.c_str());
    }
};

struct StringEqual {
    bool operator()(const String& a, const String& b) const noexcept {
        return a == b;
    }
};

class DependencyContainer {
public:
    template<typename T>
    void set(const String& key, T* instance) {
        _map[key] = reinterpret_cast<void*>(instance);
    }

    template<typename T>
    T* get(const String& key) const {
        auto it = _map.find(key);
        if (it != _map.end())
            return reinterpret_cast<T*>(it->second);
        return nullptr;
    }

private:
    std::unordered_map<String, void*, StringHash, StringEqual> _map;
};
