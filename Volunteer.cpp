#pragma once
#include <iostream>
#include <string>
using namespace std;

// OOP: Inheritance - Volunteer inherits from User
class Volunteer : public User {
    string availability; // Available | Unavailable | Busy
    string vehicleType;  // Bike | Car | Van
    int isAssigned;      // 0 = free, 1 = assigned
public:
    Volunteer() : User(), isAssigned(0) {}
    Volunteer(int id, const string& n, const string& ph, const string& addr,
              const string& avail, const string& vehicle, int assigned = 0)
        : User(id, n, ph, addr, "Volunteer"),
          availability(avail), vehicleType(vehicle), isAssigned(assigned) {}

    const string& getAvailability() const { return availability; }
    const string& getVehicleType() const { return vehicleType; }
    int getIsAssigned() const { return isAssigned; }

    void setAvailability(const string& v) { availability = v; }
    void setVehicleType(const string& v) { vehicleType = v; }
    void setIsAssigned(int v) { isAssigned = v; }

    bool isFree() const {
        return (availability == "Available") && (isAssigned == 0);
    }

    string getRole() const override { return "Volunteer"; }

    void displayProfile() const override {
        cout << "\n--- Volunteer Profile ---\n";
        User::displayProfile();
        cout << "  Availability: " << availability << "\n"
             << "  Vehicle     : " << vehicleType << "\n"
             << "  Assigned    : " << (isAssigned ? "Yes" : "No") << "\n";
    }
};
