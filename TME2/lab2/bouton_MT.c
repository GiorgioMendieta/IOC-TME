//------------------------------------------------------------------------------
// Driver headers
//------------------------------------------------------------------------------

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <mach/platform.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jorge MENDIETA, Lou THIRION");
MODULE_DESCRIPTION("Char driver pour lire un bouton");

static int major;

//------------------------------------------------------------------------------
// GPIO ACCESS
//------------------------------------------------------------------------------

// #define GPIO_PUSH_BUTTON 18

#define GPIO_FSEL_INPUT 0
#define GPIO_FSEL_OUTPUT 1

struct gpio_s
{
    uint32_t gpfsel[7];   // GPIO Function Select Registers (GPFSELn)
    uint32_t gpset[3];    // GPIO Pin Output Set Registers (GPSETn)
    uint32_t gpclr[3];    // GPIO Pin Output Clear Registers (GPCLRn)
    uint32_t gplev[3];    // GPIO Pin Level Registers (GPLEVn)
    uint32_t gpeds[3];    // GPIO Event Detect Status Registers (GPEDSn)
    uint32_t gpren[3];    // GPIO Rising Edge Detect Enable Registers (GPRENn)
    uint32_t gpfen[3];    // GPIO Falling Edge Detect Enable Registers (GPRENn)
    uint32_t gphen[3];    // GPIO High Detect Enable Registers (GPHENn)
    uint32_t gplen[3];    // GPIO Low Detect Enable Registers (GPLENn)
    uint32_t gparen[3];   // GPIO Asynchronous rising Edge Detect Enable Registers (GPARENn)
    uint32_t gpafen[3];   // GPIO Asynchronous Falling Edge Detect Enable Registers (GPAFENn)
    uint32_t gppud[1];    // GPIO Pull-up/down Register (GPPUD)
    uint32_t gppudclk[3]; // GPIO Pull-up/down Clock Registers (GPPUDCLKn)
    uint32_t test[1];
} volatile *gpio_regs = (struct gpio_s *)__io_address(GPIO_BASE);
/* Note: Volatile is used here to prevent optimizations from changing where the variable is stored,
we always want to store it in memory and not in registers */

// GPIO Function Select
static void
gpio_fsel(uint32_t pin, uint32_t fun)
{
    // Get the corresponding function select register
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs->gpfsel[reg] = (gpio_regs->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

// GPIO Read
static uint32_t
gpio_read(uint32_t pin)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;
    uint32_t value;

    // Read GPIO in the register at the specified bit
    // and mask the desired bit to read its value
    value = gpio_regs->gplev[reg] & (0x1 << bit);

    return value;
}

// ------------------------------------------------
// Driver Functions & Parameters
// ------------------------------------------------
#define NBMAX 32
static int boutons[NBMAX];
static int nbBouton;

module_param_array(boutons, int, &nbBouton, 0);
MODULE_PARM_DESC(boutons, "Button port array numbers");

static int
open_but_MT(struct inode *inode, struct file *file)
{
    int i;
    // GPIO Function select
    for (i = 0; i < nbBouton; i++)
    {
        gpio_fsel(boutons[i], GPIO_FSEL_INPUT);
        printk(KERN_DEBUG "bouton_MT : configuring GPIO %i as input\n", boutons[i]);
    }
    return 0;
}

static ssize_t
read_but_MT(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    int val;
    int i;

    for (i = 0; i < nbBouton; i++)
    {
        val = gpio_read(boutons[i]);
    }

    if(val == 1){
        printk(KERN_DEBUG "bouton_MT : Button pressed %i\n", val);
        buf = "1";
    }
    else{
        buf = "0";
    }

    return count;
}

static ssize_t
write_but_MT(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    printk(KERN_DEBUG "bouton_MT : write(%c)\n", *buf);
    
    // Write buf char to the LED given by count

    return 0;
}

static int
release_but_MT(struct inode *inode, struct file *file)
{
    printk(KERN_DEBUG "bouton_MT : close()\n");

    return 0;
}

struct file_operations fops_but =
    {
        .open = open_but_MT,
        .read = read_but_MT,
        .write = write_but_MT,
        .release = release_but_MT};

// ------------------------------------------------
// Init + Exit
// ------------------------------------------------
static int __init mon_module_init(void)
{
    major = register_chrdev(0, "bouton_MT", &fops_but); // 0 est le numéro majeur qu'on laisse choisir par linux

    printk(KERN_DEBUG "bouton_MT : Driver charge! (Major: %i)\n", major);
    return 0;
}

static void __exit mon_module_cleanup(void)
{
    unregister_chrdev(major, "bouton_MT");

    printk(KERN_DEBUG "bouton_MT : Driver decharge avec succes!\n");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);
