#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

// Tiny JSON helpers. No external library - just enough for our API.
// We build JSON responses by hand and parse only flat {"k":"v","n":123} bodies.

// Escape a string for safe embedding inside JSON quotes.
static string jsonEscape(const string& s) {
    string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c >= 0 && c < 32) { out += ' '; }
                else out += c;
        }
    }
    return out;
}

// Wrap an already-escaped/plain value in quotes.
static string jq(const string& s) { return "\"" + jsonEscape(s) + "\""; }

// Minimal JSON object builder.
class JsonObj {
    string body;
    bool first;
public:
    JsonObj() : first(true) {}
    void add(const string& key, const string& val) {
        if (!first) body += ",";
        body += jq(key) + ":" + jq(val);
        first = false;
    }
    void add(const string& key, int val) {
        if (!first) body += ",";
        body += jq(key) + ":" + to_string(val);
        first = false;
    }
    void add(const string& key, bool val) {
        if (!first) body += ",";
        body += jq(key) + ":" + (val ? "true" : "false");
        first = false;
    }
    void add(const string& key, double val) {  // 1 decimal, e.g. 7.0 km
        if (!first) body += ",";
        char buf[32]; snprintf(buf, sizeof(buf), "%.1f", val);
        body += jq(key) + ":" + buf;
        first = false;
    }
    // add a pre-built JSON value (object/array/number) without quoting
    void addRaw(const string& key, const string& rawJson) {
        if (!first) body += ",";
        body += jq(key) + ":" + rawJson;
        first = false;
    }
    string str() const { return "{" + body + "}"; }
};

// Minimal JSON array builder.
class JsonArr {
    string body;
    bool first;
public:
    JsonArr() : first(true) {}
    void addRaw(const string& rawJson) {
        if (!first) body += ",";
        body += rawJson;
        first = false;
    }
    string str() const { return "[" + body + "]"; }
};

// Parse a flat JSON object body into a string->string map.
// Handles: {"name":"Asha","qty":10,"ok":true}  (nested objects/arrays not supported)
static map<string, string> parseJsonBody(const string& body) {
    map<string, string> m;
    size_t i = 0, n = body.size();
    auto skipWs = [&]() { while (i < n && (body[i]==' '||body[i]=='\t'||body[i]=='\n'||body[i]=='\r')) i++; };
    auto parseStr = [&]() -> string {
        string out;
        if (i < n && body[i] == '"') i++;
        while (i < n) {
            char c = body[i++];
            if (c == '\\' && i < n) { out += body[i++]; }
            else if (c == '"') break;
            else out += c;
        }
        return out;
    };
    skipWs();
    if (i < n && body[i] == '{') i++;
    while (true) {
        skipWs();
        if (i >= n || body[i] == '}') break;
        string key = parseStr();
        skipWs();
        if (i < n && body[i] == ':') i++;
        skipWs();
        string val;
        if (i < n && body[i] == '"') val = parseStr();
        else { while (i < n && body[i] != ',' && body[i] != '}') val += body[i++]; }
        // trim spaces from unquoted value
        size_t a = val.find_first_not_of(" \t\n\r"), b = val.find_last_not_of(" \t\n\r");
        if (a != string::npos) val = val.substr(a, b - a + 1); else val = "";
        m[key] = val;
        skipWs();
        if (i < n && body[i] == ',') { i++; continue; }
        break;
    }
    return m;
}

static string jsonGet(const map<string,string>& m, const string& key, const string& dflt = "") {
    auto it = m.find(key);
    return it == m.end() ? dflt : it->second;
}
static int jsonGetInt(const map<string,string>& m, const string& key, int dflt = 0) {
    auto it = m.find(key);
    if (it == m.end()) return dflt;
    try { return stoi(it->second); } catch (...) { return dflt; }
}
