#pragma once
#include <string>
#include <stdexcept>
#include <sqlite3.h>

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
