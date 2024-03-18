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
# Устанавливаем рабочую директорию
RUN apt install -y git
WORKDIR /usr/src/app/server
# Копируем исходный код сервера внутрь контейнера
COPY src /usr/src/app/server/src
COPY CMakeLists.txt /usr/src/app/server
# Команда, которая будет выполнена при запуске контейнера
RUN mkdir build
WORKDIR /usr/src/app/server/build
#WORKDIR /usr/src/app/server
#RUN pwd
#RUN ls -la
#RUN cat main.cpp
#CMD [ "cat main.cpp" ]
RUN cmake ..
RUN make