#pragma once
#include <iostream>
#include <string>
using namespace std;

class Request {
    int requestID;
    int recipientID;
    string foodType;
    int quantity;
    string unit;
    string preferredLocation;
    int priority; // 1-5, higher = more urgent
    string status; // Pending | Matched | Accepted | Completed | Cancelled
    int matchedFoodID;
public:
    Request() : requestID(0), recipientID(0), quantity(0), priority(1), matchedFoodID(-1) {}
    Request(int rid, int recid, const string& ftype, int qty,
            const string& u, const string& loc, int prio = 1,
            const string& st = "Pending", int mfid = -1)
        : requestID(rid), recipientID(recid), foodType(ftype),
          quantity(qty), unit(u), preferredLocation(loc),
          priority(prio), status(st), matchedFoodID(mfid) {}

    int getRequestID() const { return requestID; }
    int getRecipientID() const { return recipientID; }
    const string& getFoodType() const { return foodType; }
    int getQuantity() const { return quantity; }
    const string& getUnit() const { return unit; }
    const string& getPreferredLocation() const { return preferredLocation; }
    int getPriority() const { return priority; }
    const string& getStatus() const { return status; }
    int getMatchedFoodID() const { return matchedFoodID; }

    void setStatus(const string& s) { status = s; }
    void setMatchedFoodID(int id) { matchedFoodID = id; }
    void setFoodType(const string& v) { foodType = v; }
    void setQuantity(int v) { quantity = v; }
    void setUnit(const string& v) { unit = v; }
    void setPreferredLocation(const string& v) { preferredLocation = v; }
    void setPriority(int v) { priority = v; }

    void display() const {
        cout << "  Request ID: " << requestID << "\n"
             << "  Recipient : " << recipientID << "\n"
             << "  Food Type : " << foodType << "\n"
             << "  Qty/Unit  : " << quantity << " " << unit << "\n"
             << "  Location  : " << preferredLocation << "\n"
             << "  Priority  : " << priority << "\n"
             << "  Status    : " << status << "\n"
             << "  Matched ID: " << (matchedFoodID == -1 ? "None" : to_string(matchedFoodID)) << "\n";
    }
};
