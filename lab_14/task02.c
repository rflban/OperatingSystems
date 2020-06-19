#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#define COOKIE_SIZE PAGE_SIZE

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Faris Nabiev");

static int cookie_index;
static int next_fortune;
static char *cookie_pot;

static struct proc_dir_entry *p_entry;
static struct proc_dir_entry *p_dir;
static struct proc_dir_entry *p_link;

static ssize_t fortune_read(struct file *fd, char __user *buf,
                            size_t len, loff_t *ppos);
static ssize_t fortune_write(struct file *fd, const char __user *buf,
                             size_t len, loff_t *ppos);

static int fortune_init(void)
{
    int rc;

    static struct file_operations fops = {
        .owner = THIS_MODULE,
        .read = &fortune_read,
        .write = &fortune_write
    };

    if (!(cookie_pot = (char *)vmalloc(COOKIE_SIZE)))
    {
        printk(KERN_ERR "fortune: can't alloc memory for cookie_pot!\n");
        return -ENOMEM;
    }

    memset(cookie_pot, 0, COOKIE_SIZE);

    if (!(p_entry = proc_create("fortune", 0644, NULL, &fops)))
    {
        printk(KERN_ERR "fortune: can't create fortune entry!\n");
        vfree(cookie_pot);
        return -ENOMEM;
    }

    printk(KERN_INFO "fortune: module have loaded.\n");

    rc = 0;

    if (!(p_dir = proc_mkdir("fortune_dir", NULL)))
    {
        printk(KERN_ERR "fortune: can't create fortune directory!\n");
        rc = -ENOMEM;
    }
    if (!(p_link = proc_symlink("fortune_link",
                                NULL, "fortune")))
    {
        printk(KERN_ERR "fortune: can't create fortune symlink!\n");
        rc = -ENOMEM;
    }

    return rc;
}

static void fortune_exit(void)
{
    proc_remove(p_entry);
    proc_remove(p_dir);
    proc_remove(p_link);

    vfree(cookie_pot);

    printk(KERN_INFO "fortune: module have unloaded.\n");
}

static ssize_t fortune_read(struct file *fd, char __user *buf,
                            size_t len, loff_t *ppos)
{
    if (*ppos > 0)
        return 0;

    if (next_fortune >= cookie_index)
        next_fortune = 0;

    len = copy_to_user(buf, cookie_pot + next_fortune, len);
    next_fortune += len;
    *ppos += len;

    return len;
}

static ssize_t fortune_write(struct file *fd, const char __user *buf,
                             size_t len, loff_t *ppos)
{
    int available_size = COOKIE_SIZE - cookie_index + 1;

    if (len > (size_t)available_size)
    {
        printk(KERN_NOTICE "fortune: there is not enough "
                           "memory in cookie pot!\n");
        return -ENOSPC;
    }

    if (copy_from_user(cookie_pot + cookie_index, buf, len))
    {
        printk(KERN_NOTICE "fortune: copy_to_user failed!\n");
        return -EFAULT;
    }

    cookie_index += len;
    cookie_pot[cookie_index - 1] = 0;

    return len;
}

module_init(fortune_init);
module_exit(fortune_exit);

