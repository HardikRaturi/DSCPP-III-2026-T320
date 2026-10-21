#pragma once
#include <iostream>
#include <vector>
using namespace std;

// DSA: Binary Max-Heap Priority Queue for Requests
class RequestHeap {
    vector<Request> heap;
    int parent(int i) const { return (i - 1) / 2; }
    int left(int i) const { return 2 * i + 1; }
    int right(int i) const { return 2 * i + 2; }

    void heapifyUp(int i) {
        while (i > 0 && heap[parent(i)].getPriority() < heap[i].getPriority()) {
            swap(heap[parent(i)], heap[i]);
            i = parent(i);
        }
    }
    void heapifyDown(int i) {
        int n = (int)heap.size(), largest = i;
        if (left(i) < n && heap[left(i)].getPriority() > heap[largest].getPriority()) largest = left(i);
        if (right(i) < n && heap[right(i)].getPriority() > heap[largest].getPriority()) largest = right(i);
        if (largest != i) { swap(heap[i], heap[largest]); heapifyDown(largest); }
    }
public:
    void push(const Request& r) {
        heap.push_back(r);
        heapifyUp((int)heap.size() - 1);
    }
    Request top() const { return heap.front(); }
    void pop() {
        if (heap.empty()) return;
        heap[0] = heap.back(); heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
    }
    bool empty() const { return heap.empty(); }
    int size() const { return (int)heap.size(); }

    bool removeByID(int rid) {
        for (int i = 0; i < (int)heap.size(); ++i) {
            if (heap[i].getRequestID() == rid) {
                heap[i] = heap.back(); heap.pop_back();
                if (i < (int)heap.size()) { heapifyUp(i); heapifyDown(i); }
                return true;
            }
        }
        return false;
    }
    vector<Request> snapshot() const { return heap; }
};
