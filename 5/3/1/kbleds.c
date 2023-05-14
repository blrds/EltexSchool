#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h>          /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>           /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h>       /* For vc_cons */
#include <linux/vt_kern.h>
#include <linux/timer.h>

#include <linux/printk.h> 
#include <linux/kobject.h> 
#include <linux/sysfs.h> 
#include <linux/fs.h> 
#include <linux/string.h>
 
 
MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");
MODULE_LICENSE("GPL");
struct timer_list my_timer;
struct tty_driver *my_driver;
static int _kbledstatus = 0xFF;
#define BLINK_DELAY   HZ/5
#define RESTORE_LEDS  0xFF

static struct kobject *example_kobject;
static int test;
 
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
        return sprintf(buf, "%d\n", test);
}
 
static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
                      const char *buf, size_t count)
{
        sscanf(buf, "%du", &test);
        return count;
}
 
static struct kobj_attribute foo_attribute =__ATTR(test, 0660, foo_show,foo_store);

static void my_timer_func(struct timer_list *ptr)
{
        if(test>=0 && test<=7)
                (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,test);
        my_timer.expires = jiffies + BLINK_DELAY;
        add_timer(&my_timer);
}

static int __init kbleds_init(void)
{
        int error = 0;
 
        pr_debug("Module initialized successfully \n");
 
        example_kobject = kobject_create_and_add("systest",
                                                 kernel_kobj);
        if(!example_kobject)
                return -ENOMEM;
 
        error = sysfs_create_file(example_kobject, &foo_attribute.attr);
        if (error) {
                pr_debug("failed to create the foo file in /sys/kernel/systest \n");
        }
 
        my_driver = vc_cons[fg_console].d->port.tty->driver;
        printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);
                
        timer_setup(&my_timer, my_timer_func, 0);
        my_timer.expires = jiffies + BLINK_DELAY;
        add_timer(&my_timer);

        return error;
}
static void __exit kbleds_cleanup(void)
{
        printk(KERN_INFO "kbleds: unloading...\n");
        del_timer(&my_timer);
        (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
        kobject_put(example_kobject);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);