# Используем базовый образ с нужной версией ОС
FROM ubuntu:latest

# Обновляем пакеты внутри образа
RUN apt-get update && apt-get install -y \
# Установка компилятора C++
    g++ \                 
    libboost-all-dev \    
    # Установка библиотеки Boost (если ваш сервер использует Boost)
    && rm -rf /var/lib/apt/lists/*
# устанавливаем cmake
RUN apt update
RUN apt install cmake -y
# устанавливаем постгрес
RUN apt install -y libpqxx-dev
# устанавливаем гит
RUN apt install -y git
# устанавливаем гугл тест
RUN apt install -y libgtest-dev
# Устанавливаем рабочую директорию
WORKDIR /usr/src/app
# Копируем исходный код сервера внутрь контейнера

COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]