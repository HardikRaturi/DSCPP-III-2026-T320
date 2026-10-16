#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class FoodManager {
    FoodNode* head;
    int count;
    int nextFoodID;

    void appendNode(const Food& f) {
        FoodNode* n = new FoodNode(f);
        if (!head) { head = n; }
        else {
            FoodNode* cur = head;
            while (cur->next) cur = cur->next;
            cur->next = n;
        }
        count++;
        if (f.getFoodID() >= nextFoodID) nextFoodID = f.getFoodID() + 1;
    }

    // DSA: Linear search on linked list
    FoodNode* findNode(int fid) const {
        FoodNode* cur = head;
        while (cur) {
            if (cur->data.getFoodID() == fid) return cur;
            cur = cur->next;
        }
        return nullptr;
    }

public:
    FoodManager() : head(nullptr), count(0), nextFoodID(101) {}
    ~FoodManager() {
        FoodNode* cur = head;
        while (cur) { FoodNode* t = cur; cur = cur->next; delete t; }
    }

    void markExpired(const string& today) {
        FoodNode* cur = head;
        int marked = 0;
        while (cur) {
            Food& f = cur->data;
            if (f.getStatus() == "Available" && f.isExpired(today)) {
                f.setStatus("Expired");
                marked++;
            }
            cur = cur->next;
        }
        if (marked) cout << "[FoodManager] Marked " << marked << " items as Expired.\n";
    }

    void addFood(int donorID, const string& pickupAddress) {
        cout << "\n=== Add Food Donation ===\n";
        string name, ftype, unitIn, expiry, addr, qtyIn;
        cout << "Food Name  : "; getline(cin, name);
        if (name.empty()) { cout << "Name cannot be empty.\n"; return; }
        cout << "Food Type (Cooked/Raw/Packaged/Produce): "; getline(cin, ftype);
        cout << "Quantity   : "; getline(cin, qtyIn);
        int qty = 0;
        try { qty = stoi(qtyIn); } catch (...) { cout << "Invalid quantity.\n"; return; }
        if (qty <= 0) { cout << "Quantity must be positive.\n"; return; }
        cout << "Unit (kg/servings/boxes): "; getline(cin, unitIn);
        cout << "Expiry Date (YYYY-MM-DD): "; getline(cin, expiry);
        cout << "Pickup Address [Enter for donor address]: "; getline(cin, addr);
        if (addr.empty()) addr = pickupAddress;

        Food f(nextFoodID++, donorID, name, ftype, qty, unitIn, expiry, addr, "Available");
        appendNode(f);
        cout << "Food listed! Food ID = " << f.getFoodID() << "\n";
    }

    Food* findFoodByID(int fid) {
        FoodNode* n = findNode(fid);
        return n ? &n->data : nullptr;
    }

    void displayAll() const {
        if (!head) { cout << "  No food donations.\n"; return; }
        FoodNode* cur = head;
        while (cur) { cout << "\n"; cur->data.display(); cur = cur->next; }
    }

    void displayByDonor(int donorID) const {
        bool found = false;
        FoodNode* cur = head;
        while (cur) {
            if (cur->data.getDonorID() == donorID) {
                cout << "\n"; cur->data.display(); found = true;
            }
            cur = cur->next;
        }
        if (!found) cout << "  No donations for donor ID " << donorID << ".\n";
    }

    bool setFoodStatus(int foodID, const string& newStatus) {
        FoodNode* n = findNode(foodID);
        if (!n) return false;
        n->data.setStatus(newStatus);
        return true;
    }

    vector<Food*> getAvailableFood() {
        vector<Food*> result;
        FoodNode* cur = head;
        while (cur) {
            if (cur->data.getStatus() == "Available") result.push_back(&cur->data);
            cur = cur->next;
        }
        return result;
    }

    int foodCount() const { return count; }
};
