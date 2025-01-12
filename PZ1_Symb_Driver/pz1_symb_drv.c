#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DRIVER_NAME "pz1_symb_drv"
#define BUFFER_SIZE 1024

// Глобальный буфер
static char global_buffer[BUFFER_SIZE];
static int buffer_size = BUFFER_SIZE;

// Мажорный и минорный номер устройства
static int major_number;
static struct cdev cdev;

// Указатели на struct class и struct device
static struct class *dev_class;
static struct device *dev_device;

// Функции для работы с устройством
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);
static long dev_ioctl(struct file *, unsigned int, unsigned long);

// Структура с описанием операций над файлом
static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

static int dev_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "pz1_symb_drv: Device opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "pz1_symb_drv: Device closed\n");
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset) {
    int bytes_to_copy;
    int bytes_copied;

    bytes_to_copy = min((int)count, (int)(buffer_size - *offset));
    bytes_copied = copy_to_user(user_buffer, global_buffer + *offset, bytes_to_copy);

    if (bytes_copied) {
        printk(KERN_ERR "pz1_symb_drv: Failed to copy %d bytes to user\n", bytes_copied);
        return -EFAULT;
    }

    *offset += bytes_to_copy;
    printk(KERN_INFO "pz1_symb_drv: Read %d bytes from offset %lld\n", bytes_to_copy, *offset - bytes_to_copy);
    return bytes_to_copy;
}

static ssize_t dev_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset) {
    int bytes_to_copy;
    int bytes_copied;

    bytes_to_copy = min((int)count, (int)(buffer_size - *offset));
    bytes_copied = copy_from_user(global_buffer + *offset, user_buffer, bytes_to_copy);

    if (bytes_copied) {
        printk(KERN_ERR "pz1_symb_drv: Failed to copy %d bytes from user\n", bytes_copied);
        return -EFAULT;
    }

    *offset += bytes_to_copy;
    printk(KERN_INFO "pz1_symb_drv: Wrote %d bytes at offset %lld\n", bytes_to_copy, *offset - bytes_to_copy);
    return bytes_to_copy;
}

#define IOCTL_RESET_BUFFER _IOW('k', 1, int)

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_RESET_BUFFER:
            if (copy_from_user(&buffer_size, (int __user *)arg, sizeof(buffer_size))) {
                printk(KERN_ERR "pz1_symb_drv: ioctl copy_from_user failed\n");
                return -EFAULT;
            }
            if(buffer_size > BUFFER_SIZE || buffer_size <= 0){
                printk(KERN_ERR "pz1_symb_drv: Invalid buffer size requested: %d\n", buffer_size);
                buffer_size = BUFFER_SIZE;
                return -EINVAL;
            }
            memset(global_buffer, 0, buffer_size);
            printk(KERN_INFO "pz1_symb_drv: Buffer reset to size %d\n", buffer_size);
            break;
        default:
            printk(KERN_WARNING "pz1_symb_drv: Unknown ioctl command\n");
            return -ENOTTY;
    }
    return 0;
}

static int __init pz1_symb_drv_init(void) {
    // Регистрация устройства
    major_number = register_chrdev(0, DRIVER_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "pz1_symb_drv: Failed to register a major number\n");
        return major_number;
    }

    // Инициализация cdev
    cdev_init(&cdev, &fops);
    cdev.owner = THIS_MODULE;

    // Добавление устройства в систему
    if (cdev_add(&cdev, MKDEV(major_number, 0), 1) < 0) {
        printk(KERN_ERR "pz1_symb_drv: Failed to add cdev\n");
        unregister_chrdev(major_number, DRIVER_NAME);
        return -1;
    }

    // Создание класса устройства
    dev_class = class_create(DRIVER_NAME);
    if (IS_ERR(dev_class)) {
        printk(KERN_ERR "pz1_symb_drv: Failed to create device class\n");
        cdev_del(&cdev);
        unregister_chrdev(major_number, DRIVER_NAME);
        return PTR_ERR(dev_class);
    }

    // Создание устройства
    dev_device = device_create(dev_class, NULL, MKDEV(major_number, 0), NULL, DRIVER_NAME);
    if (IS_ERR(dev_device)) {
        printk(KERN_ERR "pz1_symb_drv: Failed to create device\n");
        class_destroy(dev_class);
        cdev_del(&cdev);
        unregister_chrdev(major_number, DRIVER_NAME);
        return PTR_ERR(dev_device);
    }

    printk(KERN_INFO "pz1_symb_drv: Module loaded. Major number: %d\n", major_number);
    return 0;
}

static void __exit pz1_symb_drv_exit(void) {
    // Удаление устройства
    device_destroy(dev_class, MKDEV(major_number, 0));

    // Удаление класса устройства
    class_destroy(dev_class);

    // Удаление устройства из системы
    cdev_del(&cdev);

    // Отмена регистрации устройства
    unregister_chrdev(major_number, DRIVER_NAME);

    printk(KERN_INFO "pz1_symb_drv: Module unloaded\n");
}

module_init(pz1_symb_drv_init);
module_exit(pz1_symb_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kolganovr & mantlern");
MODULE_DESCRIPTION("PZ1 Symbolic Driver");