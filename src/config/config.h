#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <map>

typedef std::map<std::string, std::string> config_map;
/**
 * @brief Класс для работы с конфигурационным файлом.
 * 
 * Читает параметры из файла конфигурации и предоставляет методы для доступа к ним.
 */
class config {
private:
    config_map m_config_params; ///< Словарь для хранения параметров конфигурации.

public:
    config(const std::string& configFile);
    std::string get_param_value(const std::string & param);
};