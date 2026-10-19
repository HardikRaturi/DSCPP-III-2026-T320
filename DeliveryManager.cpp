#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class DeliveryManager {
    vector<Delivery> deliveries;
    IntQueue pendingDeliveryQueue; // DSA: FIFO queue
    int nextDeliveryID;
    UserManager* um;
    FoodManager* fm;
    RequestManager* rm;

    Delivery* findByID(int did) {
        for (auto& d : deliveries)
            if (d.getDeliveryID() == did) return &d;
        return nullptr;
    }
    Delivery* findByRequest(int rid) {
        for (auto& d : deliveries)
            if (d.getRequestID() == rid) return &d;
        return nullptr;
    }
public:
    DeliveryManager(UserManager* u, FoodManager* f, RequestManager* r)
        : nextDeliveryID(301), um(u), fm(f), rm(r) {}

    void assignPendingDeliveries() {
        auto ids = pendingDeliveryQueue.toVector();
        for (int did : ids) {
            Delivery* d = findByID(did);
            if (!d || d->getStatus() != "Pending") continue;
            int vid = um->getNextAvailableVolunteer();
            if (vid == -1) break;
            d->setVolunteerID(vid);
            d->setStatus("Assigned");
            d->setAssignedDate(todayDate());
            um->markVolunteerAssigned(vid);
            rm->setRequestStatus(d->getRequestID(), "Accepted");
            pendingDeliveryQueue.removeValue(did);
        }
    }

    bool createDelivery(int requestID) {
        Request* req = rm->findRequestByID(requestID);
        if (!req) { cout << "Request ID " << requestID << " not found.\n"; return false; }
        if (req->getStatus() != "Matched") {
            cout << "Request must be Matched to create delivery.\n"; return false;
        }
        int fid = req->getMatchedFoodID();
        Food* food = fm->findFoodByID(fid);
        if (!food || food->getStatus() != "Reserved") {
            cout << "Food " << fid << " is not Reserved.\n"; return false;
        }
        if (findByRequest(requestID)) {
            cout << "Delivery already exists for Request " << requestID << ".\n"; return false;
        }
        int vid = um->getNextAvailableVolunteer();
        Delivery del(nextDeliveryID++, requestID, fid, vid);
        if (vid != -1) {
            del.setStatus("Assigned");
            del.setAssignedDate(todayDate());
            um->markVolunteerAssigned(vid);
            rm->setRequestStatus(requestID, "Accepted");
            cout << "Delivery " << del.getDeliveryID()
                 << " created and assigned to Volunteer " << vid << ".\n";
        } else {
            del.setStatus("Pending");
            pendingDeliveryQueue.enqueue(del.getDeliveryID());
            cout << "Delivery " << del.getDeliveryID()
                 << " created. No volunteer - queued as Pending.\n";
        }
        deliveries.push_back(del);
        return true;
    }

    bool markPickedUp(int deliveryID, int volunteerID) {
        Delivery* d = findByID(deliveryID);
        if (!d) { cout << "Delivery not found.\n"; return false; }
        if (d->getVolunteerID() != volunteerID) {
            cout << "Not assigned to you.\n"; return false;
        }
        if (d->getStatus() != "Assigned") {
            cout << "Must be Assigned (current: " << d->getStatus() << ").\n"; return false;
        }
        d->setStatus("PickedUp");
        fm->setFoodStatus(d->getFoodID(), "Collected");
        cout << "Delivery " << deliveryID << " marked PickedUp.\n";
        return true;
    }

    bool markDelivered(int deliveryID, int volunteerID) {
        Delivery* d = findByID(deliveryID);
        if (!d) { cout << "Delivery not found.\n"; return false; }
        if (d->getVolunteerID() != volunteerID) {
            cout << "Not assigned to you.\n"; return false;
        }
        if (d->getStatus() != "PickedUp") {
            cout << "Must be PickedUp first.\n"; return false;
        }
        d->setStatus("Delivered");
        rm->setRequestStatus(d->getRequestID(), "Completed");
        um->releaseVolunteer(volunteerID);
        assignPendingDeliveries();
        cout << "Delivery " << deliveryID << " marked Delivered.\n";
        return true;
    }

    void displayAll() const {
        if (deliveries.empty()) { cout << "  No deliveries.\n"; return; }
        for (auto& d : deliveries) { cout << "\n"; d.display(); }
    }

    void displayPendingDeliveryQueue() const {
        cout << "\n--- Pending Delivery Queue (FIFO) ---\n";
        if (pendingDeliveryQueue.isEmpty()) { cout << "  Queue is empty.\n"; return; }
        auto ids = pendingDeliveryQueue.toVector();
        cout << "  Front -> ";
        for (int i = 0; i < (int)ids.size(); ++i) {
            cout << "[D" << ids[i] << "]";
            if (i + 1 < (int)ids.size()) cout << " -> ";
        }
        cout << " <- Back\n";
    }

    int deliveryCount() const { return (int)deliveries.size(); }
};
