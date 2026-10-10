#pragma once
#include <iostream>
#include <string>
using namespace std;

// OOP: Base class - Encapsulation + Polymorphism
class User {
protected:
    int userID;
    string name;
    string phone;
    string address;
    string userType;
public:
    User() : userID(0) {}
    User(int id, const string& n, const string& ph,
         const string& addr, const string& type)
        : userID(id), name(n), phone(ph), address(addr), userType(type) {}
    virtual ~User() {}

    int getID() const { return userID; }
    const string& getName() const { return name; }
    const string& getPhone() const { return phone; }
    const string& getAddress() const { return address; }
    const string& getUserType() const { return userType; }

    void setName(const string& v) { name = v; }
    void setPhone(const string& v) { phone = v; }
    void setAddress(const string& v) { address = v; }

    virtual void displayProfile() const {
        cout << "  ID      : " << userID << "\n"
             << "  Name    : " << name << "\n"
             << "  Phone   : " << phone << "\n"
             << "  Address : " << address << "\n"
             << "  Type    : " << userType << "\n";
    }
    virtual string getRole() const { return userType; }
};
