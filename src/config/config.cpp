#include "config.h"

/**
 * @brief Конструктор класса config.
 * 
 * Читает параметры из указанного файла конфигурации.
 * 
 * @param configFile Путь к файлу конфигурации.
 * @throw std::runtime_error Если не удалось открыть файл конфигурации.
 */
config::config(const std::string& configFile)
{
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open config file: " + configFile);
    }

    std::string line;
    while (std::getline(file, line)) {
        if(line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value))
            m_config_params.emplace(key, value);
    }
}

/**
 * @brief Получает значение параметра из конфигурационного файла.
 * 
 * @param param Имя параметра.
 * @return Значение параметра, если он существует, иначе пустая строка.
 */
std::string config::get_param_value(const std::string & param)
{
    auto itParam = m_config_params.find(param);
    if(itParam == m_config_params.end())
        return {};
    return itParam->second;
}