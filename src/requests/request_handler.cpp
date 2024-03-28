#include "request_handler.h"
#include <iostream>
using namespace std;
using json = nlohmann::json;

request_handler::request_map request_handler::s_request_ids = {
    {"get_dishes_by_filter", get_dish_by_filter},
    {"nutritional_info", nutritional_info},
    {"insert_into_meals", insert_into_meals},
    {"get_list_dish_of_day", get_list_dish_of_day}
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
    case nutritional_info: make_nutritional_info(); break;
    case insert_into_meals: make_insert_into_meals(); break;
    case get_list_dish_of_day: make_get_list_dish_of_day(); break;
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

/**
 * @brief Составляем SQL запрос получения информации о блюде.
*/
void request_handler::make_nutritional_info()
{
    string dish_id = m_json_request["dish_id"];
    if(dish_id.empty())
    {
        m_request_status = request_status::error;
        m_database_request << "Error: parametr empty JSON-parametr <dish_id>.";
        return;
    }
    m_database_request << "SELECT "
                         << "dishes.dish_id, "
                         << "dishes.dish_name, "
                         << "dishes.dish_kkal AS total_kkal, "
                         << "SUM(CASE WHEN microelements.element_name = 'Белки' THEN ingredients_microelements.quantity ELSE 0 END) AS protein, "
                         << "SUM(CASE WHEN microelements.element_name = 'Жиры' THEN ingredients_microelements.quantity ELSE 0 END) AS fat, "
                         << "SUM(CASE WHEN microelements.element_name = 'Углеводы' THEN ingredients_microelements.quantity ELSE 0 END) AS carbohydrates "
                         << "FROM "
                         << "dishes "
                         << "JOIN ingredients_microelements ON dishes.dish_id = ingredients_microelements.ingredient_id "
                         << "JOIN microelements ON ingredients_microelements.element_id = microelements.element_id "
                         << "WHERE "
                         << "dishes.dish_id = '" << dish_id << "' "
                         << "GROUP BY "
                         << "dishes.dish_id, dishes.dish_name";
    m_request_status = request_status::ready;
}

/**
 * @brief Составляем SQL запрос добавления информации о съеденном блюде.
*/
void request_handler::make_insert_into_meals()
{
    string user_id = m_json_request["user_id"];
    string dish_id = m_json_request["dish_id"];
    string amount = m_json_request["amount"];
    string date = m_json_request["date"];
    if(user_id.empty() || dish_id.empty() || amount.empty() || date.empty())
    {
        m_request_status = request_status::error;
        m_database_request << "Error: parametr empty JSON-parametr";
        return;
    }
    m_database_request << "INSERT INTO meals (user_id, dish_id, eaten_on, amount_gramm) "
                        << "VALUES ('"<< user_id << "', "
                        << "'" << dish_id << "', "
                        << "'" << date << "', "
                        << "'" << amount << "')";
    m_request_status = request_status::ready;
}

/**
 * @brief Составляем SQL запрос добавления информации о съеденном блюде.
*/
void request_handler::make_get_list_dish_of_day()
{
    string user_id = m_json_request["user_id"];
    string date = m_json_request["date"];
        if(user_id.empty() || date.empty())
    {
        m_request_status = request_status::error;
        m_database_request << "Error: parametr empty JSON-parametr";
        return;
    }
    m_database_request << "SELECT meals.meal_id, meals.dish_id, meals.eaten_on, meals.amount_gramm, "
          << "dishes.dish_name, dishes.dish_kkal, "
          << "COALESCE(SUM(CASE WHEN microelements.element_name = 'Белки' THEN ingredients_microelements.quantity END), 0) AS proteins, "
          << "COALESCE(SUM(CASE WHEN microelements.element_name = 'Жиры' THEN ingredients_microelements.quantity END), 0) AS fats, "
          << "COALESCE(SUM(CASE WHEN microelements.element_name = 'Углеводы' THEN ingredients_microelements.quantity END), 0) AS carbohydrates "
          << "FROM meals "
          << "INNER JOIN dishes ON meals.dish_id = dishes.dish_id "
          << "LEFT JOIN dishes_ingredients ON meals.dish_id = dishes_ingredients.dish_id "
          << "LEFT JOIN ingredients_microelements ON dishes_ingredients.ingredient_id = ingredients_microelements.ingredient_id "
          << "LEFT JOIN microelements ON ingredients_microelements.element_id = microelements.element_id "
          << "WHERE meals.user_id = '" << user_id << "' AND DATE(meals.eaten_on) = '" << date << "' "
          << "GROUP BY meals.meal_id, meals.dish_id, dishes.dish_name, dishes.dish_kkal, meals.eaten_on "
          << "ORDER BY meals.eaten_on DESC";
    m_request_status = request_status::ready;
}