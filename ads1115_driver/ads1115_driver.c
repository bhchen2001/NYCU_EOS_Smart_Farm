#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#define DEVICE_NAME "ads1115"
#define I2C_BUS_NUM 1
#define ADS1115_I2C_ADDR 0x48

// ADS1115 Registers
#define ADS1115_REG_CONVERSION    0x00
#define ADS1115_REG_CONFIG        0x01

// Configuration register settings for single-shot read from AIN0, ±4.096V, 1600SPS
// Similar configuration to user code
// MSB example (for AIN0): 0xC3 (0b11000011)
// LSB: 0x83
// illustrate:
// PGA: ±4.096V (001) --> bits[11:9] = 001
// MUX: AIN0 = 100
// MODE: single-shot = 1
// DR: 1600SPS = 100
static unsigned char ads1115_config_channel0[3] = {
    ADS1115_REG_CONFIG,
    0xC3, // MUX = AIN0, PGA=±4.096V, single-shot
    0x83  // 1600SPS, comparator disabled
};

static int major;
// static char data_buffer[2];
static struct cdev ads1115_cdev;
static struct class *ads1115_class;
static struct i2c_client *ads1115_client; 
static struct i2c_adapter *ads1115_adapter;

static int ads1115_open(struct inode *inode, struct file *file) {
    return 0;
}

static int ads1115_release(struct inode *inode, struct file *file) {
    return 0;
}

// 設定並讀取 ADS1115 的流程
// 1. 寫入 config register 設定單次轉換
// 2. 等待一段時間讓 ADC 完成轉換
// 3. 讀取 conversion register 兩個 byte
static int ads1115_read_value(struct i2c_client *client, int *raw_value) {
    int ret;
    unsigned char data[2];
    unsigned char reg = ADS1115_REG_CONVERSION;

    // 寫入 config 設定 (單次讀取)
    ret = i2c_master_send(client, ads1115_config_channel0, 3);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to write config register\n");
        return ret;
    }

    // 等待轉換完成：理論上可用Poll Ready bit，但這裡簡單以延遲替代 (100ms)
    msleep(100);

    // 設定要讀取的 register (conversion register)
    ret = i2c_master_send(client, &reg, 1);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to set conversion register\n");
        return ret;
    }

    // 讀取兩個 byte 資料
    ret = i2c_master_recv(client, data, 2);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read conversion data\n");
        return ret;
    }

    // 組合 16-bit 整數
    *raw_value = (data[0] << 8) | data[1];
    if (*raw_value > 32767)
        *raw_value -= 65536;

    return 0;
}

static ssize_t ads1115_read_fn(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    int raw_value;
    int ret;
    char outbuf[10];
    int outlen;

    if (!ads1115_client) {
        pr_err("ads1115 client not initialized.\n");
        return -EIO;
    }

    // 從 ADS1115 讀取數值
    ret = ads1115_read_value(ads1115_client, &raw_value);
    if (ret < 0) {
        return ret;
    }

    // 將數值轉為字串，供使用者空間讀取
    outlen = scnprintf(outbuf, sizeof(outbuf), "%d\n", raw_value);

    if (*offset >= outlen) {
        return 0; // EOF
    }

    if (len < outlen) {
        return -EINVAL;
    }

    if (copy_to_user(buf, outbuf, outlen)) {
        return -EFAULT;
    }
    // *offset += outlen;

    return outlen;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ads1115_open,
    .read = ads1115_read_fn,
    .release = ads1115_release,
};


static int __init ads1115_init(void) {
    dev_t dev_num;
    int ret;

    // 動態分配 major
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate chrdev region\n");
        return ret;
    }
    major = MAJOR(dev_num);

    cdev_init(&ads1115_cdev, &fops);
    ads1115_cdev.owner = THIS_MODULE;
    ret = cdev_add(&ads1115_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    ads1115_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(ads1115_class)) {
        pr_err("Failed to create class\n");
        cdev_del(&ads1115_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(ads1115_class);
    }

    device_create(ads1115_class, NULL, dev_num, NULL, DEVICE_NAME);

    // 取得對應 I2C bus 的 adapter
    ads1115_adapter = i2c_get_adapter(I2C_BUS_NUM);
    if (!ads1115_adapter) {
        pr_err("Failed to get I2C adapter\n");
        device_destroy(ads1115_class, dev_num);
        class_destroy(ads1115_class);
        cdev_del(&ads1115_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -ENODEV;
    }

    // 建立一個對應 ADS1115 的 i2c_client (dummy device)
    {
        struct i2c_board_info info = {
            I2C_BOARD_INFO("ads1115", ADS1115_I2C_ADDR),
        };
        ads1115_client = i2c_new_client_device(ads1115_adapter, &info);
        if (!ads1115_client) {
            pr_err("Failed to create i2c client\n");
            i2c_put_adapter(ads1115_adapter);
            device_destroy(ads1115_class, dev_num);
            class_destroy(ads1115_class);
            cdev_del(&ads1115_cdev);
            unregister_chrdev_region(dev_num, 1);
            return -ENODEV;
        }
    }

    pr_info("ADS1115 driver initialized\n");
    return 0;
}

static void __exit ads1115_exit(void) {
    dev_t dev_num = MKDEV(major, 0);

    if (ads1115_client) {
        i2c_unregister_device(ads1115_client);
        ads1115_client = NULL;
    }

    if (ads1115_adapter) {
        i2c_put_adapter(ads1115_adapter);
        ads1115_adapter = NULL;
    }

    device_destroy(ads1115_class, dev_num);
    class_destroy(ads1115_class);
    cdev_del(&ads1115_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("ADS1115 driver exited\n");
}

module_init(ads1115_init);
module_exit(ads1115_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("312605012 Yujia He");
MODULE_DESCRIPTION("ADS1115 I2C Driver for Raspberry Pi");

