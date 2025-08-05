
# Library Management CLI (C++)

A modern, user-friendly command-line tool to manage lendable assets (such as books and laptops), track users, issue and return items, and detect overdue loans. It’s built with clean architecture, uses SQLite for persistent storage, and applies SOLID principles and proven design patterns to ensure maintainability and scalability.

---

## Project Intent

This project started as a simple in-memory book tracker and gradually evolved into a more realistic and structured library management system. The goals include:

- Replacing in-memory data with persistent SQLite storage
- Supporting multiple asset types (beyond books)
- Handling workflows like issuing, returning, and tracking overdue items
- Providing a clean and contextual command-line interface
- Writing modular, testable, and well-structured C++ code
- Introducing login/registration flows with secure password hashing using libsodium
- Managing roles (User and Staff) with appropriate menu access and permissions

---

## Folder Structure

```
src/
├── models/             # Domain models: Asset, Book, Laptop, User
├── persistence/        # SQLite integration + repository layer
├── services/           # Business logic (LoanService, NotificationService)
├── ui/                 # CLI, Context handler, Help menu
├── util/               # Logger and shared utilities (incl. password hashing)
└── main.cpp            # Entry point

root/
├── context.txt         # Stores last-used asset/user IDs
├── library.db          # SQLite database file
├── CMakeLists.txt      # CMake build file
└── README.md           
```

---

## SOLID Principles Applied

- **Single Responsibility**: Each class handles one well-defined task—logic, data, or UI.
- **Open/Closed**: Core logic can be extended (e.g., new asset types) without being modified.
- **Liskov Substitution**: All asset types implement a common interface and are treated uniformly.
- **Interface Segregation**: Interfaces only expose methods that are actually used.
- **Dependency Inversion**: High-level services depend on abstractions, not concrete implementations.

---

## Design Patterns Used

- **Repository Pattern** – Separates business logic from database operations.
- **Strategy Pattern** – Pluggable notification mechanisms handled by `NotificationService`.
- **Facade Pattern** – Services offer simplified interfaces over complex logic.
- **Memento-style Context** – Remembers recently used asset/user IDs to enhance usability.
- **Command Parser** – CLI supports aliases and shortcuts for smoother interaction.

---

## Features

- Persistent storage using SQLite (schema auto-created)
- Secure password hashing with libsodium
- Support for multiple asset types (books, laptops, etc.)
- Role-based access control (Staff vs. Users)
- User management: register, login, search, and list users
- Issue and return functionality with confirmations
- Overdue detection with simulated notifications
- Helpful command-line interface with shortcuts and context recall
- Nicely formatted tables for display

---

## Build & Run

### Requirements

- C++17 compatible compiler
- CMake version 3.25 or higher
- SQLite3
- libsodium
- pkg-config (for locating libsodium)

### Build Instructions

```bash
brew install sqlite3 libsodium pkg-config
mkdir -p build
cd build
cmake ../src
cmake --build . --target app
```

Or, open the project in CLion and build using the GUI.

### Run

```bash
./app
```

---

## Menu Commands

```
[1]  Add Asset (Book/Laptop)
[2]  Add User
[3]  Issue Asset
[4]  Return Asset
[5]  List All Assets
[6]  Show Overdues
[7]  Search Asset by ID
[8]  Search User by ID
[9]  List All Users
[10] Exit

Shortcuts:
h / help
a / u / i / r / l / o / sa / su / lu / q
```

---

## Login & Registration

Upon first run, the app prompts for staff account creation. Subsequent users can register themselves with a password.

Passwords are hashed using `libsodium` and safely stored in the database.

To simulate login failures or bad passwords, try registering with one password and logging in with another.

---

## Overdue Notifications

Overdue checks are performed automatically when the application starts and can also be triggered manually using option [6].

Currently, an "Email Notification" is simulated for any asset issued more than 14 days ago.

To simulate an overdue case:

```bash
sqlite3 library.db "UPDATE loans SET issue_date = strftime('%s','now','-15 days') WHERE asset_id='1';"
```

Then restart the CLI.

---

## Example Flow

```
$ ./app

[1] Register new user
[2] Add Book
[3] Issue Book to User
[5] List Assets → shows status "Issued"
[6] Overdues → Simulate and display alerts
[4] Return Book → prompts confirmation
```

---

## Future Improvements

- Email or terminal notifications via cronjob
- Fine-grained permissions (e.g., read-only users)
- Admin dashboard GUI (Qt-based)
