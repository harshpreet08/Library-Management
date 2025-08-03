#pragma once
#include "../models/Book.h"
#include "DatabaseManager.h"
#include <memory>
#include <vector>

class BookRepository {
public:
    explicit BookRepository(std::shared_ptr<DatabaseManager> db);
    void add(const Book& book);
    std::unique_ptr<Book> find(const std::string& id);
    std::vector<Book> getAll();
    void setIssued(const std::string& id, bool issued);
    bool isIssued(const std::string& id);

    std::shared_ptr<DatabaseManager> getDb() const { return _db; }

private:
    std::shared_ptr<DatabaseManager> _db;
};
