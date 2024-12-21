#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#define DEVICE_NAME "l298n"
#define IN1 17
#define IN2 27
#define ENA 22


static int major;
static struct class *dev_class;
static char motor_cmd;

static int l298n_open(struct inode *inode, struct file *file) {
    pr_info("L298N Device Opened\n");
    return 0;
}

static int l298n_release(struct inode *inode, struct file *file) {
    pr_info("L298N Device Closed\n");
    return 0;
}

static void motor_on(void) {
    gpio_set_value(IN1, 0);
    gpio_set_value(IN2, 1);
    gpio_set_value(ENA, 1);
    pr_info("Motor ON\n");
}

static void motor_off(void) {
    gpio_set_value(IN1, 0);
    gpio_set_value(IN2, 0);
    gpio_set_value(ENA, 0);
    pr_info("Motor OFF\n");
}

static ssize_t l298n_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    if (copy_from_user(&motor_cmd, buf, 1) != 0) {
        pr_err("ERROR: Failed to copy data from user\n");
        return -EFAULT;
    }
    
    if (motor_cmd == 'O') {
        motor_on();
    } else {
        motor_off();
    }
    return len;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .write   = l298n_write,
    .open    = l298n_open,
    .release = l298n_release,
};

static int __init l298n_driver_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register device\n");
        return major;
    }

    dev_class = class_create(THIS_MODULE, "l298n_class");
    if (IS_ERR(dev_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(dev_class);
    }

    gpio_request(IN1, "sysfs");
    gpio_request(IN2, "sysfs");
    gpio_request(ENA, "sysfs");


    gpio_direction_output(IN1, 0);
    gpio_direction_output(IN2, 0);
    gpio_direction_output(ENA, 0);


    pr_info("L298N Driver Initialized\n");
    return 0;
}

static void __exit l298n_driver_exit(void) {
    motor_off();
    gpio_free(IN1);
    gpio_free(IN2);
    gpio_free(ENA);

    class_destroy(dev_class);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("L298N Driver Removed\n");
}

module_init(l298n_driver_init);
module_exit(l298n_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("312605012 Yujia Ho");
MODULE_DESCRIPTION("L298N Motor Driver");

