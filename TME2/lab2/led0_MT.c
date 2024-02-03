#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jorge MENDIETA, Lou THIRION");
MODULE_DESCRIPTION("Module pour contrôler une LED");

static int major;

// ------------------------------------------------
// PARAMETERS
// ------------------------------------------------
#define NBMAX_LED 32
static int leds[NBMAX_LED];
static int nbled;
module_param_array(leds, int, &nbled, 0);
MODULE_PARM_DESC(LEDS, "tableau des numéros de port LED");

static int
open_led_MT(struct inode *inode, struct file *file)
{
    printk(KERN_DEBUG "led0_MT :open()\n");
    return 0;
}

static ssize_t
read_led_MT(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    printk(KERN_DEBUG "led0_MT :read()\n");
    return count;
}

static ssize_t
write_led_MT(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    printk(KERN_DEBUG "led0_MT :write()\n");
    return count;
}

static int
release_led_MT(struct inode *inode, struct file *file)
{
    printk(KERN_DEBUG "led0_MT :close()\n");
    return 0;
}

struct file_operations fops_led =
    {
        .open = open_led_MT,
        .read = read_led_MT,
        .write = write_led_MT,
        .release = release_led_MT};

// ------------------------------------------------
// Init + Exit
// ------------------------------------------------
static int __init mon_module_init(void)
{
    printk(KERN_DEBUG "led0_MT : Led module Jorge MENDIETA, Lou THIRION !\n");

    major = register_chrdev(0, "led0_MT", &fops_led); // 0 est le numéro majeur qu'on laisse choisir par linux

    int i;
    for (i = 0; i < nbled; i++)
        printk(KERN_DEBUG "led0_MT : %d = %d\n", i, leds[i]);

    return 0;
}

static void __exit mon_module_cleanup(void)
{
    printk(KERN_DEBUG "led0_MT : Au revoir, ma poule!\n");
    unregister_chrdev(major, "led0_MT");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);
