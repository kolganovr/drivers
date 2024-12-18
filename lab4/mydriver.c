#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/device.h> // Добавлен заголовочный файл для device_* функций

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kolganovr");
MODULE_DESCRIPTION("A simple character driver for generating and measuring reaction time");

#define DEVICE_NAME "mydriver"
#define CLASS_NAME "mydriver_class"

static int majorNumber;
static struct class* mydriverClass = NULL;
static struct device* mydriverDevice = NULL;

// Таймер
static struct timer_list my_timer;
static unsigned long interval_ms = 1000; // Интервал в миллисекундах (1 секунда по умолчанию)

// Переменные для измерения времени реакции
static ktime_t start_time;
static ktime_t reaction_time;
static unsigned long long num_reactions = 0;
static unsigned long long sum_reaction_times = 0;
static unsigned long long min_reaction_time = ULLONG_MAX; // Инициализируем максимальным значением
static unsigned long long max_reaction_time = 0;

#define MAX_REACTIONS 1024 // Максимальное количество хранимых реакций
static unsigned long long reaction_times[MAX_REACTIONS]; // Массив для хранения времен реакций
static unsigned long long current_index = 0; // Индекс для добавления новых значений в массив

// Прототипы функций
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

// Структура file_operations определяет, как драйвер будет реагировать на операции с файлом
static struct file_operations fops =
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

// Функция, вызываемая таймером
static void my_timer_callback(struct timer_list *t)
{
    // Фиксируем время начала "воздействия"
    start_time = ktime_get();

    // Здесь генерируем "внешнее воздействие" - просто выводим сообщение в лог ядра
    printk(KERN_INFO "mydriver: Timer fired! (simulated external event)\n");

    // Перезапускаем таймер
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(interval_ms));
}

// Функция инициализации модуля - вызывается при загрузке драйвера
static int __init mydriver_init(void)
{
    printk(KERN_INFO "mydriver: Initializing the mydriver LKM\n");

    // Регистрация старшего номера устройства
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
        printk(KERN_ALERT "mydriver failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "mydriver: registered correctly with major number %d\n", majorNumber);

    // Регистрация класса устройства
    mydriverClass = class_create(CLASS_NAME);
    if (IS_ERR(mydriverClass)){
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(mydriverClass);
    }
    printk(KERN_INFO "mydriver: device class registered correctly\n");

    // Регистрация драйвера устройства
    mydriverDevice = device_create(mydriverClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mydriverDevice)){
        class_destroy(mydriverClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(mydriverDevice);
    }
    printk(KERN_INFO "mydriver: device class created correctly\n");

    // Инициализация таймера
    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(interval_ms));

    return 0;
}

// Функция выгрузки модуля - вызывается при выгрузке драйвера
static void __exit mydriver_exit(void)
{
    // Удаление таймера
    del_timer(&my_timer);

    // Удаление устройства
    device_destroy(mydriverClass, MKDEV(majorNumber, 0));
    // Удаление класса устройства
    class_destroy(mydriverClass);
    // Отмена регистрации старшего номера устройства
    unregister_chrdev(majorNumber, DEVICE_NAME);

    // Обнуляем переменные, относящиеся ко второму пункту
    num_reactions = 0;
    sum_reaction_times = 0;
    min_reaction_time = ULLONG_MAX;
    max_reaction_time = 0;

    // Обнуляем индекс массива времен реакций
    current_index = 0;

    printk(KERN_INFO "mydriver: Goodbye!\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mydriver: Device has been opened\n");
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    char message[256];
    int message_len;
    static bool data_read = false;
    size_t total_sent = 0; // Общее количество байт, отправленных пользователю

    if (data_read) {
        data_read = false;
        return 0;
    }

    // Рассчитываем среднее время реакции
    unsigned long long avg_reaction_time = 0;
    if (num_reactions > 0) {
        avg_reaction_time = sum_reaction_times / num_reactions;
    }

    // Формируем строку с результатами
    message_len = snprintf(message, sizeof(message), "Average: %llu ns, Max: %llu ns, Min: %llu ns\n",
                           avg_reaction_time, max_reaction_time, min_reaction_time);

    if (message_len >= sizeof(message)) {
        printk(KERN_WARNING "mydriver: Message too long\n");
        return -EINVAL;
    }

    // Копируем данные в пользовательское пространство
    error_count = copy_to_user(buffer, message, message_len);
    
    if (error_count != 0) {
        printk(KERN_INFO "mydriver: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }

    printk(KERN_INFO "mydriver: Sent %d characters to the user\n", message_len);
    total_sent += message_len;

    // Выводим все времена реакций
    for (unsigned long long i = 0; i < current_index; i++) {
        message_len = snprintf(message, sizeof(message), "%llu ns\n", reaction_times[i]);

        if (message_len >= sizeof(message)) {
            printk(KERN_WARNING "mydriver: Message too long\n");
            return -EINVAL;
        }

        if (total_sent + message_len > len) {
            printk(KERN_WARNING "mydriver: Not enough space in user buffer\n");
            break; // Прерываем цикл, если не хватает места в буфере
        }

        error_count = copy_to_user(buffer + total_sent, message, message_len);

        if (error_count != 0) {
            printk(KERN_INFO "mydriver: Failed to send %d characters to the user\n", error_count);
            return -EFAULT;
        }
        printk(KERN_INFO "mydriver: Sent %d characters to the user\n", message_len);
        total_sent += message_len;
    }

    data_read = true;
    return (size_t)total_sent;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
    // Фиксируем время реакции
    reaction_time = ktime_sub(ktime_get(), start_time);
    unsigned long long reaction_time_ns = ktime_to_ns(reaction_time);

    // Обновляем статистику
    num_reactions++;
    sum_reaction_times += reaction_time_ns;

    if (reaction_time_ns < min_reaction_time) {
        min_reaction_time = reaction_time_ns;
    }
    if (reaction_time_ns > max_reaction_time) {
        max_reaction_time = reaction_time_ns;
    }

    // Добавляем время реакции в массив
    if (current_index < MAX_REACTIONS) {
        reaction_times[current_index] = reaction_time_ns;
        current_index++;
    } else {
        printk(KERN_WARNING "mydriver: Maximum number of reactions reached, not storing new values\n");
    }

    printk(KERN_INFO "mydriver: Device write, reaction time: %llu ns\n", reaction_time_ns);
    return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mydriver: Device successfully closed\n");
   return 0;
}

// Макросы для регистрации функций инициализации и выгрузки
module_init(mydriver_init);
module_exit(mydriver_exit);