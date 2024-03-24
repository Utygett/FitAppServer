#pragma once

#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "nlohmann/json.hpp"


/**
 * @brief Класс для управления пулом соединений с базой данных.
 * 
 * Позволяет создавать соединения с базой данных, выполнять запросы и возвращать соединения обратно в пул.
 */
class database_connection_pool {
private:
    std::string m_connectionString;                 ///< Строка подключения к базе данных.
    std::queue<pqxx::connection*> m_connections;    ///< Очередь соединений.
    std::mutex m_mutex;                             ///< Мьютекс для синхронизации доступа к пулу соединений.
    std::condition_variable m_cv;                   ///< Условная переменная для ожидания доступного соединения.
    bool m_stop;                                    ///< Флаг для остановки работы пула.

public:
    database_connection_pool(const std::string& connectionString, size_t poolSize);
    ~database_connection_pool();
    nlohmann::json execute_query_to_json(const std::string& sql_query);           
};