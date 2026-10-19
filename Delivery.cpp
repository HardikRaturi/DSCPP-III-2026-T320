#pragma once
#include <iostream>
#include <string>
using namespace std;

class Delivery {
    int deliveryID;
    int requestID;
    int foodID;
    int volunteerID; // -1 = unassigned
    string status; // Pending | Assigned | PickedUp | Delivered | Cancelled
    string assignedDate;
public:
    Delivery() : deliveryID(0), requestID(0), foodID(0), volunteerID(-1), status("Pending") {}
    Delivery(int did, int rid, int fid, int vid = -1,
             const string& st = "Pending", const string& dt = "")
        : deliveryID(did), requestID(rid), foodID(fid),
          volunteerID(vid), status(st), assignedDate(dt) {}

    int getDeliveryID() const { return deliveryID; }
    int getRequestID() const { return requestID; }
    int getFoodID() const { return foodID; }
    int getVolunteerID() const { return volunteerID; }
    const string& getStatus() const { return status; }
    const string& getAssignedDate() const { return assignedDate; }

    void setStatus(const string& s) { status = s; }
    void setVolunteerID(int v) { volunteerID = v; }
    void setAssignedDate(const string& d) { assignedDate = d; }

    void display() const {
        cout << "  Delivery ID : " << deliveryID << "\n"
             << "  Request ID  : " << requestID << "\n"
             << "  Food ID     : " << foodID << "\n"
             << "  Volunteer ID: " << (volunteerID == -1 ? "Unassigned" : to_string(volunteerID)) << "\n"
             << "  Status      : " << status << "\n"
             << "  Assigned On : " << (assignedDate.empty() ? "N/A" : assignedDate) << "\n";
    }
};
