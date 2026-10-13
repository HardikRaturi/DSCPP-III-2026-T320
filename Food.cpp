#pragma once
#include <iostream>
#include <string>
#include <ctime>
using namespace std;

// Helper: today's date as YYYY-MM-DD (used for expiry check)
static string todayDate() {
    time_t t = time(nullptr);
    tm* lt = localtime(&t);
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
             lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
    return string(buf);
}

class Food {
    int foodID;
    int donorID;
    string foodName;
    string foodType;   // Cooked | Raw | Packaged | Produce
    int quantity;
    string unit;       // kg | servings | boxes
    string expiryDate; // YYYY-MM-DD
    string pickupAddress;
    string status;     // Available | Reserved | Collected | Expired
public:
    Food() : foodID(0), donorID(0), quantity(0) {}
    Food(int fid, int did, const string& nm, const string& ftype,
         int qty, const string& u, const string& exp,
         const string& addr, const string& st = "Available")
        : foodID(fid), donorID(did), foodName(nm), foodType(ftype),
          quantity(qty), unit(u), expiryDate(exp),
          pickupAddress(addr), status(st) {}

    int getFoodID() const { return foodID; }
    int getDonorID() const { return donorID; }
    const string& getFoodName() const { return foodName; }
    const string& getFoodType() const { return foodType; }
    int getQuantity() const { return quantity; }
    const string& getUnit() const { return unit; }
    const string& getExpiryDate() const { return expiryDate; }
    const string& getPickupAddress() const { return pickupAddress; }
    const string& getStatus() const { return status; }

    void setStatus(const string& s) { status = s; }
    void setQuantity(int q) { quantity = q; }

    bool isExpired(const string& today) const {
        return expiryDate < today;
    }

    void display() const {
        cout << "  Food ID   : " << foodID << "\n"
             << "  Donor ID  : " << donorID << "\n"
             << "  Name      : " << foodName << "\n"
             << "  Type      : " << foodType << "\n"
             << "  Qty/Unit  : " << quantity << " " << unit << "\n"
             << "  Expiry    : " << expiryDate << "\n"
             << "  Pickup    : " << pickupAddress << "\n"
             << "  Status    : " << status << "\n";
    }
};
