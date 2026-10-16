#pragma once
#include <iostream>
#include <string>
using namespace std;

// OOP: Inheritance - Recipient inherits from User
class Recipient : public User {
    string organizationName;
    string recipientCategory; // NGO | Shelter | Individual | Other
public:
    Recipient() : User() {}
    Recipient(int id, const string& n, const string& ph, const string& addr,
              const string& org, const string& cat)
        : User(id, n, ph, addr, "Recipient"),
          organizationName(org), recipientCategory(cat) {}

    const string& getOrganizationName() const { return organizationName; }
    const string& getRecipientCategory() const { return recipientCategory; }
    void setOrganizationName(const string& v) { organizationName = v; }
    void setRecipientCategory(const string& v) { recipientCategory = v; }

    string getRole() const override { return "Recipient"; }

    void displayProfile() const override {
        cout << "\n--- Recipient Profile ---\n";
        User::displayProfile();
        cout << "  Org     : " << organizationName << "\n"
             << "  Category: " << recipientCategory << "\n";
    }
};
