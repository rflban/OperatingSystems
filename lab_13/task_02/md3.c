#include "md1.h"

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Faris Nabiev");

static int __init my_module_init(void);

module_init(my_module_init);

static int __init my_module_init(void)
{
    printk(KERN_INFO "Module 3 task_02: init\n");

    printk(KERN_INFO "Module 3 task_02: md1_int_data      = %d\n",
           md1_int_data);
    printk(KERN_INFO "Module 3 task_02: md1_str_data      = %s\n",
           md1_str_data);
    printk(KERN_INFO "Module 3 task_02: md1_get_str(10)   = %s\n",
           md1_get_str(10));
    printk(KERN_INFO "Module 3 task_02: md1_get_str(1)    = %s\n",
           md1_get_str(1));
    printk(KERN_INFO "Module 3 task_02: md1_get_str(2)    = %s\n",
           md1_get_str(2));
    printk(KERN_INFO "Module 3 task_02: md1_factorial(10) = %d\n",
           md1_factorial(10));

    return -1;
}
