#pragma once
#include <vector>
#include <utility>
using namespace std;

// WHAT: binary min-heap of (key, id) pairs
// WHY HERE: Dijkstra's algorithm repeatedly needs the closest unvisited node
// COMPLEXITY: push/pop O(log n), top O(1)
// VIVA ONE-LINER: "The smallest key sits at the root, so finding the minimum is instant and restoring order after a pop costs O(log n)."

class MinHeap {
    vector<pair<double,int>> h;  // (key, id)
    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (h[p].first <= h[i].first) break;
            swap(h[p], h[i]); i = p;
        }
    }
    void siftDown(int i) {
        int n = (int)h.size();
        while (true) {
            int l = 2 * i + 1, r = 2 * i + 2, m = i;
            if (l < n && h[l].first < h[m].first) m = l;
            if (r < n && h[r].first < h[m].first) m = r;
            if (m == i) break;
            swap(h[m], h[i]); i = m;
        }
    }
public:
    void push(double key, int id) { h.push_back({key, id}); siftUp((int)h.size() - 1); }
    pair<double,int> top() const { return h.front(); }
    void pop() {
        h[0] = h.back(); h.pop_back();
        if (!h.empty()) siftDown(0);
    }
    bool empty() const { return h.empty(); }
    int size() const { return (int)h.size(); }
};
