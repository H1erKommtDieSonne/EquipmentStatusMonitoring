#pragma once
#include <string>
#include <stdexcept>

/** @cond SQLITE_INCLUDE */
#if __has_include(<sqlite3.h>)
  // Системный путь (vcpkg/SDK)
#include <sqlite3.h>
#elif __has_include("sqlite3.h")
  // Локальная копия (third_party/sqlite/sqlite3.h) попадает через Additional Include Dirs
#include "sqlite3.h"
#elif __has_include("../third_party/sqlite/sqlite3.h")
#include "../third_party/sqlite/sqlite3.h"
#else
#error "sqlite3.h not found. Add it to third_party/sqlite or install via vcpkg."
#endif
/** @endcond */

/**
 * @class SqliteStorage
 * @brief обёртка соединения с SQLite
 * Обеспечивает открытие/закрытие БД, транзакции и выполнение SQL
 */
class SqliteStorage {
public:
    /**
     * @brief Открыть/создать БД по пути @p path
     * @param path Путь к файлу БД
     */
    explicit SqliteStorage(const std::string& path);

    /// @brief Деструктор, закрывает соединение
    ~SqliteStorage();

    SqliteStorage(const SqliteStorage&) = delete;
    SqliteStorage& operator=(const SqliteStorage&) = delete;

    /// @brief Доступ к сырому указателю sqlite3*
    sqlite3* handle() noexcept { return db_; }

    /**
     * @brief Выполнить SQL без выборки результатов
     * @param sql Текст SQL-запроса
     */
    void exec(const char* sql);

    /// @brief Начать транзакцию
    void begin();

    /// @brief Зафиксировать транзакцию
    void commit();

    /// @brief Откатить транзакцию
    void rollback();

    /**
     * @brief Применить SQL-миграцию из файла
     * @param sqlPath Путь к .sql файлу
     */
    void applyMigrationFile(const std::string& sqlPath);

private:
    sqlite3* db_{ nullptr };
};
