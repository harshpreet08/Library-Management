# Library Management System (C++)

This is a simple and beginner-friendly **Library Management System** built using **C++**. It allows users to manage a collection of books from the command line.

## Features

- Add a new book
- Search for a book b its ID
- Display all books in the system
- Issue a book
- Return a book
- Book data is saved to a file (`books.txt`) so that it persists between runs

## How It Works

- Book details are stored in a plain text file named `books.txt`
- Each book record includes:
    - Book ID
    - Title
    - Author
    - Issue status (Issued or Available)
- The program automatically loads existing books when it starts and saves all changes when you add, issue, or return a book

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/harshpreet08/Library-Management.git
cd Library-Management
```

### 2. Compile the Code
```bash
g++ main.cpp -o library_management
```
### 3. Run the Program
```bash
./library_management
```
### 4. Requirements
- C++ compiler (g++, clang++)
