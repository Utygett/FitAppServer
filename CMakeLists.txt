cmake_minimum_required(VERSION 3.0)
project(MyProject)

# Указываем переменную с путем к директории с заголовочными файлами
set(INCLUDE_DIRS
    src
)

# Добавляем директории с заголовочными файлами из переменной
include_directories(${INCLUDE_DIRS})

# Указываем путь к заголовочным файлам libpqxx
include_directories(/usr/include)
include_directories(/usr/include/x86_64-linux-gnu)

# Указываем путь к библиотеке libpqxx
link_directories(/usr/lib/x86_64-linux-gnu)

# Добавляем исполняемый файл
add_executable(server src/main.cpp
                      src/requests/request_handler.cpp
                      src/database/database_connection_pool.cpp
                      src/config/config.cpp           
)

# Копируем файл конфигурации в папку сборки
configure_file(${CMAKE_SOURCE_DIR}/config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)

# Указываем библиотеки для линковки
target_link_libraries(server pqxx-6.4)