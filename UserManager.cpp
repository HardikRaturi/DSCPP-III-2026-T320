#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// DSA: Insertion Sort - keeps vectors sorted by ID for binary search
template<typename T>
void insertionSort(vector<T>& v) {
    for (int i = 1; i < (int)v.size(); ++i) {
        T key = v[i];
        int j = i - 1;
        while (j >= 0 && v[j].getID() > key.getID()) {
            v[j + 1] = v[j];
            --j;
        }
        v[j + 1] = key;
    }
}

// DSA: Binary Search - O(log n) lookup by ID
template<typename T>
int binarySearchByID(const vector<T>& v, int id) {
    int lo = 0, hi = (int)v.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int mid_id = v[mid].getID();
        if (mid_id == id) return mid;
        else if (mid_id < id) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

class UserManager {
    vector<Donor> donors;
    vector<Recipient> recipients;
    vector<Volunteer> volunteers;
    IntQueue freeVolunteerQueue; // DSA: FIFO queue

    int nextDonorID;
    int nextRecipientID;
    int nextVolunteerID;

    void rebuildFreeQueue() {
        freeVolunteerQueue.clear();
        for (auto& v : volunteers) {
            if (v.isFree()) freeVolunteerQueue.enqueue(v.getID());
        }
    }

public:
    UserManager() : nextDonorID(1), nextRecipientID(1), nextVolunteerID(1) {}

    void registerDonor() {
        cout << "\n=== Register Donor ===\n";
        string name, phone, address, org, catIn;
        cout << "Name    : "; getline(cin, name);
        if (name.empty()) { cout << "Name cannot be empty.\n"; return; }
        cout << "Phone   : "; getline(cin, phone);
        cout << "Address : "; getline(cin, address);
        cout << "Organization / Personal name: "; getline(cin, org);
        cout << "Donor Category:\n  1. Restaurant\n  2. Individual\n  3. Supermarket\n  4. Other\nChoice: ";
        getline(cin, catIn);
        int ch = 4;
        try { ch = stoi(catIn); } catch (...) {}
        string cat;
        switch (ch) {
            case 1: cat = "Restaurant"; break;
            case 2: cat = "Individual"; break;
            case 3: cat = "Supermarket"; break;
            default: cat = "Other";
        }
        Donor d(nextDonorID++, name, phone, address, org, cat);
        donors.push_back(d);
        // IDs increase, so vector stays sorted for binary search
        cout << "Donor registered! ID = " << d.getID() << "\n";
    }

    void registerRecipient() {
        cout << "\n=== Register Recipient ===\n";
        string name, phone, address, org, catIn;
        cout << "Name    : "; getline(cin, name);
        if (name.empty()) { cout << "Name cannot be empty.\n"; return; }
        cout << "Phone   : "; getline(cin, phone);
        cout << "Address : "; getline(cin, address);
        cout << "Organization / Personal name: "; getline(cin, org);
        cout << "Recipient Category:\n  1. NGO\n  2. Shelter\n  3. Individual\n  4. Other\nChoice: ";
        getline(cin, catIn);
        int ch = 4;
        try { ch = stoi(catIn); } catch (...) {}
        string cat;
        switch (ch) {
            case 1: cat = "NGO"; break;
            case 2: cat = "Shelter"; break;
            case 3: cat = "Individual"; break;
            default: cat = "Other";
        }
        Recipient r(nextRecipientID++, name, phone, address, org, cat);
        recipients.push_back(r);
        cout << "Recipient registered! ID = " << r.getID() << "\n";
    }

    void registerVolunteer() {
        cout << "\n=== Register Volunteer ===\n";
        string name, phone, address, vIn;
        cout << "Name    : "; getline(cin, name);
        if (name.empty()) { cout << "Name cannot be empty.\n"; return; }
        cout << "Phone   : "; getline(cin, phone);
        cout << "Address : "; getline(cin, address);
        cout << "Vehicle Type:\n  1. Bike\n  2. Car\n  3. Van\nChoice: ";
        getline(cin, vIn);
        int ch = 1;
        try { ch = stoi(vIn); } catch (...) {}
        string vehicle;
        switch (ch) {
            case 2: vehicle = "Car"; break;
            case 3: vehicle = "Van"; break;
            default: vehicle = "Bike";
        }
        Volunteer v(nextVolunteerID++, name, phone, address, "Available", vehicle, 0);
        volunteers.push_back(v);
        freeVolunteerQueue.enqueue(v.getID());
        cout << "Volunteer registered! ID = " << v.getID() << "\n";
    }

    Donor* findDonorByID(int id) {
        int i = binarySearchByID(donors, id);
        return (i >= 0) ? &donors[i] : nullptr;
    }
    Recipient* findRecipientByID(int id) {
        int i = binarySearchByID(recipients, id);
        return (i >= 0) ? &recipients[i] : nullptr;
    }
    Volunteer* findVolunteerByID(int id) {
        int i = binarySearchByID(volunteers, id);
        return (i >= 0) ? &volunteers[i] : nullptr;
    }

    int peekNextAvailableVolunteer() const { return freeVolunteerQueue.peek(); }
    int getNextAvailableVolunteer() { return freeVolunteerQueue.dequeue(); }

    void markVolunteerAssigned(int id) {
        Volunteer* v = findVolunteerByID(id);
        if (!v) return;
        v->setIsAssigned(1);
        v->setAvailability("Busy");
        freeVolunteerQueue.removeValue(id);
    }

    void releaseVolunteer(int id) {
        Volunteer* v = findVolunteerByID(id);
        if (!v) return;
        v->setIsAssigned(0);
        v->setAvailability("Available");
        if (!freeVolunteerQueue.contains(id)) freeVolunteerQueue.enqueue(id);
    }

    void changeVolunteerAvailability(int id) {
        Volunteer* v = findVolunteerByID(id);
        if (!v) { cout << "Volunteer ID " << id << " not found.\n"; return; }
        if (v->getIsAssigned()) {
            cout << "Cannot change availability while assigned.\n"; return;
        }
        cout << "Current: " << v->getAvailability()
             << "\nSet to:\n  1. Available\n  2. Unavailable\nChoice: ";
        string ch; getline(cin, ch);
        if (ch == "1") {
            v->setAvailability("Available");
            if (!freeVolunteerQueue.contains(id)) freeVolunteerQueue.enqueue(id);
            cout << "Volunteer " << id << " set to Available.\n";
        } else {
            v->setAvailability("Unavailable");
            freeVolunteerQueue.removeValue(id);
            cout << "Volunteer " << id << " set to Unavailable.\n";
        }
    }

    void displayAllDonors() const {
        if (donors.empty()) { cout << "  No donors registered.\n"; return; }
        for (auto& d : donors) d.displayProfile();
    }
    void displayAllRecipients() const {
        if (recipients.empty()) { cout << "  No recipients registered.\n"; return; }
        for (auto& r : recipients) r.displayProfile();
    }
    void displayAllVolunteers() const {
        if (volunteers.empty()) { cout << "  No volunteers registered.\n"; return; }
        for (auto& v : volunteers) v.displayProfile();
    }

    void displayFreeVolunteerQueue() const {
        cout << "\n--- Free Volunteer Queue (FIFO) ---\n";
        if (freeVolunteerQueue.isEmpty()) { cout << "  Queue is empty.\n"; return; }
        auto ids = freeVolunteerQueue.toVector();
        cout << "  Front -> ";
        for (int i = 0; i < (int)ids.size(); ++i) {
            cout << "[V" << ids[i] << "]";
            if (i + 1 < (int)ids.size()) cout << " -> ";
        }
        cout << " <- Back\n";
    }

    int donorCount() const { return (int)donors.size(); }
    int recipientCount() const { return (int)recipients.size(); }
    int volunteerCount() const { return (int)volunteers.size(); }
    int freeVolCount() const { return freeVolunteerQueue.size(); }
};
