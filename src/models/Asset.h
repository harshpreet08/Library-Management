#pragma once
#include "ILendable.h"
#include <string>

enum class AssetType {
    Book,
    Laptop,
    Unknown
};

inline std::string assetTypeToString(AssetType t) {
    switch (t) {
        case AssetType::Book: return "book";
        case AssetType::Laptop: return "laptop";
        default: return "unknown";
    }
}

inline AssetType stringToAssetType(const std::string& s) {
    if (s == "book") return AssetType::Book;
    if (s == "laptop") return AssetType::Laptop;
    return AssetType::Unknown;
}

class Asset : public ILendable {
public:
    Asset(std::string id, AssetType type, std::string title, std::string authorOrOwner);

    std::string id() const override;
    bool isIssued() const override;
    void setIssued(bool issued) override;

    AssetType type() const;
    std::string title() const;
    std::string authorOrOwner() const;

private:
    std::string _id;
    AssetType _type;
    std::string _title;
    std::string _authorOrOwner;
    bool _issued = false;
};