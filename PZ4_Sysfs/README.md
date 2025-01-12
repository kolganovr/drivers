# Symbolic Driver

Этот драйвер ядра Linux предоставляет символьное устройство и реализует взаимодействие с ним через sysfs. Драйвер управляет глобальной целой переменной и таймером.

## Описание

Драйвер выполняет следующие действия:

1. **Создает директорию `/sys/kernel/symbolic_driver`**.
2. **В этой директории создает два файла:**
    *   `global_variable`: Позволяет читать и записывать (сбрасывать) значение глобальной переменной.
    *   `timer_start_stop`: Позволяет запускать и останавливать таймер.
3. **Таймер, при срабатывании, инкрементирует `global_variable` каждую секунду.**

## Сборка

Чтобы собрать модуль, выполните команду:

```bash
make
```

В результате будет создан файл `symbolic_driver.ko`.

## Использование

### Загрузка модуля

```bash
sudo insmod symbolic_driver.ko
```

### Взаимодействие через sysfs

После загрузки модуля, в `/sys/kernel/` появится директория `symbolic_driver`.

**Чтение значения `global_variable`:**

```bash
cat /sys/kernel/symbolic_driver/global_variable
```

**Запись (сброс) значения `global_variable`:**

```bash
echo 0 > /sys/kernel/symbolic_driver/global_variable
```

**Запуск таймера:**

```bash
echo "start" > /sys/kernel/symbolic_driver/timer_start_stop
```

**Остановка таймера:**

```bash
echo "stop" > /sys/kernel/symbolic_driver/timer_start_stop
```

**Просмотр состояния таймера:**

```bash
cat /sys/kernel/symbolic_driver/timer_start_stop
```

Команда вернет `running` если таймер запущен, и `stopped`, если остановлен.

### Выгрузка модуля

```bash
sudo rmmod symbolic_driver
```

## Лицензия

Этот модуль распространяется под лицензией GPL.