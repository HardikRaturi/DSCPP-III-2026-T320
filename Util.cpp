#pragma once
#include <string>
#include <ctime>
using namespace std;

// WHAT: date/time and string helpers shared by every manager
// WHY HERE: expiry checks, priority aging, logs and analytics all need the same date math
// COMPLEXITY: all O(1), except trim/toLower which are O(n) on the string length
// VIVA ONE-LINER: "Dates become plain integers, so 'expires in 3 days' is just subtraction."

// "YYYY-MM-DD HH:MM:SS" in local time, for log stamps and timestamps
string nowStamp() {
    time_t t = time(nullptr);
    tm* lt = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    return string(buf);
}

// "YYYY-MM-DD" in local time (single definition; used project-wide)
string todayDate() {
    time_t t = time(nullptr);
    tm* lt = localtime(&t);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
             lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
    return string(buf);
}

// Seconds since 1970-01-01 00:00:00 UTC
long nowEpoch() {
    return (long)time(nullptr);
}

// Internal: is d a real calendar date in YYYY-MM-DD form? (leap-year aware)
static bool isYMD(const string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (d[i] < '0' || d[i] > '9') return false;
    }
    int y = stoi(d.substr(0, 4)), m = stoi(d.substr(5, 2)), day = stoi(d.substr(8, 2));
    if (y < 1970 || y > 2100 || m < 1 || m > 12 || day < 1) return false;
    int dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) dim[2] = 29;
    return day <= dim[m];
}

// Days since 1970-01-01 for a YYYY-MM-DD date; -1 if not a valid date.
// Uses a plain leap-year-aware formula (no <chrono>).
int dayNumber(const string& d) {
    if (!isYMD(d)) return -1;
    int y = stoi(d.substr(0, 4)), m = stoi(d.substr(5, 2)), day = stoi(d.substr(8, 2));
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);                    // [0, 399]
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + day - 1; // [0, 365]
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;       // [0, 146096]
    return era * 146097 + (int)doe - 719468;
}

// Whole days from today until d (negative = past, 0 = today)
int daysUntil(const string& d) {
    return dayNumber(d) - dayNumber(todayDate());
}

string toLower(const string& s) {
    string r = s;
    for (char& c : r) c = (char)tolower((unsigned char)c);
    return r;
}

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
