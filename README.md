# FoodRescue

**Save Food. Serve People.**

FoodRescue is a food-waste rescue system written in C++17. Donors list surplus
food, recipients (NGOs, shelters) post requests for what they need, and
volunteers deliver matched donations. The system matches requests to the best
available donation using real road distances, priorities and dietary rules.

Built as a Project-Based Learning (PBL) project to demonstrate Data Structures,
Algorithms and Object-Oriented Programming in one working application — a
terminal menu and a web dashboard served by an HTTP server written from
scratch in C++ (no frameworks, no libraries).

## Features

- Register donors, recipients and volunteers
- List food donations with expiry dates and per-type shelf-life limits
  (Cooked: 2 days, Raw/Produce: 7 days, Packaged: 365 days)
- Smart matching: hard filters (type, unit, quantity, veg-only, allergens)
  followed by a weighted score — 45% distance, 35% expiry urgency, 20% fit —
  with a human-readable reason for every match
- Requests are prioritised in a heap with priority aging, so old requests
  never starve
- Deliveries with automatic volunteer assignment (FIFO free-volunteer queue)
- Full undo/redo for every action
- Live dashboard with stats, expiring-soon list and an activity feed
- One-click demo data: 3 donors, 3 recipients, 4 volunteers, 8 donations,
  4 requests in a 10-location city map

## Tech Stack

- **Language:** C++17 (single translation unit, no header files)
- **Server:** hand-written POSIX socket HTTP server on port 8080
- **Frontend:** one HTML file with vanilla JavaScript (no build step)
- **Storage:** in-memory only — no database, by design (see below)

## Getting Started

### Build

```bash
g++ -std=c++17 -Wall -Wextra main.cpp -o FoodRescue
```

Only `main.cpp` is compiled — it `#include`s every other `.cpp` file in the
project, so there is nothing else to link.

### Run

```bash
./FoodRescue            # web dashboard at http://localhost:8080
./FoodRescue --cli      # terminal menu
./FoodRescue --test     # built-in self-tests (51 checks)
```

Web login: **admin / admin123**

On the dashboard, click **Load Demo Data** to fill the system with example
data, then try **Match All** on the Requests page.

## How Matching Works

1. A request enters the pending heap (base priority 1–5, growing +1 every
   10 minutes it waits, capped at 10).
2. Hard filters remove donations that are expired, the wrong type or unit,
   too small, non-veg for a veg-only request, or carrying an avoided allergen.
3. Survivors are scored:
   `score = 0.45 × nearness + 0.35 × urgency + 0.20 × fit`, where nearness
   comes from the shortest **road** distance (Dijkstra on the city graph —
   e.g. Riverside → Central Market is 10 km direct but 7 km via Old Town).
4. The best donation is Reserved, the request becomes Matched, and a
   delivery can be created.

## Project Structure

```
main.cpp            Entry point, CLI menu, undo/redo core, self-tests, demo data
Util.cpp            Date/time helpers (dayNumber, daysUntil), string helpers
IntQueue.cpp        FIFO queue of ints (linked list)
HashTable.cpp       String->int hash table (djb2, separate chaining)
MinHeap.cpp         Binary min-heap of (key, id) pairs
ActivityLog.cpp     Circular buffer (50) of recent activity
Graph.cpp           City map: locations, roads, Dijkstra shortest paths
User.cpp            Base class for all users (virtual displayProfile/getRole)
Donor.cpp           Donor        (inherits User)
Recipient.cpp       Recipient    (inherits User)
Volunteer.cpp       Volunteer    (inherits User)
UserManager.cpp     All users + free-volunteer queue, binary search templates
Food.cpp            Food donation entity
LinkedList.cpp      FoodNode for the donation linked list
FoodManager.cpp     Donations: linked list, expiry heap, shelf life, merge sort
Request.cpp         Request entity + effectivePriority() aging
RequestHeap.cpp     Binary max-heap of pending requests
RequestManager.cpp  All requests, pending heap, counting sort by priority
Delivery.cpp        Delivery entity
MatchingEngine.cpp  Scored matching (filters + weighted score + reasons)
DeliveryManager.cpp Deliveries, volunteer assignment, pending-delivery queue
Json.cpp            Minimal JSON builder/parser
Auth.cpp            Accounts and session tokens (hash-table indexed)
Api.cpp             REST API: routing, validation, JSON responses
HttpServer.cpp      Raw-socket HTTP server, serves web/ and /api/*
web/index.html      Dashboard (single page, vanilla JS)
```

## Data Structures Used

| Structure | Where |
|---|---|
| Singly linked list | Food donations (FoodManager) |
| Binary max-heap | Pending requests by effective priority |
| Binary min-heap | Dijkstra priority queue; expiry tracking |
| FIFO queue | Free volunteers; pending deliveries |
| Hash table (chaining) | Username lookup; location name → graph node |
| Graph + Dijkstra | City map distances and routes |
| Merge sort | Donations sorted by expiry |
| Counting sort | Requests sorted by priority (keys 1–5) |
| Stack | Undo/redo history |
| Circular buffer | Activity log (last 50 events) |

## API Overview

All `/api/*` endpoints (except login) need `Authorization: Bearer <token>`.

```
POST /api/login                 GET  /api/stats
GET|POST /api/donors            GET|POST /api/foods
GET|POST /api/recipients        GET  /api/foods/sorted      (merge sort)
GET|POST /api/volunteers        GET  /api/foods/expiring?days=N
POST /api/match/next            GET|POST /api/requests
POST /api/match/all             GET  /api/requests/sorted   (counting sort)
POST /api/deliveries            POST /api/undo · /api/redo
POST /api/deliveries/:id/pickup GET  /api/route?from=A&to=B (Dijkstra)
POST /api/deliveries/:id/delivered
```

## Design Notes

- **In-memory by design.** The project is graded on the data structures
  themselves; a database would hide them. Restarting starts empty.
- **Validation lives in the API layer** (names, 10-digit phones, real
  calendar dates, allergen list, shelf life, known locations). The web form
  only mirrors those rules.
- **Undo stores inverse actions.** Undoing a status change offers redo;
  undoing a registration does not (re-adding would create a new ID).

## Status & Roadmap

- ✅ Core system, web dashboard, scored matching, undo/redo — complete
- 🚧 In progress: delivery timestamps, scored volunteer picker (vehicle
  capacity), delivery OTP, failed-delivery flow, pickup time windows
- 📋 Planned: feedback & ratings with leaderboard, trie-based search
  suggestions, notifications, impact analytics, visual map page

## Author

**Hardik Raturi** — BTech (Cyber), Project-Based Learning project.
