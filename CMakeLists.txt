cmake_minimum_required(VERSION 3.0)
project(MyProject)

# Указываем путь к заголовочным файлам libpqxx
include_directories(/usr/include)
include_directories(/usr/include/x86_64-linux-gnu)

# Указываем путь к библиотеке libpqxx
link_directories(/usr/lib/x86_64-linux-gnu)

# Добавляем исполняемый файл
add_executable(server src/main.cpp)

# Указываем библиотеки для линковки
target_link_libraries(server pqxx-6.4)