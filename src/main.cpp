#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "nlohmann/json.hpp"
#include <pqxx/pqxx>

#include "requests/request_handler.h"
#include "database/database_connection_pool.h"
#include "config/config.h"

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;

using namespace std;

using json = nlohmann::json;

config g_cfg("config.ini");

std::string g_connectionString = "host=localhost port=54321 dbname=health user=postgres password =12345";



database_connection_pool g_database_pool(g_connectionString, 5); ///< Глобальный объект пула потоков



// Функция для обработки HTTP-запросов
void handle_request(http::request<http::string_body>& req, tcp::socket& socket)
{
    std::cout << "Request method is" << req.target() << std::endl;
    // Обрезка запроса до первого вопросительного знака, если он присутствует
    std::string target = req.target().to_string();
    if (const auto pos = target.find('?'); pos != std::string::npos) 
    {
        target.erase(pos);
    }
    http::status status = http::status::not_found;
    json answer;
    answer["status"] = "unknow request";
    // если метод POST то обрабатываем запрос
    if (req.method() == http::verb::post) 
    {
        // Обработка запроса
        request_handler handler(req.body());
        string sql_request = handler.prepare_database_request();
        answer = g_database_pool.execute_query_to_json(sql_request);
        status = http::status::ok;
    }
    http::response<http::string_body> res{status, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = answer.dump();
    res.prepare_payload();
    http::write(socket, res);
}

// Функция для обработки соединения
void do_session(tcp::socket socket) {
    try {
        // Буфер для запроса
        beast::flat_buffer buffer;
        // Парсер HTTP-запроса
        http::request<http::string_body> req;
        // Получение запроса
        std::cout << "void do_session(tcp::socket socket)" << std::endl;
        http::read(socket, buffer, req);
        // Обработка запроса
        handle_request(req, socket);
    } catch (const std::exception& e) {
        std::cerr << "Exception in session: " << e.what() << std::endl;
    }
}

/**
 * @brief Entry point
 *
 * Execution of the program
 * starts here.
 *
 * @param argc Number of arguments
 * @param argv List of arguments
 *
 * @return Program exit status
 */
int main() {
    try {
        // Создание io_context для асинхронных операций
        boost::asio::io_context io_context;
        // Создание acceptor для прослушивания порта 8080
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "Server started" << std::endl;
        while (true) {
            // Создание сокета
            tcp::socket socket(io_context);
            // Ожидание входящего соединения
            acceptor.accept(socket);
            // Обработка соединения в новом потоке
            std::thread(do_session, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl; //TODO LOG error
        return 1;
    }

    return 0;
}


/*
#include <boost/array.hpp>
#include <iostream>
#include <pqxx/pqxx>


using namespace std;
int main()
{
    std::string connectionString = "host=localhost port=54321 dbname=health user=postgres password =12345";

    try
    {
        pqxx::connection connectionObject(connectionString.c_str());

        pqxx::work worker(connectionObject);

        pqxx::result response = worker.exec("SELECT * FROM meals");

        for (size_t i = 0; i < response.size(); i++)
        {
            std::cout << "Id: " << response[i][0].c_str() << " Username: " << response[i][1].c_str() << " Password: " << response[i][2].c_str() << " Email: " << response[i][3].c_str() << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    // system("pause");
    return 0;
}
*/