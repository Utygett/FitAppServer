cmake_minimum_required(VERSION 3.10)
project(MyProjectTests)

# Включаем GTest
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Добавляем исполняемый файл для наших тестов
add_executable(runTests src/main.cpp src/test1.cpp) # Добавьте сюда ваши тестовые файлы

# Линкуем с библиотеками
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)

# Добавляем тесты в CTest
add_test(NAME MyTest COMMAND runTests)