//------------------------------------------------------------------------------
// Driver headers
//------------------------------------------------------------------------------

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jorge MENDIETA, Lou THIRION");
MODULE_DESCRIPTION("Char driver pour contrôler une LED");

// static int major;

//------------------------------------------------------------------------------
// GPIO ACCESS
//------------------------------------------------------------------------------

// #define GPIO_LED0 4
// #define GPIO_LED1 17
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

// GPIO Write
static void
gpio_write(uint32_t pin, char val)
{
    // Since there are 54 GPIO, dividing by 32 gives us the register where the pin is located
    uint32_t reg = pin / 32;
    // Getting the modulo gives us the exact bit of the pin in the register
    uint32_t bit = pin % 32;

    if (val == '0')
    {
        // Set GPIO 1
        gpio_regs->gpset[reg] = (1 << bit);
    }
    else
    {
        // Clear GPIO 0
        gpio_regs->gpclr[reg] = (1 << bit);
    }
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
    value = gpio_regs->gplev[reg] & (1 << bit);

    return value;
}

// ------------------------------------------------
// Driver Functions & Parameters
// ------------------------------------------------
#define NBMAX_LED 32
static int leds[NBMAX_LED];
static int nbLed;

module_param_array(leds, int, &nbLed, 0);
MODULE_PARM_DESC(LEDS, "LED port array numbers");

static int
open_led_MT(struct inode *inode, struct file *file)
{
    printk(KERN_DEBUG "led1_MT : configuring LEDs as output\n");
    // GPIO Function select
    int i;
    for (i = 0; i < nbLed; i++)
    {
        gpio_fsel(leds[i], GPIO_FSEL_OUTPUT);
    }
    return 0;
}

static ssize_t
read_led_MT(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    printk(KERN_DEBUG "led1_MT : read()\n");
    // Read buf char from the LED given by count
    int val;
    val = gpio_read(count);
    return val;
}

static ssize_t
write_led_MT(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    printk(KERN_DEBUG "led1_MT : write()\n");
    // Write buf char to the LED given by count
    gpio_write(count, &buf);

    return count;
}

static int
release_led_MT(struct inode *inode, struct file *file)
{
    printk(KERN_DEBUG "led1_MT : close()\n");
    // Turn of fleds
    int i;
    for (i = 0; i < nbLed; i++)
    {
        gpio_write(leds[i], '0');
    }
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
    int major;
    major = register_chrdev(0, "led1_MT", &fops_led); // 0 est le numéro majeur qu'on laisse choisir par linux
    if (major < 0)
    {
        printk(KERN_WARNING "led1_MT : Probleme sur le major\n");
        return major;
    }

    printk(KERN_DEBUG "led1_MT : Driver chargé !\n");
    return 0;
}

static void __exit mon_module_cleanup(void)
{
    int ret;
    ret = unregister_chrdev(major, "led1_MT");

    if (ret < 0)
    {
        printk(KERN_WARNING "led1_MT : Probleme unregister\n");
    }
    printk(KERN_DEBUG "led1_MT : Driver déchargé avec succès!\n");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);