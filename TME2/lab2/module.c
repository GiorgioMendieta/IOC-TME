#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jorge MENDIETA, Lou THIRION");
MODULE_DESCRIPTION("Module pour contrôler une LED");

// ------------------------------------------------
// PARAMETERS
// ------------------------------------------------
// Button parameter
static int btn;
module_param(btn, int, 0);
MODULE_PARM_DESC(btn, "numéro du port du bouton");

// led parameter
#define NBMAX_LED 32
static int leds[NBMAX_LED];
static int nbled;
module_param_array(leds, int, &nbled, 0);
MODULE_PARM_DESC(LEDS, "tableau des numéros de port LED");


// ------------------------------------------------
// Init + Exit
// ------------------------------------------------
static int __init mon_module_init(void)
{
   printk(KERN_DEBUG "Hello Jorge MENDIETA, Lou THIRION !\n");
   printk(KERN_DEBUG "btn=%d !\n", btn);

   int i;
   for (i = 0; i < nbled; i++)
       printk(KERN_DEBUG "LED %d = %d\n", i, leds[i]);

   return 0;
}

static void __exit mon_module_cleanup(void)
{
   printk(KERN_DEBUG "Goodbye World!\n");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);
