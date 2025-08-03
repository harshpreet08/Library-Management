#include "Book.h"

Book::Book(std::string id, std::string title, std::string author)
    : _id(std::move(id)), _title(std::move(title)), _author(std::move(author)) {}

std::string Book::id() const { return _id; }
bool Book::isIssued() const { return _issued; }
void Book::setIssued(bool issued) { _issued = issued; }

std::string Book::title() const { return _title; }
std::string Book::author() const { return _author; }