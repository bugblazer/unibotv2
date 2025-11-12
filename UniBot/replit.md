# UniBot - FAQ Chatbot

## Overview
UniBot is a university FAQ chatbot application with a C++ backend using the Crow web framework and a static HTML/CSS/JS frontend. The chatbot uses keyword matching to answer frequently asked questions about university admissions, facilities, and services.

## Project Structure
```
.
├── backend/
│   ├── data/
│   │   ├── admin.txt          # Admin credentials (username and password)
│   │   └── faqs.txt            # FAQ database (questions, answers, keywords)
│   ├── main.cpp                # C++ backend server (Crow framework)
│   └── unibot                  # Compiled backend binary
├── webgui/
│   ├── index.html              # Main chatbot interface
│   ├── script.js               # Chatbot frontend logic
│   ├── style.css               # Chatbot styling
│   ├── admin.html              # Admin panel interface
│   ├── adminScript.js          # Admin panel logic
│   └── adminStyle.css          # Admin panel styling
├── crow/
│   └── crow.h                  # Crow web framework header
├── compile.sh                  # Compilation script for C++ backend
└── run.sh                      # Startup script (backend + frontend)
```

## Technology Stack
- **Backend**: C++ with Crow web framework
- **Frontend**: Vanilla HTML/CSS/JavaScript
- **Web Server**: Python's http.server for static files
- **Database**: Plain text file storage (faqs.txt)

## Architecture
- Backend runs on `127.0.0.1:18080` (internal, not exposed)
- Python proxy server on `0.0.0.0:5000` (exposed via Replit)
  - Serves static frontend files from `webgui/` directory
  - Proxies API requests from `/api/*` to backend at `127.0.0.1:18080/*`
- CORS handled by both Crow middleware and Python proxy
- RESTful API for chatbot and admin operations

## API Endpoints

### Public Endpoints
- `POST /ask` - Submit a question and get an answer

### Admin Endpoints
- `POST /admin/login` - Admin login
- `GET /admin/faqs` - Get all FAQs
- `POST /admin/faqs` - Add a new FAQ
- `PUT /admin/faqs/<id>` - Update an FAQ
- `DELETE /admin/faqs/<id>` - Delete an FAQ

## Admin Credentials
- Username: admin
- Password: admin123
- Stored in: `backend/data/admin.txt`

## How It Works
1. User submits a question via the frontend
2. Frontend sends POST request to `/ask` endpoint
3. Backend processes the question using improved keyword matching:
   - Tokenizes user input into individual words
   - Scores each FAQ using a multi-tier matching system:
     * **Exact word matches** (highest score: 3.0 points)
     * **Stemmed matches** (medium score: 2.0 points) - handles plurals, -ing, -ed, -ly suffixes
     * **Partial substring matches** (lowest score: 0.5 points)
   - Normalizes score by keyword match percentage
   - Applies 1.2x bonus for matching multiple keywords
   - Requires minimum 30% match threshold to return a result
4. Best matching FAQ is returned based on weighted score
5. If no match meets threshold, default contact message is returned

### Matching Algorithm Improvements (November 12, 2025)
The keyword matching algorithm has been significantly improved with:
- **Word boundary detection**: Prevents false matches (e.g., "apply" won't match "reapply")
- **Basic stemming**: Handles common word variations:
  - Plurals: "loans" matches "loan"
  - Gerunds: "applying" matches "apply"
  - Past tense: "applied" matches "apply"
  - Adverbs: "quickly" matches "quick"
  - Preserves double-s words: "class" stays "class"
- **Intelligent scoring**: Prioritizes exact matches over partial matches
- **Match threshold**: Prevents poor matches (requires 30% keyword overlap)
- **Multi-keyword bonus**: Rewards queries matching multiple keywords

## Development Setup
1. Backend is compiled using `compile.sh`
2. Application starts with `run.sh` which:
   - Compiles backend if needed
   - Starts C++ backend server on 127.0.0.1:18080
   - Starts Python proxy server (server.py) on 0.0.0.0:5000
3. Frontend accessible via Replit webview on port 5000
4. API requests from browser go to `/api/*` and are proxied to backend

## Deployment Configuration
- **Type**: VM (always-on)
- **Build**: Compiles C++ backend
- **Run**: Starts both backend and frontend servers
- **Port**: 5000 (frontend webview)

## Recent Changes (November 12, 2025)
- Migrated project to Replit environment
- Updated backend to use Crow v1.2.0
- Added CORS support using Crow's CORSHandler middleware
- Combined duplicate HTTP method handlers for routes (GET/POST and PUT/DELETE)
- Created compile and run scripts for easy deployment
- Configured workflow to serve frontend on port 5000
- Backend binds to 127.0.0.1:18080 for security (internal only)
- Created Python proxy server (server.py) that:
  - Serves static frontend files from webgui/ directory
  - Proxies API requests from /api/* to backend
  - Handles CORS for external browser connections
- Updated frontend to use relative API paths (/api/*)

## User Preferences
- Language: C++ for backend performance
- Minimal dependencies (header-only Crow framework)
- Simple text-based storage for FAQ data
- Clean, modern UI with light/dark theme support

## Notes
- The project uses keyword-based matching (not AI/ML)
- FAQ data persists in plain text files
- Admin panel allows CRUD operations on FAQs
- 25 pre-configured FAQs about university services
