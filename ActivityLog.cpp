#pragma once
#include <string>
#include <vector>
using namespace std;

// WHAT: fixed-capacity circular buffer of timestamped log strings
// WHY HERE: the dashboard "recent activity" feed; bounded memory, O(1) append,
//           and old entries are exactly what we want to forget first
// COMPLEXITY: push O(1), getAll O(n)
// VIVA ONE-LINER: "When full it overwrites the oldest entry, so the log can never grow without bound."

const int ACTIVITY_CAPACITY = 50;  // max entries kept; oldest is overwritten first

class ActivityLog {
    string entries[ACTIVITY_CAPACITY];
    int head;    // index of the oldest entry
    int count;   // entries currently stored (<= ACTIVITY_CAPACITY)
public:
    ActivityLog() : head(0), count(0) {}
    void push(const string& msg) {
        string stamped = nowStamp() + " " + msg;
        if (count < ACTIVITY_CAPACITY) {
            entries[(head + count) % ACTIVITY_CAPACITY] = stamped;
            count++;
        } else {
            entries[head] = stamped;                    // overwrite oldest
            head = (head + 1) % ACTIVITY_CAPACITY;
        }
    }
    // Oldest to newest. Exempt from undo: pure bookkeeping, not user data.
    vector<string> getAll() const {
        vector<string> out;
        for (int i = 0; i < count; i++) out.push_back(entries[(head + i) % ACTIVITY_CAPACITY]);
        return out;
    }
    int size() const { return count; }
};
