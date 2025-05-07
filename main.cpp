#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class Book {
public:
    string id, title, author;
    bool isIssued;

    Book() : isIssued(false) {};
    Book(string i, string t, string a, bool issue): id(i), title(t), author(a), isIssued(issue) {};

    string toString() const {
        return id + "|" + title + "|" + author + "|" + (isIssued ? "1" : "0");
    }

    static Book fromString(const string &line) {
        stringstream ss(line);
        string id, title, author, issuedStr;
        getline(ss, id, '|');
        getline(ss, title, '|');
        getline(ss, author, '|');
        getline(ss, issuedStr);

        return Book(id, title, author, issuedStr == "1");
    }

    void display() const {
        cout << "\nBook ID: " << id
        << "\nTitle: " << title
        << "\nAuthor: " << author
        << "\nStatus: " << (isIssued ? "Issued" : "Available") << endl;
    }
};

class LibraryManagement {
private:
    vector<Book> books;
    const string filename = "books.txt";

    void loadBooks() {
        books.clear();
        ifstream in(filename);
        string line;
        while (getline(in, line)) {
            if (!line.empty()) {
                books.push_back(Book::fromString(line));
            }
        }
        in.close();
    }

    void saveBooks() {
        ofstream out(filename);
        for (auto &book : books) {
            out << book.toString() << endl;
        }
    }

    public:
    LibraryManagement() {
        loadBooks();
    }

    void addBook() {
        string id, title, author;
        cout << "Enter book ID: ";
        cin >> id;
        cin.ignore();
        cout << "Enter title: ";
        getline(cin, title);
        cout << "Enter author: ";
        getline(cin, author);

        for (auto &book : books) {
            if (book.id == id) {
                cout << "Book already exists" << endl;
                return;
            }
        }

        books.emplace_back(id, title, author, false);
        saveBooks();
        cout << "Book added successfully" << endl;
    }

    void showAllBooks() {
        if (books.empty()) {
            cout << "No books found" << endl;
            return;
        }
        for (auto &book : books) {
            book.display();
        }
    }

    void seachBook() {
        string id;
        cout << "Enter book ID: ";
        cin >> id;
        bool found = false;
        for (auto &book : books) {
            if (book.id == id) {
                book.display();
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "Book not found" <<endl;
        }
    }

    void issueBook() {
        string id;
        cout << "Enter book ID: ";
        cin >> id;
        for (auto &book : books) {
            if (book.id == id) {
                if (book.isIssued) {
                    cout << "Book is already issued" << endl;
                }
                else {
                    book.isIssued = true;
                    saveBooks();
                    cout << "Book issued" << endl;
                }
                return;
            }
        }
        cout << "Book not found" <<endl;
    }

    void returnBook() {
        string id;
        cout << "Enter book ID to return: ";
        cin >> id;
        for (auto &book : books) {
            if (book.id == id) {
                if (!book.isIssued) {
                    cout << "Book is already available" << endl;
                }
                else {
                    book.isIssued = false;
                    saveBooks();
                    cout << "Book returned" << endl;
                }
                return;
            }
        }
        cout << "Book not found" <<endl;
    }

    void menu() {
        LibraryManagement lib;
        int choice;
        do {
            cout << "\n=== Library Management System ===\n";
            cout << "1. Add Book\n";
            cout << "2. Search Book\n";
            cout << "3. Display All Books\n";
            cout << "4. Issue Books\n";
            cout << "5. Return Book\n";
            cout << "6. Exit\n";
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
                case 1: lib.addBook(); break;
                case 2: lib.seachBook(); break;
                case 3: lib.showAllBooks(); break;
                case 4: lib.issueBook(); break;
                case 5: lib.returnBook(); break;
                case 6: lib.menu(); break;
                default: cout << "Invalid choice" << endl;
            }
        } while (choice != 6);
    }
};


int main() {
    LibraryManagement lib;
    lib.menu();
}