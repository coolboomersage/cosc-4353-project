# COSC 4353 – Queue Management System

A C++ web server application built for COSC 4353 (Software Design) that allows users to join and manage queues, with an admin panel and account management.

---

## Features

- **Join Queue** – Users can browse and join available queues
- **Active Queues** – View currently active queues and their status
- **Calendar Page** – Schedule and view upcoming queue events
- **Admin Panel** – Manage queues and users from an administrative interface
- **Account Panel** – User account creation and management
- **SQLite Database** – Lightweight persistent storage via SQLite3

---

## Smart Feature: Dynamic Wait-Time Estimation

QueueSmart includes a dynamic wait-time estimation feature. Each service stores an estimated service time in the database. When a user joins a queue, the system uses the service’s estimated duration and the user’s queue position to calculate the expected wait time. Queue wait times are also recalculated when queue entries change, helping users and administrators see a more accurate estimate of how long each person may wait.

This feature improves the queue management process by giving users clearer expectations and helping administrators monitor service demand.

---

## Tech Stack

| Layer | Technology |
|---|---|
| Backend | C++ |
| Database | SQLite3 |
| Build System | CMake |
| CI/CD | GitHub Actions |

---

## Prerequisites

- **CMake** 3.15+
- **A C++17-compatible compiler** (GCC, Clang, or MSVC)
- **Make** or **Ninja**

---

## Building

```bash
# Clone the repository
git clone https://github.com/coolboomersage/cosc-4353-project.git
cd cosc-4353-project

# Configure and build
cmake -S . -B build
cmake --build build
```

---

## Running

```bash
./build/serverExe
```
or

```bash
./build/server
```

The server will start and serve the application pages from the `sub pages/` directory.

---

## Project Structure

```
cosc-4353-project/
├── .github/workflows/   # CI/CD pipelines
├── external/            # Third-party dependencies
├── serverExe/           # Server executable source
├── sub pages/           # Frontend HTML/web pages
├── main.cpp             # Application entry point
├── sqlite3_wrapper.c    # SQLite3 C wrapper
└── CMakeLists.txt       # Build configuration
```

---

## Contributing

This is a course project for COSC 4353. To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes (`git commit -m 'Add your feature'`)
4. Push to the branch (`git push origin feature/your-feature`)
5. Open a Pull Request

---

## License

This project was created for educational purposes as part of COSC 4353 at the University of Houston.
