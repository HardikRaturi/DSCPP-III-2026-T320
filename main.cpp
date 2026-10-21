#include <iostream>
#include <string>
using namespace std;

// Include all component .cpp files (no .h files used)
#include "IntQueue.cpp"
#include "User.cpp"
#include "Donor.cpp"
#include "Recipient.cpp"
#include "Volunteer.cpp"
#include "UserManager.cpp"
#include "Food.cpp"
#include "LinkedList.cpp"
#include "FoodManager.cpp"
#include "Request.cpp"
#include "RequestHeap.cpp"
#include "RequestManager.cpp"
#include "Delivery.cpp"
#include "MatchingEngine.cpp"
#include "DeliveryManager.cpp"

void printHeader() {
    cout << "\n================================\n";
    cout << "          FOODRESCUE\n";
    cout << "     Save Food. Serve People.\n";
    cout << "================================\n";
}

int promptInt(const string& msg) {
    cout << msg;
    string s; getline(cin, s);
    try { return stoi(s); } catch (...) { return -1; }
}

int main() {
    UserManager um;
    FoodManager fm;
    RequestManager rm;
    MatchingEngine me(&fm, &rm);
    DeliveryManager dm(&um, &fm, &rm);

    // Mark expired at startup (in-memory only)
    fm.markExpired(todayDate());

    printHeader();

    while (true) {
        cout << "\n================================\n";
        cout << "          FOODRESCUE\n";
        cout << "     Save Food. Serve People.\n";
        cout << "================================\n";
        cout << "1. Register Donor\n";
        cout << "2. Register Recipient\n";
        cout << "3. Register Volunteer\n";
        cout << "4. Add Food Donation\n";
        cout << "5. Create Food Request\n";
        cout << "6. View Donations\n";
        cout << "7. View Requests\n";
        cout << "8. Match Food\n";
        cout << "9. Process Delivery\n";
        cout << "10. Display All Data\n";
        cout << "11. Exit\n";
        cout << "Enter your choice: ";

        string ch; getline(cin, ch);

        if (ch == "1") {
            um.registerDonor();
        }
        else if (ch == "2") {
            um.registerRecipient();
        }
        else if (ch == "3") {
            um.registerVolunteer();
        }
        else if (ch == "4") {
            int id = promptInt("Enter Donor ID: ");
            Donor* d = um.findDonorByID(id);
            if (!d) cout << "Donor not found.\n";
            else fm.addFood(id, d->getAddress());
        }
        else if (ch == "5") {
            int id = promptInt("Enter Recipient ID: ");
            Recipient* r = um.findRecipientByID(id);
            if (!r) cout << "Recipient not found.\n";
            else rm.createRequest(id);
        }
        else if (ch == "6") {
            cout << "\n--- Food Donations ---\n";
            fm.displayAll();
        }
        else if (ch == "7") {
            cout << "\n--- Food Requests ---\n";
            rm.displayAll();
        }
        else if (ch == "8") {
            cout << "\n--- Matching ---\n";
            cout << "1. Match next pending\n2. Match all pending\nChoice: ";
            string m; getline(cin, m);
            if (m == "1") me.matchNextPending();
            else me.matchAllPending();
        }
        else if (ch == "9") {
            cout << "\n--- Process Delivery ---\n";
            cout << "1. Create delivery for Matched request\n";
            cout << "2. Mark PickedUp\n";
            cout << "3. Mark Delivered\n";
            cout << "Choice: ";
            string dch; getline(cin, dch);
            if (dch == "1") {
                int rid = promptInt("Enter Matched Request ID: ");
                dm.createDelivery(rid);
            } else if (dch == "2") {
                int did = promptInt("Delivery ID: ");
                int vid = promptInt("Volunteer ID: ");
                dm.markPickedUp(did, vid);
            } else if (dch == "3") {
                int did = promptInt("Delivery ID: ");
                int vid = promptInt("Volunteer ID: ");
                dm.markDelivered(did, vid);
            } else cout << "Invalid.\n";
        }
        else if (ch == "10") {
            cout << "\n========== ALL DATA ==========\n";
            cout << "\n--- Donors ---\n"; um.displayAllDonors();
            cout << "\n--- Recipients ---\n"; um.displayAllRecipients();
            cout << "\n--- Volunteers ---\n"; um.displayAllVolunteers();
            cout << "\n--- Donations ---\n"; fm.displayAll();
            cout << "\n--- Requests ---\n"; rm.displayAll();
            cout << "\n--- Deliveries ---\n"; dm.displayAll();
            um.displayFreeVolunteerQueue();
            rm.displayPendingHeap();
            dm.displayPendingDeliveryQueue();
        }
        else if (ch == "11") {
            cout << "Thank you for using FoodRescue!\n";
            break;
        }
        else {
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}
