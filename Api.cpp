#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

// FoodRescueApi: thin bridge between the HTTP server and the existing managers.
// It NEVER duplicates business logic - every operation calls the real manager
// methods. It only translates: JSON in -> manager call -> JSON out.

static string donorToJson(const Donor& d) {
    JsonObj o;
    o.add("id", d.getID()); o.add("name", d.getName()); o.add("phone", d.getPhone());
    o.add("address", d.getAddress()); o.add("org", d.getOrganizationName());
    o.add("category", d.getDonorCategory());
    return o.str();
}
static string recipientToJson(const Recipient& r) {
    JsonObj o;
    o.add("id", r.getID()); o.add("name", r.getName()); o.add("phone", r.getPhone());
    o.add("address", r.getAddress()); o.add("org", r.getOrganizationName());
    o.add("category", r.getRecipientCategory());
    return o.str();
}
static string volunteerToJson(const Volunteer& v) {
    JsonObj o;
    o.add("id", v.getID()); o.add("name", v.getName()); o.add("phone", v.getPhone());
    o.add("address", v.getAddress()); o.add("availability", v.getAvailability());
    o.add("vehicle", v.getVehicleType()); o.add("assigned", v.getIsAssigned());
    return o.str();
}
static string foodToJson(const Food& f) {
    JsonObj o;
    o.add("id", f.getFoodID()); o.add("donorId", f.getDonorID());
    o.add("name", f.getFoodName()); o.add("type", f.getFoodType());
    o.add("qty", f.getQuantity()); o.add("unit", f.getUnit());
    o.add("expiry", f.getExpiryDate()); o.add("address", f.getPickupAddress());
    o.add("status", f.getStatus());
    return o.str();
}
static string requestToJson(const Request& r) {
    JsonObj o;
    o.add("id", r.getRequestID()); o.add("recipientId", r.getRecipientID());
    o.add("type", r.getFoodType()); o.add("qty", r.getQuantity());
    o.add("unit", r.getUnit()); o.add("location", r.getPreferredLocation());
    o.add("priority", r.getPriority()); o.add("status", r.getStatus());
    o.add("matchedFoodId", r.getMatchedFoodID());
    return o.str();
}
static string deliveryToJson(const Delivery& d) {
    JsonObj o;
    o.add("id", d.getDeliveryID()); o.add("requestId", d.getRequestID());
    o.add("foodId", d.getFoodID()); o.add("volunteerId", d.getVolunteerID());
    o.add("status", d.getStatus()); o.add("date", d.getAssignedDate());
    return o.str();
}

static string okJson(const string& msg, int id = -1) {
    JsonObj o; o.add("ok", true); o.add("message", msg);
    if (id != -1) o.add("id", id);
    return o.str();
}
static string errJson(const string& msg) {
    JsonObj o; o.add("ok", false); o.add("error", msg);
    return o.str();
}

// ---- basic input validation (used by every mutating endpoint) ----
static bool validName(const string& s) {   // letters + spaces only, 2..60 chars
    if (s.size() < 2 || s.size() > 60) return false;
    for (char c : s) if (!isalpha((unsigned char)c) && c != ' ') return false;
    for (char c : s) if (isalpha((unsigned char)c)) return true;
    return false;
}
static bool validPhone(const string& p) {  // empty = not given; else exactly 10 digits
    if (p.empty()) return true;
    if (p.size() != 10) return false;
    for (char c : p) if (!isdigit((unsigned char)c)) return false;
    return true;
}
static bool validDate(const string& d) {   // YYYY-MM-DD, real calendar date
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit((unsigned char)d[i])) return false;
    }
    int y = atoi(d.substr(0,4).c_str()), m = atoi(d.substr(5,2).c_str()), day = atoi(d.substr(8,2).c_str());
    if (y < 2000 || y > 2100 || m < 1 || m > 12 || day < 1) return false;
    int dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) dim[2] = 29;
    return day <= dim[m];
}
// Minimal query-string parsing (?a=b&c=d), with URL decoding. Under 30 lines.
static string urlDecode(const string& s) {
    string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            out += (char)stoi(s.substr(i + 1, 2), nullptr, 16); i += 2;
        } else if (s[i] == '+') out += ' ';
        else out += s[i];
    }
    return out;
}
static map<string,string> parseQuery(const string& qs) {
    map<string,string> q;
    size_t i = 0;
    while (i < qs.size()) {
        size_t e = qs.find('=', i), a = qs.find('&', i);
        if (a == string::npos) a = qs.size();
        string k = urlDecode(qs.substr(i, (e == string::npos || e > a ? a : e) - i));
        string v = (e != string::npos && e < a) ? urlDecode(qs.substr(e + 1, a - e - 1)) : "";
        if (!k.empty()) q[k] = v;
        i = a + 1;
    }
    return q;
}
static string getQueryParam(const map<string,string>& q, const string& name) {
    auto it = q.find(name);
    return it == q.end() ? "" : it->second;
}

class FoodRescueApi {
    UserManager um;
    FoodManager fm;
    RequestManager rm;
    MatchingEngine me;
    DeliveryManager dm;
    AuthManager auth;
    Graph city;  // city map; seeded once
    vector<UndoAction> undoStack;  // DSA: undo history (in-memory)
    vector<UndoAction> redoStack;  // DSA: redo history (in-memory)
    ActivityLog activity;          // circular buffer; bookkeeping, exempt from undo
public:
    FoodRescueApi() : me(&fm, &rm), dm(&um, &fm, &rm) {
        city.seedDefaultMap();
        fm.markExpired(todayDate());
    }
    // Location guard: address/location fields must be a known map location.
    // Stores the canonical (properly cased) name. Returns "" when valid.
    string checkLocation(const string& v) {
        string canon = city.canonicalName(v);
        return canon.empty() ? "Unknown location. Choose one from the list." : "";
    }

    // A brand-new action invalidates anything that was undone before it.
    void recordUndo(const string& kind, int a, int b = -1, int c = -1, const string& prev = "") {
        redoStack.clear();
        pushUndo(undoStack, kind, a, b, c, prev);
    }

    // Returns {httpStatus, jsonBody}
    pair<int,string> handle(const string& method, const string& path,
                            const map<string,string>& body,
                            const map<string,string>& headers) {
        // split path from query string
        string p = path;
        map<string,string> query;
        size_t q = p.find('?');
        if (q != string::npos) { query = parseQuery(p.substr(q + 1)); p = p.substr(0, q); }

        // split /api/<res>[/<id>[/<action>]]
        vector<string> seg;
        {
            size_t i = 0;
            while (i < p.size()) {
                while (i < p.size() && p[i] == '/') i++;
                size_t j = i;
                while (j < p.size() && p[j] != '/') j++;
                if (j > i) seg.push_back(p.substr(i, j - i));
                i = j;
            }
        }
        if (seg.empty() || seg[0] != "api") return {404, errJson("Not found")};
        string res = seg.size() > 1 ? seg[1] : "";
        int id = (seg.size() > 2) ? atoi(seg[2].c_str()) : -1;
        string action = seg.size() > 3 ? seg[3] : "";

        // ---- auth: every /api/* except /api/login needs a valid Bearer token ----
        string token;
        auto ah = headers.find("authorization");
        if (ah != headers.end()) {
            string v = ah->second;
            if (v.rfind("Bearer ", 0) == 0) token = v.substr(7);
        }
        if (res == "login" && method == "POST") {
            string tok = auth.login(jsonGet(body, "username"), jsonGet(body, "password"));
            if (tok.empty()) return {401, errJson("Invalid username or password.")};
            JsonObj o; o.add("ok", true); o.add("token", tok);
            o.add("username", jsonGet(body, "username"));
            return {200, o.str()};
        }
        if (!auth.valid(token)) return {401, errJson("Please log in.")};
        if (res == "logout" && method == "POST") {
            auth.logout(token);
            return {200, okJson("Logged out.")};
        }
        if (res == "me" && method == "GET") {
            JsonObj o; o.add("ok", true); o.add("username", auth.who(token));
            return {200, o.str()};
        }
        if (res == "users") {
            if (method == "GET") {
                JsonArr a;
                for (auto& u : auth.listUsers()) {
                    JsonObj o; o.add("username", u.username); o.add("role", u.role);
                    a.addRaw(o.str());
                }
                return {200, a.str()};
            }
            if (method == "POST") {
                string u = jsonGet(body, "username"), p = jsonGet(body, "password");
                if (u.empty() || p.empty()) return {400, errJson("Username and password required.")};
                if (!auth.addUser(u, p)) return {400, errJson("Username already exists.")};
                return {200, okJson("User added.")};
            }
        }

        if (res == "stats" && method == "GET") return {200, stats()};
        if (res == "locations" && method == "GET") {
            JsonArr a;
            for (auto& nm : city.getLocationNames()) {
                JsonObj o; o.add("id", city.findLocation(nm)); o.add("name", nm);
                a.addRaw(o.str());
            }
            JsonObj o; o.add("ok", true); o.addRaw("locations", a.str());
            return {200, o.str()};
        }
        if (res == "graph" && method == "GET") {
            JsonArr nodes, edges;
            for (auto& nm : city.getLocationNames()) {
                int id = city.findLocation(nm);
                JsonObj o; o.add("id", id); o.add("name", nm);
                o.add("x", city.getX(id)); o.add("y", city.getY(id));
                nodes.addRaw(o.str());
            }
            for (auto& nm : city.getLocationNames()) {  // each undirected edge once
                int id = city.findLocation(nm);
                for (auto& e : city.edgesFrom(id))
                    if (e.to > id - LOCATION_ID_BASE) {
                        JsonObj o; o.add("a", id); o.add("b", LOCATION_ID_BASE + e.to);
                        o.add("km", e.km);
                        edges.addRaw(o.str());
                    }
            }
            JsonObj o; o.add("ok", true);
            o.addRaw("nodes", nodes.str()); o.addRaw("edges", edges.str());
            return {200, o.str()};
        }
        if (res == "route" && method == "GET") {
            int a = city.findLocation(getQueryParam(query, "from"));
            int b = city.findLocation(getQueryParam(query, "to"));
            if (a == -1 || b == -1)
                return {400, errJson("Unknown location. Choose one from the list.")};
            vector<int> path;
            double km = city.shortestPath(a, b, path);
            if (km < 0) return {400, errJson("No route between those locations.")};
            JsonArr pa;
            for (int id : path) pa.addRaw(jq(city.locationName(id)));
            JsonObj o; o.add("ok", true); o.add("km", km); o.addRaw("path", pa.str());
            return {200, o.str()};
        }

        if (res == "undo" && method == "POST") {
            if (undoStack.empty()) return {200, okJson("Nothing to undo.")};
            UndoAction u = undoStack.back(); undoStack.pop_back();
            string msg = applyUndo(u, um, fm, rm, dm, &redoStack);
            if (msg.rfind("Undone:", 0) == 0) activity.push(msg);  // bookkeeping; exempt from undo
            return {200, okJson(msg)};
        }
        if (res == "redo" && method == "POST") {
            if (redoStack.empty()) return {200, okJson("Nothing to redo.")};
            UndoAction u = redoStack.back(); redoStack.pop_back();
            string msg = applyRedo(u, um, fm, rm, dm, undoStack);
            if (msg.rfind("Redone:", 0) == 0) activity.push(msg);  // bookkeeping; exempt from undo
            return {200, okJson(msg)};
        }
        if (res == "activity" && method == "GET") {
            JsonArr a;
            for (auto& s : activity.getAll()) a.addRaw(jq(s));
            JsonObj o; o.add("ok", true); o.addRaw("items", a.str());
            return {200, o.str()};
        }
        if (res == "demo" && seg.size() > 2 && seg[2] == "load" && method == "POST") {
            if (um.donorCount() > 0)
                return {400, errJson("Demo data can only be loaded into an empty system.")};
            seedDemoData(um, fm, rm, undoStack, redoStack);
            activity.push("Demo data loaded");
            return {200, okJson("Demo data loaded.")};
        }

        if (res == "donors") {
            if (method == "GET") return {200, listDonors()};
            if (method == "POST") return addDonor(body);
            if ((method == "PUT" || method == "PATCH") && id > 0) return updateDonor(id, body);
        }
        if (res == "recipients") {
            if (method == "GET") return {200, listRecipients()};
            if (method == "POST") return addRecipient(body);
            if ((method == "PUT" || method == "PATCH") && id > 0) return updateRecipient(id, body);
        }
        if (res == "volunteers") {
            if (method == "GET") return {200, listVolunteers()};
            if (method == "POST") return addVolunteer(body);
            if ((method == "PUT" || method == "PATCH") && id > 0) return updateVolunteer(id, body);
        }
        if (res == "foods") {
            if (method == "GET" && seg.size() > 2 && seg[2] == "sorted") return {200, listFoodsSorted()};
            if (method == "GET") return {200, listFoods()};
            if (method == "POST") return addFood(body);
            if (method == "DELETE" && id > 0) {
                if (fm.removeFood(id)) return {200, okJson("Food deleted.")};
                return {404, errJson("Food not found.")};
            }
        }
        if (res == "requests") {
            if (method == "GET") return {200, listRequests()};
            if (method == "POST" && action == "cancel" && id > 0) {
                Request* r = rm.findRequestByID(id);
                if (!r) return {404, errJson("Request not found.")};
                if (r->getStatus() != "Pending") return {400, errJson("Only Pending requests can be cancelled.")};
                rm.cancelRequest(id);
                recordUndo("cancel_request", id, -1, -1, "Pending");
                activity.push("Request #" + to_string(id) + " cancelled");
                return {200, okJson("Request cancelled.")};
            }
            if (method == "POST") return addRequest(body);
        }
        if (res == "match" && method == "POST" && seg.size() > 2) {
            if (seg[2] == "next") {
                Request* top = rm.getPendingTop();
                if (!top) return {200, errJson("No match found.")};
                int rid = top->getRequestID();
                int fid = me.matchNextPending();
                if (fid == -1) return {200, errJson("No match found.")};
                recordUndo("match", rid, fid);
                activity.push("Request #" + to_string(rid) + " matched to food #" + to_string(fid));
                return {200, okJson("Matched.", fid)};
            }
            if (seg[2] == "all") {
                int n = matchAll();
                if (n > 0) activity.push("Matched " + to_string(n) + " requests");
                JsonObj o; o.add("ok", true); o.add("matched", n);
                o.add("message", string("Matching complete."));
                return {200, o.str()};
            }
        }
        if (res == "deliveries") {
            if (method == "GET") return {200, listDeliveries()};
            if (method == "POST" && id == -1) {
                int rid = jsonGetInt(body, "requestId", -1);
                int did = dm.createDelivery(rid);
                if (did == -1) return {400, errJson("Could not create delivery. Request must be Matched with Reserved food.")};
                { Delivery* d = dm.findDeliveryByID(did);
                  recordUndo("create_delivery", did, d ? d->getVolunteerID() : -1, rid); }
                activity.push("Delivery #" + to_string(did) + " created");
                return {200, okJson("Delivery created.", did)};
            }
            if (method == "POST" && action == "pickup" && id > 0) {
                int vid = jsonGetInt(body, "volunteerId", -1);
                if (dm.markPickedUp(id, vid)) {
                    Delivery* d = dm.findDeliveryByID(id);
                    recordUndo("picked_up", id, d ? d->getFoodID() : -1);
                    activity.push("Delivery #" + to_string(id) + " picked up");
                    return {200, okJson("Marked picked up.")};
                }
                return {400, errJson("Cannot mark picked up. Check delivery/volunteer.")};
            }
            if (method == "POST" && action == "delivered" && id > 0) {
                int vid = jsonGetInt(body, "volunteerId", -1);
                if (dm.markDelivered(id, vid)) {
                    Delivery* d = dm.findDeliveryByID(id);
                    recordUndo("delivered", id, vid, d ? d->getRequestID() : -1);
                    activity.push("Delivery #" + to_string(id) + " delivered");
                    return {200, okJson("Marked delivered.")};
                }
                return {400, errJson("Cannot mark delivered. Must be PickedUp first.")};
            }
        }
        return {404, errJson("Unknown endpoint.")};
    }

private:
    string stats() {
        fm.markExpired(todayDate());
        JsonObj o;
        o.add("donors", um.donorCount());
        o.add("recipients", um.recipientCount());
        o.add("volunteers", um.volunteerCount());
        o.add("freeVolunteers", um.freeVolCount());
        o.add("foods", fm.foodCount());
        o.add("availableFoods", (int)fm.getAvailableFood().size());
        o.add("requests", rm.totalCount());
        o.add("pendingRequests", rm.pendingCount());
        o.add("deliveries", dm.deliveryCount());
        o.add("pendingDeliveries", dm.pendingDeliveryCount());
        return o.str();
    }
    string listDonors() {
        JsonArr a;
        for (auto& d : um.getDonors()) a.addRaw(donorToJson(d));
        return a.str();
    }
    string listRecipients() {
        JsonArr a;
        for (auto& r : um.getRecipients()) a.addRaw(recipientToJson(r));
        return a.str();
    }
    string listVolunteers() {
        JsonArr a;
        for (auto& v : um.getVolunteers()) a.addRaw(volunteerToJson(v));
        return a.str();
    }
    string listFoods() {
        fm.markExpired(todayDate());  // Phase 0: never show a stale Available label
        JsonArr a;
        for (auto& f : fm.getAllFood()) a.addRaw(foodToJson(f));
        return a.str();
    }
    string listFoodsSorted() {
        fm.markExpired(todayDate());
        JsonArr a;
        for (auto& f : fm.getFoodSortedByExpiry()) a.addRaw(foodToJson(f));
        return a.str();
    }
    string listRequests() {
        JsonArr a;
        for (auto& r : rm.getAllRequests()) a.addRaw(requestToJson(r));
        return a.str();
    }
    string listDeliveries() {
        JsonArr a;
        for (auto& d : dm.getDeliveries()) a.addRaw(deliveryToJson(d));
        return a.str();
    }

    pair<int,string> addDonor(const map<string,string>& b) {
        string name = jsonGet(b, "name"), phone = jsonGet(b, "phone");
        if (!validName(name)) return {400, errJson("Name: letters and spaces only (2-60 chars).")};
        if (!validPhone(phone)) return {400, errJson("Phone: exactly 10 digits.")};
        int id = um.addDonor(name, phone, jsonGet(b,"address"),
                             jsonGet(b,"org"), jsonGet(b,"category","Other"));
        recordUndo("add_donor", id);
        activity.push("Donor " + name + " registered (#" + to_string(id) + ")");
        return {200, okJson("Donor registered.", id)};
    }
    pair<int,string> updateDonor(int id, const map<string,string>& b) {
        Donor* d = um.findDonorByID(id);
        if (!d) return {404, errJson("Donor not found.")};
        if (b.count("name") && !validName(jsonGet(b,"name")))
            return {400, errJson("Name: letters and spaces only (2-60 chars).")};
        if (b.count("phone") && !validPhone(jsonGet(b,"phone")))
            return {400, errJson("Phone: exactly 10 digits.")};
        if (b.count("name")) d->setName(jsonGet(b,"name"));
        if (b.count("phone")) d->setPhone(jsonGet(b,"phone"));
        if (b.count("address")) d->setAddress(jsonGet(b,"address"));
        if (b.count("org")) d->setOrganizationName(jsonGet(b,"org"));
        if (b.count("category")) d->setDonorCategory(jsonGet(b,"category"));
        return {200, okJson("Donor updated.", id)};
    }
    pair<int,string> addRecipient(const map<string,string>& b) {
        string name = jsonGet(b, "name"), phone = jsonGet(b, "phone");
        if (!validName(name)) return {400, errJson("Name: letters and spaces only (2-60 chars).")};
        if (!validPhone(phone)) return {400, errJson("Phone: exactly 10 digits.")};
        int id = um.addRecipient(name, phone, jsonGet(b,"address"),
                                 jsonGet(b,"org"), jsonGet(b,"category","Other"));
        recordUndo("add_recipient", id);
        activity.push("Recipient " + name + " registered (#" + to_string(id) + ")");
        return {200, okJson("Recipient registered.", id)};
    }
    pair<int,string> updateRecipient(int id, const map<string,string>& b) {
        Recipient* r = um.findRecipientByID(id);
        if (!r) return {404, errJson("Recipient not found.")};
        if (b.count("name") && !validName(jsonGet(b,"name")))
            return {400, errJson("Name: letters and spaces only (2-60 chars).")};
        if (b.count("phone") && !validPhone(jsonGet(b,"phone")))
            return {400, errJson("Phone: exactly 10 digits.")};
        if (b.count("name")) r->setName(jsonGet(b,"name"));
        if (b.count("phone")) r->setPhone(jsonGet(b,"phone"));
        if (b.count("address")) r->setAddress(jsonGet(b,"address"));
        if (b.count("org")) r->setOrganizationName(jsonGet(b,"org"));
        if (b.count("category")) r->setRecipientCategory(jsonGet(b,"category"));
        return {200, okJson("Recipient updated.", id)};
    }
    pair<int,string> addVolunteer(const map<string,string>& b) {
        string name = jsonGet(b, "name"), phone = jsonGet(b, "phone");
        if (!validName(name)) return {400, errJson("Name: letters and spaces only (2-60 chars).")};
        if (!validPhone(phone)) return {400, errJson("Phone: exactly 10 digits.")};
        string vehicle = jsonGet(b, "vehicle", "Bike");
        if (vehicle != "Bike" && vehicle != "Car" && vehicle != "Van") vehicle = "Bike";
        string vaddr = city.canonicalName(jsonGet(b,"address"));
        if (vaddr.empty()) return {400, errJson("Unknown location. Choose one from the list.")};
        int id = um.addVolunteer(name, phone, vaddr, vehicle);
        recordUndo("add_volunteer", id);
        activity.push("Volunteer " + name + " registered (#" + to_string(id) + ")");
        return {200, okJson("Volunteer registered.", id)};
    }
    pair<int,string> updateVolunteer(int id, const map<string,string>& b) {
        Volunteer* v = um.findVolunteerByID(id);
        if (!v) return {404, errJson("Volunteer not found.")};
        if (b.count("name") && !validName(jsonGet(b,"name")))
            return {400, errJson("Name: letters and spaces only (2-60 chars).")};
        if (b.count("phone") && !validPhone(jsonGet(b,"phone")))
            return {400, errJson("Phone: exactly 10 digits.")};
        if (b.count("name")) v->setName(jsonGet(b,"name"));
        if (b.count("phone")) v->setPhone(jsonGet(b,"phone"));
        if (b.count("address")) {
            string canon = city.canonicalName(jsonGet(b,"address"));
            if (canon.empty()) return {400, errJson("Unknown location. Choose one from the list.")};
            v->setAddress(canon);
        }
        if (b.count("vehicle")) v->setVehicleType(jsonGet(b,"vehicle"));
        if (b.count("availability")) {
            if (!um.setVolunteerAvailability(id, jsonGet(b,"availability")))
                return {400, errJson("Cannot change availability while assigned.")};
        }
        return {200, okJson("Volunteer updated.", id)};
    }
    pair<int,string> addFood(const map<string,string>& b) {
        int donorId = jsonGetInt(b, "donorId", -1);
        if (!um.findDonorByID(donorId)) return {400, errJson("Donor not found.")};
        int qty = jsonGetInt(b, "qty", 0);
        if (qty <= 0) return {400, errJson("Quantity must be positive.")};
        string fname = jsonGet(b,"name"), fexp = jsonGet(b,"expiry","2099-12-31");
        if (!validName(fname)) return {400, errJson("Food name: letters and spaces only (2-60 chars).")};
        if (!validDate(fexp)) return {400, errJson("Expiry: use a real date as YYYY-MM-DD.")};
        string faddr = city.canonicalName(jsonGet(b,"address"));
        if (faddr.empty()) return {400, errJson("Unknown location. Choose one from the list.")};
        int id = fm.addFoodParams(donorId, fname, jsonGet(b,"type","Cooked"),
                                  qty, jsonGet(b,"unit","kg"),
                                  fexp, faddr);
        if (id == -1) return {400, errJson("Invalid food data.")};
        recordUndo("add_food", id);
        activity.push("Food '" + fname + "' listed (#" + to_string(id) + ")");
        return {200, okJson("Food listed.", id)};
    }
    pair<int,string> addRequest(const map<string,string>& b) {
        int recipientId = jsonGetInt(b, "recipientId", -1);
        if (!um.findRecipientByID(recipientId)) return {400, errJson("Recipient not found.")};
        int qty = jsonGetInt(b, "qty", 0);
        if (qty <= 0) return {400, errJson("Quantity must be positive.")};
        string rloc = city.canonicalName(jsonGet(b,"location"));
        if (rloc.empty()) return {400, errJson("Unknown location. Choose one from the list.")};
        int id = rm.createRequestParams(recipientId, jsonGet(b,"type","Cooked"), qty,
                                        jsonGet(b,"unit","kg"), rloc,
                                        jsonGetInt(b,"priority",1));
        if (id == -1) return {400, errJson("Invalid request data.")};
        recordUndo("add_request", id);
        activity.push("Request #" + to_string(id) + " created");
        return {200, okJson("Request created.", id)};
    }
    int matchAll() {
        fm.markExpired(todayDate());  // expired food must not be matched
        int matched = 0;
        auto snap = rm.getPendingHeapSnapshot();
        for (int rid : snap) {
            Request* req = rm.findRequestByID(rid);
            if (req && req->getStatus() == "Pending") {
                int fid = me.matchRequest(req);
                if (fid != -1) { matched++; recordUndo("match", rid, fid); }
            }
        }
        return matched;
    }
};
