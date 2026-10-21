#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class RequestManager {
    vector<Request> allRequests;
    RequestHeap pendingHeap; // DSA: max-heap
    int nextRequestID;

    Request* findByID(int rid) {
        for (auto& r : allRequests)
            if (r.getRequestID() == rid) return &r;
        return nullptr;
    }
public:
    RequestManager() : nextRequestID(201) {}

    void createRequest(int recipientID) {
        cout << "\n=== Create Food Request ===\n";
        string ftype, unitIn, loc, qtyIn, prioIn;
        cout << "Food Type (Cooked/Raw/Packaged/Produce): "; getline(cin, ftype);
        cout << "Quantity : "; getline(cin, qtyIn);
        int qty = 0;
        try { qty = stoi(qtyIn); } catch (...) { cout << "Invalid quantity.\n"; return; }
        if (qty <= 0) { cout << "Quantity must be positive.\n"; return; }
        cout << "Unit (kg/servings/boxes): "; getline(cin, unitIn);
        cout << "Preferred Pickup Location: "; getline(cin, loc);
        cout << "Priority (1-5, 5 = most urgent): "; getline(cin, prioIn);
        int prio = 1;
        try { prio = stoi(prioIn); } catch (...) {}
        if (prio < 1 || prio > 5) prio = 1;

        Request r(nextRequestID++, recipientID, ftype, qty, unitIn, loc, prio);
        allRequests.push_back(r);
        pendingHeap.push(r);
        cout << "Request created! Request ID = " << r.getRequestID() << "\n";
    }

    void displayAll() const {
        if (allRequests.empty()) { cout << "  No requests.\n"; return; }
        for (auto& r : allRequests) { cout << "\n"; r.display(); }
    }

    void displayByRecipient(int recipientID) const {
        bool found = false;
        for (auto& r : allRequests) {
            if (r.getRecipientID() == recipientID) { cout << "\n"; r.display(); found = true; }
        }
        if (!found) cout << "  No requests for recipient ID " << recipientID << ".\n";
    }

    void displayPendingHeap() const {
        auto snap = pendingHeap.snapshot();
        if (snap.empty()) { cout << "  Pending queue is empty.\n"; return; }
        cout << "\n--- Pending Requests (Priority Heap) ---\n";
        for (auto& r : snap) { cout << "\n"; r.display(); }
    }

    Request* getPendingTop() {
        if (pendingHeap.empty()) return nullptr;
        int rid = pendingHeap.top().getRequestID();
        return findByID(rid);
    }

    void popPending() { pendingHeap.pop(); }

    bool setRequestStatus(int rid, const string& newStatus, int matchedFoodID = -1) {
        Request* r = findByID(rid);
        if (!r) return false;
        r->setStatus(newStatus);
        if (matchedFoodID != -1) r->setMatchedFoodID(matchedFoodID);
        if (newStatus != "Pending") pendingHeap.removeByID(rid);
        return true;
    }

    Request* findRequestByID(int rid) { return findByID(rid); }

    vector<int> getPendingHeapSnapshot() const {
        vector<int> ids;
        for (auto& r : pendingHeap.snapshot()) ids.push_back(r.getRequestID());
        return ids;
    }

    int pendingCount() const { return pendingHeap.size(); }
    int totalCount() const { return (int)allRequests.size(); }
};
