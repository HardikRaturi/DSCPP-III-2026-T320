#pragma once
#include <iostream>
#include <string>
using namespace std;

// OOP: Inheritance - Donor inherits from User
class Donor : public User {
    string organizationName;
    string donorCategory; // Restaurant | Individual | Supermarket | Other
public:
    Donor() : User() {}
    Donor(int id, const string& n, const string& ph, const string& addr,
          const string& org, const string& cat)
        : User(id, n, ph, addr, "Donor"),
          organizationName(org), donorCategory(cat) {}

    const string& getOrganizationName() const { return organizationName; }
    const string& getDonorCategory() const { return donorCategory; }
    void setOrganizationName(const string& v) { organizationName = v; }
    void setDonorCategory(const string& v) { donorCategory = v; }

    string getRole() const override { return "Donor"; }

    void displayProfile() const override {
        cout << "\n--- Donor Profile ---\n";
        User::displayProfile();
        cout << "  Org     : " << organizationName << "\n"
             << "  Category: " << donorCategory << "\n";
    }
};
