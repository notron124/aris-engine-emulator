Для редактирования модели необходим Matlab с пакетом Simulink

В Matlab нужно открыть файл констант и загрузить их в оперативную память:
Matlab -> Верхняя вкладка Editor -> Кнопка "Open" -> Выбрать constants.m из этой папки
После этого нажать кнопку Run в этой же вкладке Editor. Matlab предложит сменить директорию, это <span style="color:red">обязательно</span> нужно сделать, нажав "Change folder".
Теперь константы в RAM, модель готова к редактированию

Верхняя вкладка Home -> Simulink -> Кнопка в левой колонке "Open..." -> Выбрать total_system.slx из этой папки
Для кодогенерации нужно выбрать: Simulink с открытой моделью -> Apps -> "Embedded coder" -> "Generate code"
Файлы создаются рядом в директории "total_system_ert_rtw" и "slprj".
Основные файлы модели находятся в "total_system_ert_rtw".
Компоненты отдельно в slprj/ert/(FrequencyConverter/BallastResistor/AsyncMotor/ICE)
