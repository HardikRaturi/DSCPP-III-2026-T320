#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
using namespace std;

// Demo-grade authentication: in-memory accounts + random session tokens.
// Passwords are stored plain-text — fine for a classroom demo, NOT for production.

static string randomToken() {
    static const char* hex = "0123456789abcdef";
    string t;
    t.reserve(32);
    for (int i = 0; i < 32; i++) t += hex[rand() % 16];
    return t;
}

struct Account {
    string username;
    string password; // plain text — demo only
    string role;     // "admin"
};

class AuthManager {
    vector<Account> accounts;
    HashTable userIndex;          // DSA: username -> index in accounts (O(1) lookup)
    map<string, string> sessions; // token -> username
public:
    AuthManager() {
        srand((unsigned)time(nullptr));
        addUser("admin", "admin123", "admin");
    }
    // Returns a session token, or "" when credentials are wrong.
    string login(const string& user, const string& pass) {
        int idx;
        if (userIndex.get(user, idx) && idx >= 0 && idx < (int)accounts.size()
            && accounts[idx].password == pass) {
            string tok = randomToken();
            sessions[tok] = user;
            return tok;
        }
        return "";
    }
    bool valid(const string& token) const {
        return !token.empty() && sessions.count(token) > 0;
    }
    string who(const string& token) const {
        auto it = sessions.find(token);
        return it == sessions.end() ? "" : it->second;
    }
    void logout(const string& token) { sessions.erase(token); }

    bool addUser(const string& user, const string& pass, const string& role = "admin") {
        if (userIndex.contains(user)) return false;
        accounts.push_back({user, pass, role});
        userIndex.put(user, (int)accounts.size() - 1);
        return true;
    }
    vector<Account> listUsers() const { return accounts; }
};
