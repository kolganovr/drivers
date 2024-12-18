#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/init.h>

static int global_variable = 0;
static struct timer_list my_timer;
static struct kobject *my_kobject;
static bool timer_running = false;

static void timer_callback(struct timer_list *t)
{
    global_variable++;
    printk(KERN_INFO "Symbolic Driver: global_variable incremented to %d\n", global_variable);
    mod_timer(&my_timer, jiffies + HZ); // Re-arm the timer for 1 second later
}

static ssize_t global_variable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", global_variable);
}

static ssize_t global_variable_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int value;
    if (sscanf(buf, "%d", &value) == 1) {
        global_variable = value;
        return count;
    } else {
        return -EINVAL;
    }
}

static ssize_t timer_start_stop_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", timer_running ? "running" : "stopped");
}

    static ssize_t timer_start_stop_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    if (strncmp(buf, "start", 5) == 0) {
            if (!timer_running) {
                timer_running = true;
                mod_timer(&my_timer, jiffies + HZ); // Start timer in 1 second
                printk(KERN_INFO "Symbolic Driver: Timer started\n");
            }
    } else if (strncmp(buf, "stop", 4) == 0) {
        if (timer_running) {
            del_timer(&my_timer);
            timer_running = false;
            printk(KERN_INFO "Symbolic Driver: Timer stopped\n");
        }
    } else {
        return -EINVAL;
    }
    return count;
}

// Define the attributes for global_variable and timer_start_stop
static struct kobj_attribute global_variable_attribute = __ATTR(global_variable, 0664, global_variable_show, global_variable_store);
static struct kobj_attribute timer_start_stop_attribute = __ATTR(timer_start_stop, 0664, timer_start_stop_show, timer_start_stop_store);

// Define the attribute array
static struct attribute *attrs[] = {
    &global_variable_attribute.attr,
    &timer_start_stop_attribute.attr,
    NULL,
};

// Define the attribute group
static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int __init symbolic_driver_init(void)
{
    int error;

    printk(KERN_INFO "Symbolic Driver: Initializing\n");

    // Create a kobject under /sys/kernel/
    my_kobject = kobject_create_and_add("symbolic_driver", kernel_kobj);
    if (!my_kobject) {
        printk(KERN_ERR "Symbolic Driver: Failed to create kobject\n");
        return -ENOMEM;
    }

    // Create the sysfs attributes
    error = sysfs_create_group(my_kobject, &attr_group);
    if (error) {
        printk(KERN_ERR "Symbolic Driver: Failed to create sysfs group\n");
        kobject_put(my_kobject);
        return error;
    }

    // Setup the timer
    timer_setup(&my_timer, timer_callback, 0);

    printk(KERN_INFO "Symbolic Driver: Initialization complete\n");
    return 0;
}

static void __exit symbolic_driver_exit(void)
{
    if (timer_running) {
        del_timer(&my_timer);
    }
    kobject_put(my_kobject);
    printk(KERN_INFO "Symbolic Driver: Exiting\n");
}

module_init(symbolic_driver_init);
module_exit(symbolic_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kolganovr & mantlern");
MODULE_DESCRIPTION("A symbolic driver with a global variable and a timer");