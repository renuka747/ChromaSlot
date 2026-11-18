# ChromaSlot

ChromaSlot is a deterministic, constraint-aware university timetable scheduler built with a **C11 graph-coloring and backtracking engine**, **SQLite** persistence, and a **React + TypeScript + Vite** multi-role web interface (Admin, Faculty, and Student portals).

## Key Features

- **Graph Coloring & Backtracking Solver (`backend/src/`)**: Models sessions as vertices and resource/faculty/division conflicts as edges in an adjacency list, solving with degree heuristics and backtracking repair.
- **64-Bit Bitmask Availability (`avail.c`)**: Encodes all 35 weekly teaching slots (Mon–Fri, 7 periods/day, lunch 13:00–14:00 excluded) in a `uint64_t` bitmask for O(1) faculty unavailability and overlap checks.
- **Automatic Capacity Batching (`preflight.c`)**: Automatically splits oversized lab/class sessions into safe batches (`-B1`, `-B2`, …) whenever student count exceeds available room capacity.
- **Consecutive Hour Limits & Fixed Slots**: Enforces maximum consecutive teaching hours per faculty and fixed-slot assignments, explicitly flagging mathematically unsatisfiable constraints instead of silently violating them.
- **Multi-Role Web UI (`frontend/`)**:
  - **Administrator**: Load JSON datasets, generate timetables, inspect live conflict health checks, review/resolve complaints, and approve proposed repairs.
  - **Faculty**: View assigned sessions, mark unavailable slots to queue repairs, and submit complaints.
  - **Student**: View division schedules and submit targeted shift/gap complaints by day and period.

## Project Structure

```text
ChromaSlot/
├── backend/
│   ├── Makefile             # Build targets for server and unit/regression tests
│   ├── include/             # C11 header files
│   ├── src/                 # Core DSA engine, SQLite persistence, Mongoose HTTP server
│   ├── tests/               # Unit and regression test suite + sample test data
│   ├── data/                # Sample JSON dataset (sample_dataset.json)
│   └── vendor/              # cJSON and Mongoose third-party libraries
└── frontend/
    ├── src/                 # React + TypeScript application (App.tsx, index.css)
    ├── index.html
    ├── package.json
    └── vite.config.ts
```

## Prerequisites

- **Backend**: `gcc` (C11) and `sqlite3` development headers (`libsqlite3`)
- **Frontend**: Node.js (v18+) and `npm`

## Quick Start

### 1. Build & Run the C11 Backend
```bash
cd backend
make build/server
./build/server 8000
```

### 2. Run the React Frontend
In a separate terminal:
```bash
cd frontend
npm install
npm run dev
```
Open the URL printed by Vite (default `http://localhost:8443`). You can load `backend/data/sample_dataset.json` from **Admin → Sessions → Load Dataset**.

### 3. Run the C11 Test Suite
```bash
cd backend
make test
```
