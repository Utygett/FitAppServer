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
WORKDIR /usr/src/app
# Копируем исходный код сервера внутрь контейнера
RUN ls -a
RUN git clone https://github.com/Utygett/FitAppServer.git FitAppServer
WORKDIR /usr/src/app/FitAppServer
# создание директории сборки
RUN mkdir build
WORKDIR /usr/src/app/FitAppServer/build

RUN cmake ..
RUN make
CMD [ "./my_executable" ]