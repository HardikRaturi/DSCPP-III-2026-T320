#pragma once
#include <iostream>
#include <vector>
using namespace std;

// DSA: FIFO Queue using singly linked list
struct QNode {
    int data;
    QNode* next;
    QNode(int v) : data(v), next(nullptr) {}
};

class IntQueue {
    QNode* front;
    QNode* back;
    int sz;
public:
    IntQueue() : front(nullptr), back(nullptr), sz(0) {}
    ~IntQueue() { clear(); }

    void enqueue(int v) {
        QNode* n = new QNode(v);
        if (!back) { front = back = n; }
        else { back->next = n; back = n; }
        sz++;
    }

    int dequeue() {
        if (!front) return -1;
        int v = front->data;
        QNode* t = front;
        front = front->next;
        if (!front) back = nullptr;
        delete t;
        sz--;
        return v;
    }

    int peek() const { return front ? front->data : -1; }
    bool isEmpty() const { return sz == 0; }
    int size() const { return sz; }

    bool contains(int v) const {
        QNode* c = front;
        while (c) { if (c->data == v) return true; c = c->next; }
        return false;
    }

    bool removeValue(int v) {
        QNode* c = front, *p = nullptr;
        while (c) {
            if (c->data == v) {
                if (p) p->next = c->next; else front = c->next;
                if (c == back) back = p;
                delete c; sz--; return true;
            }
            p = c; c = c->next;
        }
        return false;
    }

    vector<int> toVector() const {
        vector<int> r;
        QNode* c = front;
        while (c) { r.push_back(c->data); c = c->next; }
        return r;
    }

    void clear() {
        while (front) { QNode* t = front; front = front->next; delete t; }
        back = nullptr; sz = 0;
    }
};
