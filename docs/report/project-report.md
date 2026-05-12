# Отчет о текущем состоянии проекта `aris-engine-emulator`

## 1. Назначение проекта

`aris-engine-emulator` - это модуль эмуляции стенда обкатки дизельного двигателя. Его основная задача - предоставить внешней SCADA-системе программный интерфейс для взаимодействия с расчетной моделью стенда.

На уровне внешнего взаимодействия модуль выступает как `Modbus TCP Server`, а SCADA-система или ее backend - как `Modbus TCP Client`. Внешний клиент записывает управляющие значения в Modbus-регистры и читает из них результаты моделирования: состояние симуляции, телеметрию, диагностические признаки и данные расчетной модели.

Проект не реализует пользовательский интерфейс, хранение истории испытаний или формирование отчетов. Эти функции относятся к backend/frontend-части SCADA. Ответственность данного модуля - симуляция, обмен по Modbus и преобразование данных между внешним регистровым представлением и внутренней моделью.

Основные материалы по проекту:

- [README.md](../README.md)
- [Техническая архитектура](architecture/architecture.md)
- [Точка входа приложения](../main.cpp)

## 2. Текущее состояние

На текущей стадии в проекте уже собран рабочий каркас приложения. Реализованы основные компоненты:

- Modbus TCP server;
- банк Modbus-регистров;
- слой преобразования регистров во внутренние snapshot-структуры;
- контроллер симуляции;
- стратегии выполнения симуляции;
- интерфейс адаптера расчетной модели;
- простые и комплексные модели;
- unit-тесты и интеграционные тесты;
- контейнеризированная сборка и запуск тестов.

Главный результат текущего этапа - наличие сквозной цепочки от внешнего Modbus-запроса до обработки в симуляционном слое и публикации результата обратно в Modbus-регистры.

## 3. Зоны ответственности

Работа над проектом была разделена по функциональным зонам:

| Зона | Ответственный | Что входило в ответственность |
| --- | --- | --- |
| Требования и коммуникации| @EugeniusMiroshnichenko `Мирошенченко Евгений` | ТТ/ТЗ, общение с другими командами / заказчиком |
| Архитектура | @Di0nisP `Плотников Денис` | Общая архитектура проекта, взаимодействие слоев, архитектурные диаграммы |
| Modbus сервер| @notron124 `Жамбакиев Радий` | `ModbusServer`, чтение и запись coils/registers |
| Банк регистров | @Rygrotten `Рычков Григорий` |`RegisterBank`, преобразование данных в `ModelOutputs`, `ModelInputs` |
| Слой контроллера симуляции | @Di0nisP `Плотников Денис` | `SimulationController`, режимы запуска, остановки, сброса, аварии, шаг расчета |
| Адаптер модели | @Polina785643 `Тутынина Полина` | общий интерфейс `ModelAdapter`, взаимодействие с моделью |
| "Простая" модель | @EugeniusMiroshnichenko `Мирошенченко Евгений` | "Простая" математическая модель тестового стенда |
| complex-model/Simulink | @Andrey23525 `Титаев Андрей` | Исследования в области подключения "сложной" модели к проекту, "сложная" модель стенда|
| Интеграция и тесты | Общая ответственность, за главного @Di0nisP `Плотников Денис` | сквозные сценарии, QtTest, проверка взаимодействия компонентов |
| Сборка и инфраструктура | @notron124 `Жамбакиев Радий`, @Di0nisP `Плотников Денис`| CMake, Docker, GitHub Actions, команды сборки и тестирования |

Важная особенность: зоны ответственности не были абсолютно полностью изолированы друг от друга. Каждый участник мог повлиять на разработку другого слоя, особенно связанного со своим. Делалось это по средствам review запроса на слияние компонента в главную ветку.

## 4. Методология разработки

Процесс разработки оказался ближе к agile-подходу.

Компоненты разрабатывались итерационно. На ранних этапах не всегда требовалось довести каждый компонент до абсолютно полного и финального состояния. Вместо этого команда постепенно реализовывала рабочие части, проверяла их отдельно, сливала готовые модули в `main` и затем переходила к следующей итерации.

Такой подход позволил быстрее получить основу проекта и раньше увидеть реальные проблемы стыковки компонентов. После того как основные модули были готовы по отдельности, была проведена отдельная интеграционная итерация. На этом этапе компоненты были соединены в единую систему, появилась точка входа приложения, а некоторые уже готовые модули были доработаны с учетом реального взаимодействия друг с другом.

Процесс можно описать так:

```text
Разработка отдельных модулей
  -> локальная проверка и unit-тесты
  -> слияние готовых частей в main
  -> следующая итерация разработки
  -> интеграционная итерация
  -> точка входа приложения
  -> сквозной интеграционный тест
```

Практически это выразилось в следующем:

- сначала создавались отдельные компоненты: Modbus, register bank, simulation, model adapter;
- компоненты могли быть рабочими в рамках своей зоны, но еще не обязательно полностью согласованными с соседними слоями;
- по мере готовности код сливался в основную ветку;
- после готовности основных частей была проведена интеграционная итерация;
- в процессе интеграции была написана точка входа [main.cpp](../main.cpp);
- для проверки общей цепочки были добавлены интеграционные сценарии в [tests/test_api.cpp](../tests/test_api.cpp).

## 5. Структура проекта

Репозиторий организован как CMake/C++ проект на Qt.

```text
aris-engine-emulator/
  CMakeLists.txt
  main.cpp
  components/
    modbus/
    registerbank/
    simulation/
      contracts/
      controller/
      exchange/
    modeladapter/
    simple-models/
    complex-model/
  tests/
  docs/
    architecture/
    intro/
  docker/
```

Назначение основных директорий:

| Директория | Назначение |
| --- | --- |
| [components/modbus](../components/modbus) | Modbus TCP server и обработка внешних запросов |
| [components/registerbank](../components/registerbank) | Хранение регистров и преобразование между Modbus-представлением и внутренними структурами |
| [components/simulation](../components/simulation) | Контракты, контроллер симуляции, exchange-слой и стратегии выполнения |
| [components/modeladapter](../components/modeladapter) | Универсальный интерфейс доступа к расчетной модели |
| [components/simple-models](../components/simple-models) | Простые модели для расчетов и тестирования |
| [components/complex-model](../components/complex-model) | Более сложная модель, включая материалы Simulink |
| [tests](../tests) | Интеграционные тесты |
| [docs](../docs) | Техническая документация и архитектурные материалы |
| [docker](../docker) | Docker-окружение для сборки и тестирования |

Корневой [CMakeLists.txt](../CMakeLists.txt) подключает компоненты и собирает исполняемый файл `aris-engine`.

## 6. Общая архитектура

Общая архитектура построена вокруг разделения внешнего Modbus-представления и внутреннего snapshot API.

Внешний клиент работает с регистрами, но внутри проекта данные передаются в виде осмысленных структур:

- `ClientInputSnapshot` - входной снимок от клиента;
- `ModelOutputSnapshot` - выходной снимок модели;
- `ModelInputs` и `ModelOutputs` - данные расчетной модели;
- diagnostics-структуры - диагностическая информация.

Упрощенный поток данных:

```text
Backend SCADA
  -> ModbusServer
  -> RegisterBank
  -> SimulationSnapshotExchange
  -> SimulationBackendBridge
  -> SimulationController
  -> SimulationRunner
  -> ModelAdapter
  -> Simulation model
  -> ModelAdapter
  -> SimulationController
  -> RegisterBank
  -> Backend SCADA
```

Архитектурные диаграммы находятся в [docs/architecture/architecture.md](architecture/architecture.md). Там описаны слои проекта, последовательность обмена и упрощенная диаграмма классов.

Ключевой принцип архитектуры:

> SCADA не знает о внутренней модели. Она работает только с Modbus-регистрами. Симуляционный слой не знает о `QModbusTcpServer` и карте регистров. Он работает только со snapshot-структурами.

## 7. Точка входа приложения

Интеграционная итерация завершилась созданием точки входа [main.cpp](../main.cpp). В ней компоненты собираются в единую работающую систему.

Фрагмент:

```cpp
emulator::registerbank::RegisterBank registerBank;
emulator::modbus::ModbusServer server(&registerBank, &a);

auto model = emulator::model::ModelBase();
emulator::simulation::SimulationController controller(
    model,
    emulator::simulation::continuous5Config,
    &a);

auto exchange = emulator::registerbank::SimulationSnapshotExchangeImpl(&registerBank);
emulator::simulation::SimulationBackendBridge bridge(
    exchange,
    controller,
    &a);
```

Далее `RegisterBank` связывается с `SimulationBackendBridge` через Qt-сигнал:

```cpp
QObject::connect(
     &registerBank,
     &emulator::registerbank::RegisterBank::sig_inputSnapshotUpdated,
     &bridge,
     &emulator::simulation::SimulationBackendBridge::slot_processRequest);
```

После этого запускается Modbus TCP server:

```cpp
if (!server.start("127.0.0.1", 1502)) {
    return 1;
}
```

Этот файл хорошо показывает текущий composition root: банк регистров, Modbus-сервер, модель, контроллер симуляции и bridge между регистровым и симуляционным слоями.

## 8. Слой модели и `ModelAdapter`

Для отделения симуляционного слоя от конкретной модели используется интерфейс [ModelAdapter.hpp](../components/modeladapter/include/ModelAdapter.hpp).

Фрагмент:

```cpp
class ModelAdapter {
public:
    virtual ~ModelAdapter() = default;

    virtual bool initialize() = 0;
    virtual bool reset() = 0;

    virtual bool setInputs(const ModelInputs& inputs) = 0;
    virtual bool step(
        std::chrono::milliseconds modelTime,
        std::chrono::milliseconds dt) = 0;

    [[nodiscard]] virtual ModelOutputs readOutputs() const = 0;
    [[nodiscard]] virtual diagnostics::ModelDiagnosticsSnapshot diagnostics() const = 0;
    [[nodiscard]] virtual bool isRunning() const = 0;
    [[nodiscard]] virtual bool isEmergency() const = 0;
};
```

Через этот интерфейс `SimulationController` и `SimulationRunner` не зависят от того, какая модель находится внутри. Это может быть простая C++ модель или сгенерированная Simulink-модель.

## 9. Проверка работоспособности

Работоспособность проекта можно показать на примере интеграционного теста [tests/test_api.cpp](../tests/test_api.cpp).

Наиболее показательный сценарий:

```cpp
void TestIntegration::mainCompositionPublishesSimulationSnapshotAfterModbusWrite()
```

Этот тест повторяет основную композицию из `main.cpp`:

- создает `RegisterBank`;
- поднимает `ModbusServer`;
- создает модель и `SimulationController`;
- создает `SimulationSnapshotExchangeImpl`;
- создает `SimulationBackendBridge`;
- подключает настоящий `QModbusTcpClient`;
- пишет данные в holding registers;
- проверяет, что результат опубликован в input registers.

Фрагмент проверки:

```cpp
QModbusDataUnit request(QModbusDataUnit::HoldingRegisters,
                        0,
                        RegisterBank::simulationMode + 1);
request.setValues(makeStartColdRunRequest(1));
waitAndCheck(client_->sendWriteRequest(request, modbusServerAddress));

if (outputPublishedSpy.count() == 0) {
    QVERIFY(outputPublishedSpy.wait(1000));
}
QCOMPARE(outputPublishedSpy.count(), 1);

QModbusDataUnit stateRead(QModbusDataUnit::InputRegisters,
                          RegisterBank::state_ir,
                          1);
auto* stateReply = client_->sendReadRequest(stateRead, modbusServerAddress);
waitAndCheck(stateReply);
QCOMPARE(stateReply->result().value(0),
         static_cast<quint16>(SimulationState::Running));
```

Этот тест важен тем, что он проверяет не один класс, а всю цепочку:

```text
QModbusTcpClient
  -> ModbusServer
  -> RegisterBank
  -> SimulationBackendBridge
  -> SimulationController
  -> RegisterBank
  -> QModbusTcpClient
```

Также в интеграционных тестах есть сценарии полного жизненного цикла:

- `clientRunsFullModeSwitchingLifecycleWithoutLimitViolation`;
- `clientRunsModeSwitchingLifecycleAndPublishesLimitViolation`.

Они проверяют переключение режимов, публикацию выходных данных, обработку превышения лимитов и возврат системы из аварийного состояния.

## 10. Сборка и тестирование

В README описаны команды сборки и запуска тестов через Docker.

Сборка:

```bash
docker compose run --rm build-linux
```

Запуск тестов:

```bash
docker compose run --rm test-linux
```

Также в проекте настроен GitHub Actions workflow:

- [build-test-workflow.yml](../.github/workflows/build-test-workflow.yml)

В CI выполняются два основных шага:

```yaml
- name: Build
  run: docker compose run --rm build-linux

- name: Test
  run: docker compose run --rm test-linux
```

Это позволяет проверять, что проект собирается и тестируется в воспроизводимом Linux-окружении.

## 11. Что уже реализовано

На текущий момент можно считать реализованными следующие части:

- базовая структура CMake-проекта;
- компонентная организация исходного кода;
- Modbus TCP server;
- банк регистров;
- преобразование регистров во входные snapshot-структуры;
- публикация выходных snapshot-структур обратно в регистры;
- `SimulationController`;
- пошаговый и continuous-подход к выполнению симуляции;
- интерфейс `ModelAdapter`;
- простые модели и заготовка для работы со сложной моделью;
- точка входа приложения;
- unit-тесты и интеграционные тесты (частично);
- Docker-сборка и CI workflow (только под linux).

## 12. Что осталось развить

Следующие направления развития проекта:

- расширение диагностического слоя;
- более подробная настройка через конфигурационные файлы;
- расширение набора сценариев обмена со SCADA;
- детализация runtime-потоков и timing;
- дальнейшее расширение интеграционных тестов;
- синхронизация документации с финальным состоянием кода.

## 13. Итог

Проект находится на стадии рабочего интегрированного каркаса. Основные компоненты уже реализованы и соединены через точку входа приложения. Архитектура построена так, чтобы отделить внешний Modbus-интерфейс от внутренней симуляционной логики и расчетной модели.

Разработка шла итерационно, ближе к agile-процессу: сначала создавались отдельные модули, затем они сливались в `main`, после чего была проведена отдельная интеграционная итерация. Именно на этой итерации появилась общая точка входа, были уточнены границы компонентов и написаны сквозные тесты.

Главное достижение текущего этапа - проект уже демонстрирует рабочую цепочку от внешнего Modbus-клиента до симуляционного контроллера и обратно к опубликованным Modbus-регистрам.
