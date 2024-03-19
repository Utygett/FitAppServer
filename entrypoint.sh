#!/bin/bash

# Переходим в папку приложения
cd /usr/src/app

# Проверяем, существует ли папка приложения
if [ -d "/usr/src/app/FitAppServer" ]; then
    # Если существует, делаем пул изменений
    git -C /usr/src/app/FitAppServer pull
else
    # Если не существует, клонируем репозиторий
    git clone https://github.com/Utygett/FitAppServer.git FitAppServer
fi

# Создаем директорию для сборки и переходим в неё
mkdir -p /usr/src/app/FitAppServer/build
cd /usr/src/app/FitAppServer/build

# Запускаем cmake и make
cmake ..
make

# Запускаем ваше приложение
./server