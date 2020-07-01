#include "md1.h"

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Faris Nabiev");

static int  __init my_module_init(void);
static void __exit my_module_exit(void);

module_init(my_module_init);
module_exit(my_module_exit);

int   md1_int_data = 255;
char *md1_str_data = "First module str data";

static int __init my_module_init(void)
{
    printk(KERN_INFO "Module 1 task_02: init\n");

    return 0;
}

static void __exit my_module_exit(void)
{
    printk(KERN_INFO "Module 1 task_02: unloaded\n");
}

extern char *md1_get_str(int n)
{
    printk(KERN_INFO "Module 1 task_02: md1_get_str(%d) has called\n", n);

    switch (n)
    {
        case 1:
            return "First message";

        case 2:
            return "Second message";

        default:
            return "Default message";
    }
}

extern int md1_factorial(int n)
{
    int i;
    int res = 1;

    printk(KERN_INFO "Module 1 task_02: md1_factorial(%d) has called\n", n);

    if (n > 1)
        for (i = 2; i <= n; i++)
            res *= i;

    return res;
}

// Экспорт данных
EXPORT_SYMBOL(md1_str_data);
EXPORT_SYMBOL(md1_int_data);
// Экспорт функций
EXPORT_SYMBOL(md1_get_str);
EXPORT_SYMBOL(md1_factorial);
