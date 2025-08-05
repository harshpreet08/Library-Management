#include "Security.h"
#include <sodium.h>
#include <stdexcept>

bool initCrypto() {
    return sodium_init() >= 0;
}

std::string hashPassword(const std::string& pwd) {
    char hash[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(
            hash, pwd.c_str(), pwd.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE
        ) != 0) {
        throw std::runtime_error("Password hashing failed");
        }
    return std::string(hash);
}

bool verifyPassword(const std::string& hash, const std::string& pwd) {
    return crypto_pwhash_str_verify(
        hash.c_str(), pwd.c_str(), pwd.size()
    ) == 0;
}