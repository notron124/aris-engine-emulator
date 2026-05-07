# aris-engine-emulator

## 📚 Документация

- [Технические требования](docs/intro/ТТ%20общие.md)
- [Техническое задание](docs/intro/ТЗ%20на%20мат.%20модель%20стенда.md)
- [Архитектура](docs/architecture/architecture.md)

## ⚙️ Сборка и запуск тестов

### Linux

#### Сборка
Из корня проекта выполнить команду

`docker compose run --rm build-linux`

#### Запуск тестов
Из корня проекта выполнить команду

`docker compose run --rm test-linux`


### Программные требования

- CMake >= 3.16
- Qt >= 6.8.3
- C++ >= 17

## 📁 Структура репозитория 

```
aris-engine-emulator/
├── CMakeLists.txt              # корневая сборка
├── main.cpp                    # точка входа приложения
├── README.md                   # краткое описание проекта и сборки
├── docker-compose.yml          # контейнеризированная сборка и тестирование
│
├── .github/
│   ├── workflows/              # описание GitHub Actions
│
├── cmake/
│   └── AddTests.cmake          # общий helper для автосборки test_*.cpp
│
├── components/
│   └── <component>/            # утилитарный функционал архитектурного слоя (компонента)
│       ├── CMakeLists.txt
│       ├── include/            # публичный API компонента
│       ├── src/                # приватная реализация (если есть)
│       └── tests/              # unit-тесты компонента
│
├── docs/
│   ├── architecture/           # описание архитектуры, диаграммы
│   └── intro/                  # ТТ и ТЗ
│
├── docker/                     # файлы с описанием образов для сборки в docker
│
└── tests/
    └── integration/            # тесты нескольких компонентов вместе
```
