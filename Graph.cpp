#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdlib>
using namespace std;

// WHAT: undirected weighted graph of city locations; roads are edges with km weights
// WHY HERE: delivery distance must be the shortest ROAD distance (Dijkstra),
//           not a straight line — a far direct road can lose to a short two-hop route
// COMPLEXITY: Dijkstra O((V+E) log V) with our hand-written min-heap
// VIVA ONE-LINER: "We keep the closest unvisited node in a min-heap, so each road is relaxed once and the first time we reach a node its distance is final."

const int LOCATION_ID_BASE = 501;  // location IDs: 501, 502, ...

struct Edge { int to; double km; };

class Graph {
    vector<string> names;          // index = node index (ID = 501 + index)
    vector<int> xs, ys;            // 0-100 layout coords for the SVG map (Phase 7)
    vector<vector<Edge>> adj;      // adjacency list, undirected
    HashTable nameToIndex;         // lowercase name -> node index
    int idxOf(const string& name) const {
        int i; return nameToIndex.get(toLower(name), i) ? i : -1;
    }
public:
    int addLocation(const string& name, int x, int y) {  // returns ID, or -1 if duplicate
        if (nameToIndex.contains(toLower(name))) return -1;
        int idx = (int)names.size();
        names.push_back(name); xs.push_back(x); ys.push_back(y);
        adj.push_back({});
        nameToIndex.put(toLower(name), idx);
        return LOCATION_ID_BASE + idx;
    }
    bool addRoad(const string& a, const string& b, double km) {
        int ia = idxOf(a), ib = idxOf(b);
        if (ia < 0 || ib < 0 || ia == ib || km <= 0) return false;
        adj[ia].push_back({ib, km});
        adj[ib].push_back({ia, km});
        return true;
    }
    int findLocation(const string& name) const {  // ID or -1 (case-insensitive)
        int i = idxOf(name);
        return i < 0 ? -1 : LOCATION_ID_BASE + i;
    }
    string canonicalName(const string& name) const {  // stored casing, "" if unknown
        int i = idxOf(name);
        return i < 0 ? "" : names[i];
    }
    string locationName(int id) const {
        int i = id - LOCATION_ID_BASE;
        return (i < 0 || i >= (int)names.size()) ? "" : names[i];
    }
    int locationCount() const { return (int)names.size(); }
    vector<string> getLocationNames() const { return names; }
    int getX(int id) const { int i = id - LOCATION_ID_BASE; return (i < 0 || i >= (int)xs.size()) ? 0 : xs[i]; }
    int getY(int id) const { int i = id - LOCATION_ID_BASE; return (i < 0 || i >= (int)ys.size()) ? 0 : ys[i]; }
    vector<Edge> edgesFrom(int id) const {
        int i = id - LOCATION_ID_BASE;
        return (i < 0 || i >= (int)adj.size()) ? vector<Edge>() : adj[i];
    }

    // Dijkstra from srcID; returns km to every node (-1 = unreachable)
    vector<double> distancesFrom(int srcID) const {
        int n = (int)names.size(), s = srcID - LOCATION_ID_BASE;
        vector<double> dist(n, -1.0);
        if (s < 0 || s >= n) return dist;
        const double INF = 1e18;
        vector<double> d(n, INF);
        MinHeap pq;
        d[s] = 0; pq.push(0.0, s);
        while (!pq.empty()) {
            auto [du, u] = pq.top(); pq.pop();
            if (du > d[u] + 1e-9) continue;  // stale entry: a better distance was already found
            for (auto& e : adj[u]) {
                double nd = du + e.km;
                if (nd < d[e.to] - 1e-9) { d[e.to] = nd; pq.push(nd, e.to); }
            }
        }
        for (int i = 0; i < n; i++) if (d[i] < INF/2) dist[i] = d[i];
        return dist;
    }

    // Shortest road distance in km; fills pathIDs (501-based); -1 if unreachable
    double shortestPath(int srcID, int dstID, vector<int>& pathIDs) const {
        int n = (int)names.size(), s = srcID - LOCATION_ID_BASE, t = dstID - LOCATION_ID_BASE;
        pathIDs.clear();
        if (s < 0 || s >= n || t < 0 || t >= n) return -1;
        const double INF = 1e18;
        vector<double> d(n, INF);
        vector<int> prev(n, -1);
        MinHeap pq;
        d[s] = 0; pq.push(0.0, s);
        while (!pq.empty()) {
            auto [du, u] = pq.top(); pq.pop();
            if (du > d[u] + 1e-9) continue;  // stale entry instead of decrease-key
            if (u == t) break;
            for (auto& e : adj[u]) {
                double nd = du + e.km;
                if (nd < d[e.to] - 1e-9) { d[e.to] = nd; prev[e.to] = u; pq.push(nd, e.to); }
            }
        }
        if (d[t] >= INF/2) return -1;
        for (int c = t; c != -1; c = prev[c]) pathIDs.push_back(LOCATION_ID_BASE + c);
        reverse(pathIDs.begin(), pathIDs.end());
        return d[t];
    }

    // Default city map: 10 locations, 14 roads, fully connected.
    // Note Riverside->Central Market: direct road is 10 km but via Old Town is
    // 4+3 = 7 km, so Dijkstra visibly beats the direct edge.
    void seedDefaultMap() {
        const char* L[][3] = {
            {"Central Market","50","50"}, {"Railway Station","20","70"},
            {"City Hospital","70","30"}, {"North Bus Stand","50","85"},
            {"University Campus","80","70"}, {"Old Town","25","35"},
            {"Riverside","10","50"}, {"Airport Road","90","45"},
            {"Sunrise Shelter","65","80"}, {"Green Park","40","20"}
        };
        for (auto& r : L) addLocation(r[0], atoi(r[1]), atoi(r[2]));
        const char* R[][3] = {
            {"Central Market","Old Town","4"}, {"Central Market","Riverside","10"},
            {"Old Town","Riverside","3"}, {"Central Market","Green Park","5"},
            {"Green Park","City Hospital","6"}, {"City Hospital","Airport Road","4"},
            {"Riverside","Railway Station","4"}, {"Railway Station","North Bus Stand","5"},
            {"North Bus Stand","University Campus","6"}, {"University Campus","Sunrise Shelter","4"},
            {"Sunrise Shelter","Airport Road","7"}, {"Central Market","North Bus Stand","8"},
            {"Old Town","Green Park","7"}, {"North Bus Stand","Sunrise Shelter","9"}
        };
        for (auto& r : R) addRoad(r[0], r[1], atof(r[2]));
    }
};

// CLI helper: numbered location list, reads a choice, returns the name.
inline string pickLocationCLI(const Graph& city, const string& prompt) {
    auto locs = city.getLocationNames();
    cout << prompt << "\n";
    for (size_t i = 0; i < locs.size(); i++) cout << "  " << (i + 1) << ". " << locs[i] << "\n";
    cout << "Choice [1]: ";
    string in; getline(cin, in);
    int ch = 1;
    try { ch = stoi(in); } catch (...) {}
    if (ch < 1 || ch > (int)locs.size()) ch = 1;
    return locs[(size_t)ch - 1];
}
