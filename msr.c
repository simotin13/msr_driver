#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h> // copy_from_user, copy_to_user

MODULE_LICENSE("Dual BSD/GPL");

static int msr_devs = 1;
static int msr_major = 0;
static int msr_minor = 0;
static struct cdev msr_cdev;


// ============================================================================
// function prototype
// ============================================================================
static int msr_init(void);
ssize_t msr_read(struct file *fp, char __user *buf, size_t count, loff_t *f_pos);
static void msr_exit(void);

struct file_operations devone_fops = {
	.read = msr_read,
};

static int msr_init(void)
{
    dev_t dev;
    int alloc_ret = 0;
    int major;
    int cdev_err = 0;

    printk(KERN_DEBUG "%s called.", __FUNCTION__);
    dev = MKDEV(msr_major, 0);

    alloc_ret = alloc_chrdev_region(&dev, 0, msr_devs, "msr");
    if (alloc_ret) {
        goto error;
    }

    msr_major = major = MAJOR(dev);

    cdev_init(&msr_cdev, &devone_fops);
    msr_cdev.owner = THIS_MODULE;

    cdev_err = cdev_add(&msr_cdev, MKDEV(msr_major, msr_minor), msr_devs);
    if (cdev_err) {
        goto error;
    }

    printk(KERN_DEBUG "%s driver(major %d) installed\n", "msr", major);
    return 0;

error:
    if (cdev_err == 0) {
        cdev_del(&msr_cdev);
    }

    if (alloc_ret == 0) {
        unregister_chrdev_region(dev, msr_devs);
    }

    return -1;
}

ssize_t msr_read(struct file *fp, char __user *buf, size_t count, loff_t *f_pos)
{
	int i;
	unsigned char val = 0xff;
	int retval;

	for (i = 0 ; i < count ; i++) {
		retval = copy_to_user(&buf[i], &val, 1);
        if (retval != 0) {
            break;
        }
	}
    if (retval == 0) {
        retval = count;
    }

	return retval;
}

static void msr_exit(void)
{
	dev_t dev = MKDEV(msr_major, 0);

    printk(KERN_DEBUG "%s called.", __FUNCTION__);

	cdev_del(&msr_cdev);
	unregister_chrdev_region(dev, msr_devs);

	printk(KERN_DEBUG "msr driver removed.\n");

}

module_init(msr_init);
module_exit(msr_exit);