#include "request_handler.h"
#include <iostream>
using namespace std;
using json = nlohmann::json;

request_handler::request_map request_handler::s_request_ids = {
    {"get_dishes_by_filter", get_dish_by_filter}
};

/**
 * @brief Конструктор класса RequestHandler.
 * 
 * Конструктор принимает JSON-строку в качестве аргумента и парсит её.
 * В случае успешного парсинга JSON-строки, данные сохраняются в m_json_request.
 * В случае ошибки парсинга, статус устанавливается как "error".
 * 
 * @param request JSON-строка, содержащая данные запроса.
 */
request_handler::request_handler(const std::string &request)
{
    std::cout << "Received JSON data: " << request << std::endl;
    m_request_status = request_status::in_process;
    try {
        m_json_request = json::parse(request);
    }
    catch(const std::exception& e) {
        std::cerr << "request_handler::request_handler: " << e.what() << '\n'; //TODO LOG
        m_request_status = request_status::error;
        m_json_request["error"] = e.what();
    }
    
}

request_handler::~request_handler()
{
}

/**
 * @brief Получаем тип запроса из JSONa.
 * 
 * Производим поиск по карте запросов s_request_ids. 
 * 
 * @return Целочисленный идентификатор запроса.
 */
request_handler::request_type_id request_handler::get_request_type()
{    
    auto itRequst_id = s_request_ids.find(m_json_request["request_type"]);
    if(itRequst_id == s_request_ids.end())
        return unkwon_request;
    return itRequst_id->second;
}

/**
 * @brief Подготавливает данные для sql на основе JSON-запроса.
 * 
 * @return Строку SQL запроса к базе данных.
 */
string request_handler::prepare_database_request()
{
    if(m_request_status == request_status::error)
        return m_json_request["error"];
    int request_type = get_request_type();
    switch (request_type)
    {
    case get_dish_by_filter: make_get_dish_by_filter(); break;
    case unkwon_request:
        m_request_status = request_status::error;
        m_database_request << "Error: Unknow request type-<" << m_json_request["request_type"] << ">";
        break;
    default:
        break;
    }
    return std::move(m_database_request.str());   
}

/**
 * @brief Составляем SQL запрос получения блюд по фильтру.
*/
void request_handler::make_get_dish_by_filter()
{
    string filter = m_json_request["filter"];
    if(filter.empty())
    {
        m_request_status = request_status::error;
        m_database_request << "Error: parametr empty JSON-parametr <filter>.";
        return ;
    }
    m_database_request << "SELECT dish_id, dish_name, dish_kkal "
                << "FROM dishes "
                << "WHERE dish_name ILIKE '%" << filter << "%'";
    m_request_status = request_status::ready;
}
