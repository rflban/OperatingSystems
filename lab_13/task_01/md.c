#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init_task.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Faris Nabiev");

static void __exit my_module_exit(void);
static int  __init my_module_init(void);

module_init(my_module_init);
module_exit(my_module_exit);

// Инициализация модуля
static int __init my_module_init(void)
{
   struct task_struct *task = &init_task;

   printk(KERN_INFO "Module task_01: loaded!\n");

   do
   {
     printk(KERN_INFO "Module task_01: "
                      "process: %s - %d, ""parent: %s - %d\n",
            task->comm, task->pid, task->parent->comm, task->parent->pid);
   }
   while ((task = next_task(task)) != &init_task);

   printk(KERN_INFO "Module task_01: "
                    "current: %s — %d, parent: %s — %d\n",
          current->comm, current->pid,
          current->parent->comm, current->parent->pid);

   return 0;
}

// Удаление модуля
static void __exit my_module_exit(void)
{
   printk(KERN_INFO "Module task_01: unloaded\n");
}
