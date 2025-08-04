#include "Asset.h"

Asset::Asset(std::string id, AssetType type, std::string title, std::string authorOrOwner)
    : _id(std::move(id)), _type(type), _title(std::move(title)), _authorOrOwner(std::move(authorOrOwner)) {}

std::string Asset::id() const { return _id; }
bool Asset::isIssued() const { return _issued; }
void Asset::setIssued(bool issued) { _issued = issued; }
AssetType Asset::type() const { return _type; }
std::string Asset::title() const { return _title; }
std::string Asset::authorOrOwner() const { return _authorOrOwner; }