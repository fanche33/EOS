#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
MODULE_LICENSE("GPL");

#define MAJOR_NUM 240
#define DEVICE_NAME "mydev"

char* seg_for_c[27] = {
    "1111001100010001", // A
    "0000011100000101", // b
    "1100111100000000", // C
    "0000011001000101", // d
    "1000011100000001", // E
    "1000001100000001", // F
    "1001111100010000", // G
    "0011001100010001", // H
    "1100110001000100", // I
    "1100010001000100", // J
    "0000000001101100", // K
    "0000111100000000", // L
    "0011001110100000", // M
    "0011001110001000", // N
    "1111111100000000", // O
    "1000001101000001", // P
    "0111000001010000", //q
    "1110001100011001", //R
    "1101110100010001", //S
    "1100000001000100", //T
    "0011111100000000", //U
    "0000001100100010", //V
    "0011001100001010", //W
    "0000000010101010", //X
    "0000000010100100", //Y
    "1100110000100010", //Z
    "0000000000000000"
};

char msg_buf;

// File Operations
static ssize_t my_read(struct file *fp, char *buf, size_t count, loff_t *fpos) {
    char send_seg[16];
    printk("call read\n");

    if(msg_buf >= 'a' && msg_buf <= 'z'){
        msg_buf = msg_buf - 'a' + 'A';
    }

    if(msg_buf >= 'A' && msg_buf <= 'Z'){
        memcpy(send_seg, seg_for_c[msg_buf - 'A'], 16);
    }else{
        memcpy(send_seg, seg_for_c[26], 16);
    }
    
    //write to user
    if( copy_to_user(buf, &send_seg, count) > 0) {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }

    return count;
}

static ssize_t my_write(struct file *fp,const char *buf, size_t count, loff_t *fpos) {
    if( copy_from_user(&msg_buf, buf, count ) > 0) {
        pr_err("ERROR: Not all the bytes have been copied from user\n");
    }
    
    return count;
}

static int my_open(struct inode *inode, struct file *fp) {
    printk("call open\n");
    return 0;
}

struct file_operations my_fops = {
    read: my_read,
    write: my_write,
    open: my_open
};

static int my_init(void) {
    printk("call init\n");
    if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops) < 0) {
        printk("Can not get major %d\n", MAJOR_NUM);
        return (-EBUSY);
    }
    printk("My device is started and the major is %d\n", MAJOR_NUM);
    return 0;
}

static void my_exit(void) {
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk("call exit\n");
}

module_init(my_init);
module_exit(my_exit);