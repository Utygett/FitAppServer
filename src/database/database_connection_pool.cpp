#include "database_connection_pool.h"

using json = nlohmann::json;

/**
 * @brief Конструктор класса database_connection_pool.
 * 
 * Создает пул соединений с базой данных заданного размера.
 * 
 * @param connectionString Строка подключения к базе данных.
 * @param poolSize Размер пула соединений.
 */
database_connection_pool::database_connection_pool(const std::string& connectionString, size_t poolSize)
    : m_connectionString(connectionString), m_stop(false) 
{
    for (size_t i = 0; i < poolSize; ++i) {
        m_connections.push(new pqxx::connection(m_connectionString));
    }
}

/**
 * @brief Деструктор класса database_connection_pool.
 * 
 * Освобождает ресурсы, занятые пулом соединений.
 */
database_connection_pool::~database_connection_pool() 
{
    m_stop = true;
    while (!m_connections.empty()) {
        delete m_connections.front();
        m_connections.pop();
    }
}

/**
 * @brief Выполняет SQL-запрос к базе данных и возвращает результат в формате JSON.
 * 
 * Если в пуле есть доступные соединения, они используются для выполнения запроса.
 * Если все соединения заняты, создается новое соединение.
 * 
 * @param sql_query SQL-запрос.
 * @return Результат выполнения запроса в формате JSON.
 */
nlohmann::json database_connection_pool::execute_query_to_json(const std::string& sql_query) 
{
    pqxx::connection* conn = nullptr;
    {
        // Захватываем мютекс длч получения свободного соединентя
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_connections.empty() && !m_stop) {
            // Освобождаем мьютекс перед вызовом wait
            lock.unlock();
            m_cv.wait(lock);
            // После пробуждения снова захватываем мьютекс
            lock.lock();
        }
        if (!m_stop) {
            conn = m_connections.front();
            m_connections.pop();
        }
    }

    if (!conn) {
        conn = new pqxx::connection(m_connectionString);
    }

    try {
        pqxx::work txn(*conn); // Начало транзакции
        pqxx::result res = txn.exec(sql_query); // Выполнение запроса
        txn.commit(); // Фиксация транзакции

        // Преобразование результата в JSON
        json result_json;
        for (const auto& row : res)
        {
            json row_json;
            for (const auto& field : row) {
                row_json[field.name()] = field.c_str(); // Используйте field.c_str() для получения значения поля
            }
            result_json.push_back(row_json);
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_connections.push(conn);
        }
        m_cv.notify_one();

#ifdef DEBUG
        // Преобразование JSON в строку
        std::cout << "Response JSON DATA" << result_json.dump(4) << std::endl;
#endif
        return std::move(result_json);
    } catch (const std::exception& e) {
        std::cerr << "Error executing query: " << e.what() << std::endl;  //TODO LOG ERROR
        delete conn;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_connections.push(new pqxx::connection(m_connectionString));
        }
        m_cv.notify_one();
        
        return {}; // Возвращаем пустой JSON в случае ошибки
    }
}