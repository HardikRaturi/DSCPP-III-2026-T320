#pragma once
#include <string>
#include <vector>
using namespace std;

// WHAT: string -> int map with separate chaining
// WHY HERE: O(1) username lookup in Auth, and location-name -> node-index in Graph
// COMPLEXITY: put/get/contains/remove average O(1); worst O(n) if everything collides
// VIVA ONE-LINER: "Each bucket holds a small chain, so collisions stay correct and the average lookup is still constant time."

const int HASH_BUCKETS = 101;  // prime bucket count; plenty for hundreds of keys

class HashTable {
    vector<vector<pair<string,int>>> buckets;
    int n;
    static unsigned djb2(const string& s) {  // classic string hash: h = h*33 + c
        unsigned h = 5381;
        for (unsigned char c : s) h = h * 33u + c;
        return h;
    }
public:
    HashTable() : buckets(HASH_BUCKETS), n(0) {}
    void put(const string& key, int value) {
        auto& b = buckets[djb2(key) % HASH_BUCKETS];
        for (auto& kv : b) if (kv.first == key) { kv.second = value; return; }
        b.push_back({key, value}); n++;
    }
    bool get(const string& key, int& out) const {
        const auto& b = buckets[djb2(key) % HASH_BUCKETS];
        for (const auto& kv : b) if (kv.first == key) { out = kv.second; return true; }
        return false;
    }
    bool contains(const string& key) const { int x; return get(key, x); }
    bool remove(const string& key) {
        auto& b = buckets[djb2(key) % HASH_BUCKETS];
        for (size_t i = 0; i < b.size(); i++)
            if (b[i].first == key) { b.erase(b.begin() + (long)i); n--; return true; }
        return false;
    }
    int size() const { return n; }
};
