#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class MatchingEngine {
    FoodManager* fm;
    RequestManager* rm;
public:
    MatchingEngine(FoodManager* f, RequestManager* r) : fm(f), rm(r) {}

    int matchRequest(Request* req) {
        if (!req || req->getStatus() != "Pending") return -1;
        vector<Food*> available = fm->getAvailableFood();
        Food* best = nullptr;
        string today = todayDate();

        for (Food* f : available) {
            if (f->getFoodType() != req->getFoodType()) continue;
            if (f->getUnit() != req->getUnit()) continue;
            if (f->getQuantity() < req->getQuantity()) continue;
            if (f->isExpired(today)) continue;

            if (!best) { best = f; continue; }

            bool bestSameLoc = (best->getPickupAddress() == req->getPreferredLocation());
            bool thisSameLoc = (f->getPickupAddress() == req->getPreferredLocation());
            if (thisSameLoc && !bestSameLoc) { best = f; continue; }
            if (!thisSameLoc && bestSameLoc) continue;

            if (f->getExpiryDate() < best->getExpiryDate()) { best = f; continue; }
            if (f->getExpiryDate() > best->getExpiryDate()) continue;

            if (f->getQuantity() < best->getQuantity()) { best = f; continue; }
        }

        if (!best) return -1;

        fm->setFoodStatus(best->getFoodID(), "Reserved");
        rm->setRequestStatus(req->getRequestID(), "Matched", best->getFoodID());
        cout << "  Matched Request " << req->getRequestID()
             << " with Food " << best->getFoodID() << "\n";
        return best->getFoodID();
    }

    int matchNextPending() {
        Request* req = rm->getPendingTop();
        if (!req) { cout << "No pending requests.\n"; return -1; }
        int fid = matchRequest(req);
        if (fid == -1) {
            cout << "No suitable food found for Request " << req->getRequestID() << ".\n";
        }
        return fid;
    }

    void matchAllPending() {
        int matched = 0;
        auto snap = rm->getPendingHeapSnapshot();
        for (int rid : snap) {
            Request* req = rm->findRequestByID(rid);
            if (req && req->getStatus() == "Pending") {
                if (matchRequest(req) != -1) matched++;
            }
        }
        cout << "Matching complete. " << matched << " request(s) matched.\n";
    }
};
