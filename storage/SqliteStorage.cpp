#include "storage/SqliteStorage.h"
#include <fstream>
#include <sstream>

SqliteStorage::SqliteStorage(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("sqlite3_open failed");
    }
    exec("PRAGMA foreign_keys = ON;");
}

SqliteStorage::~SqliteStorage() {
    if (db_) sqlite3_close(db_);
}

void SqliteStorage::exec(const char* sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("sqlite3_exec: " + msg);
    }
}

void SqliteStorage::begin() { exec("BEGIN IMMEDIATE;"); }
void SqliteStorage::commit() { exec("COMMIT;"); }
void SqliteStorage::rollback() { exec("ROLLBACK;"); }

void SqliteStorage::applyMigrationFile(const std::string& sqlPath) {
    std::ifstream f(sqlPath);
    if (!f) throw std::runtime_error("Migration file not found: " + sqlPath);
    std::ostringstream ss; ss << f.rdbuf();
    exec(ss.str().c_str());
}
