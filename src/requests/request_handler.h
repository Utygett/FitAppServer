#pragma once
#include "nlohmann/json.hpp"


/**
 * @brief Класс для обработки запросов.
 * 
 * Этот класс обрабатывает запросы, и подготавливает запросы к базе данных.
 */
class request_handler
{
    /**
    * @brief Перечисление id запросов для сервера.
    */
    enum request_type_id {
        unkwon_request,          ///< Неизвестный тип запроса.
        get_dish_by_filter,      ///< Поиск блюд по определенному фильтру.
        nutritional_info,        ///< Получение данных о питательной ценности блюда
        insert_into_meals,       ///< Добавление информации о съеденном блюде
        get_list_dish_of_day     ///< Получение списка блюд съеденных за определенный день

    };
public:
    /**
    * @brief Перечисление статусов для исполняемого запроса.
    */
    enum request_status {
        in_process,     ///< Запрос в процессе преобразования
        error,          ///< Запрос завершился ошбкой преобразования
        ready           ///< Запрос успешно преобразован
    };
private:
    typedef std::map<std::string, request_handler::request_type_id> request_map;
    static request_map s_request_ids;       ///< Карта запросов для сервера.
    nlohmann::json m_json_request;          ///< JSON объект, содержащий данные запроса.
    std::stringstream m_database_request;   ///< SQL запрос для базы данных.
    request_status m_request_status;        ///< Состояние запроса.
private:
    request_type_id get_request_type();
    void make_get_dish_by_filter();
    void make_nutritional_info();
    void make_insert_into_meals();
    void make_get_list_dish_of_day();
public:
    request_handler(const std::string &request);
    ~request_handler();
    std::string prepare_database_request();
};
