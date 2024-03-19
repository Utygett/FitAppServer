#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "nlohmann/json.hpp"
#include <pqxx/pqxx>

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;

using json = nlohmann::json;

std::string g_connectionString = "host=docker-potgresql-db-1 port=5432 dbname=health user=postgres password =12345";

// Функция для выполнения запроса к базе данных и преобразования результата в JSON
nlohmann::json execute_query_to_json(pqxx::connection& conn, const std::string& sql_query) {
    try {
        pqxx::work txn(conn); // Начало транзакции
        pqxx::result res = txn.exec(sql_query); // Выполнение запроса
        txn.commit(); // Фиксация транзакции

        // Преобразование результата в JSON
        json result_json;
        for (const auto& row : res) {
            json row_json;
            for (const auto& field : row) {
                row_json[field.name()] = field.c_str(); // Используйте field.c_str() для получения значения поля
            }
            result_json.push_back(row_json);
        }

        // Преобразование JSON в строку
        std::cout << "Response JSON DATA" << result_json.dump(4) << std::endl; 
        return std::move(result_json);
    } catch (const std::exception& e) {
        std::cerr << "Error executing query: " << e.what() << std::endl;
        return ""; // Возвращаем пустую строку в случае ошибки
    }
}

nlohmann::json getDishesByfilterFromDB(const std::string &filter) {

    std::stringstream ssRequest; 
    ssRequest << "SELECT dish_id, dish_name, dish_kkal "
                << "FROM dishes "
                << "WHERE dish_name ILIKE '%" << filter << "%'";
    try
    {
        pqxx::connection connectionObject(g_connectionString.c_str());

        return std::move(execute_query_to_json(connectionObject, ssRequest.str()));
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return "";
}

void filter_by_dish(http::request<http::string_body>& req, tcp::socket& socket) {
   // Здесь должен быть ваш код для поиска блюд по filter
    std::cout << "void handle_request(http::request<http::string_body>& req, tcp::socket& socket) {" << std::endl;
    // Получаем данные из тела запроса
    std::string json_data = req.body();
    // Здесь вы можете разобрать JSON и выполнить необходимую обработку
    std::cout << "Received JSON data: " << json_data << std::endl;
    nlohmann::json jRequestData = json::parse(json_data);
    // Возвращаем результат в формате JSON
    nlohmann::json found_dishes = getDishesByfilterFromDB(jRequestData["filter"]);
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = found_dishes.dump();
    res.prepare_payload();
    http::write(socket, res);
}

// Функция для обработки HTTP-запросов
void handle_request(http::request<http::string_body>& req, tcp::socket& socket) {
    std::cout << "Request method is" << req.target() << std::endl;
    // Обрезка запроса до первого вопросительного знака, если он присутствует
    std::string target = req.target().to_string();
    if (const auto pos = target.find('?'); pos != std::string::npos) {
        target.erase(pos);
    }
    // Извлечение параметра dish_name_filter из запроса
    if (req.method() == http::verb::post && target == "/find_dish_by_name_filter") {
        // Обработка запроса для маршрута /find_dish_by_name_filter
        filter_by_dish(req, socket);
    } else {
        // В случае, если маршрут не совпадает с ожидаемым, можно отправить ответ с ошибкой 404 Not Found
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, "MyServer");
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = "Error 404: Not Found";
        res.prepare_payload();
        http::write(socket, res);
    }
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
        std::cerr << "Exception: " << e.what() << std::endl;
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