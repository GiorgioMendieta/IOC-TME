//------------------------------------------------------------------------------
// Headers that are required for printf and mmap
//------------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pthread.h> // Threads POSIX

//------------------------------------------------------------------------------
// GPIO ACCESS
//------------------------------------------------------------------------------

#define BCM2835_PERIPH_BASE 0x20000000
#define BCM2835_GPIO_BASE (BCM2835_PERIPH_BASE + 0x200000)

#define GPIO_LED0 4
#define GPIO_LED1 17
#define GPIO_PUSH_BUTTON 18

#define GPIO_FSEL_INPUT 0
#define GPIO_FSEL_OUTPUT 1

// Note: There is one extra element in the array for reserved space in the address space for each configuration register
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
};

// Structures to send parameters to threads
struct blink_params
{
    int led;
    int half_period;
};

struct button_params
{
    int pin;
};


// Note: Volatile is used here to prevent optimizations from changing where the variable is stored, we always want
// to store it in memory and not in registers
volatile struct gpio_s *gpio_regs_virt;

// GPIO Function Select
static void
gpio_fsel(uint32_t pin, uint32_t fun)
{
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs_virt->gpfsel[reg] = (gpio_regs_virt->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

// GPIO Write
static void gpio_write(uint32_t pin, uint32_t val)
{
    // Since there are 54 GPIO, dividing by 32 gives us the register where the pin is located
    uint32_t reg = pin / 32;
    // Getting the modulo gives us the exact bit of the pin in the register
    uint32_t bit = pin % 32;

    if (val == 1)
    {
        // Set GPIO: Store 1 in the Set register at the specified bit
        gpio_regs_virt->gpset[reg] = (1 << bit);
    }
    else
    {
        // Clear GPIO: Store 1 in the Clear register at the specified bit
        gpio_regs_virt->gpclr[reg] = (1 << bit);
    }
}

// GPIO Read
static uint32_t gpio_read(uint32_t pin)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;
    uint32_t value;
    // Read GPIO in the register at the specified bit
    // and mask the desired bit to read its value
    value = gpio_regs_virt->gplev[reg] & (1 << bit);

    return value;
}

//------------------------------------------------------------------------------
// Access to memory-mapped I/O
//------------------------------------------------------------------------------

#define RPI_PAGE_SIZE 4096
#define RPI_BLOCK_SIZE 4096

static int mmap_fd;

static int
gpio_mmap(void **ptr)
{
    void *mmap_result;

    mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (mmap_fd < 0)
    {
        return -1;
    }

    mmap_result = mmap(NULL, RPI_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, BCM2835_GPIO_BASE);

    if (mmap_result == MAP_FAILED)
    {
        close(mmap_fd);
        return -1;
    }

    *ptr = mmap_result;

    return 0;
}

void gpio_munmap(void *ptr)
{
    munmap(ptr, RPI_BLOCK_SIZE);
}

//------------------------------------------------------------------------------
// Main Programm
//------------------------------------------------------------------------------

void delay(unsigned int milisec)
{
    struct timespec ts, dummy;
    ts.tv_sec = (time_t)milisec / 1000;
    ts.tv_nsec = (long)(milisec % 1000) * 1000000;
    nanosleep(&ts, &dummy);
}

void *blink(void *threadarg)
{
    // Each thread receives a unique instance of the structure
    struct blink_params *args;
    args = (struct blink_params *)threadarg;
    int led = args->led;
    int half_period = args->half_period;

    // Setup GPIO to output
    gpio_fsel(led, GPIO_FSEL_OUTPUT);

    // Begin alternating the value
    uint32_t is_on = 0;
    while (1)
    {
        gpio_write(led, is_on);
        delay(half_period);
        is_on = 1 - is_on; // is_on = not is_on
    }
}

uint32_t BP_ON = 0;   // mis à 1 si le bouton a été appuyé, mis à 0 quand la tâche qui attend l'appui a vu l'appui
uint32_t BP_OFF = 0;  // mis à 1 si le bouton a été relâché, mis à 0 quand la tâche qui attend le relâchement a vu le relâchement

void *read_button(void *threadarg)
{

    // Each thread receives a unique instance of the structure
    struct button_params *args;
    args = (struct button_params *)threadarg;
    uint32_t pin = args->pin;

    // Setup GPIO of Push Button to input
    gpio_fsel(pin, GPIO_FSEL_INPUT);

    uint32_t old_value = 1;
    uint32_t new_value = 1;
    while(1){
        delay(60);
        new_value = gpio_read(GPIO_PUSH_BUTTON);
        if (old_value != new_value)
        {   // changement d'état
            if(new_value == 0)
            {       
                // appui détecté 
                BP_ON = 1;
            }
            else
            {                          
                // relâchement détecté
                BP_OFF = 1;
            }
            old_value = new_value;
        }
    }
}

void led_state()
{
    enum FSM
    {
        OFF,
        IN_PHASE,
        OUT_OF_PHASE
    }state, next_state;

    state = OFF;
    next_state = OFF;

    if(state == OFF)
    {
        gpio_write(GPIO_LED0, 0);
        next_state = IN_PHASE
    }
}

int main(int argc, char **argv)
{
    // Get args
    int period, half_period;

    period = 1000; /* default = 1Hz (1000 ms) */
    if (argc > 1)
    {
        period = atoi(argv[1]);
    }
    // Half of the time on, the other half off
    half_period = period / 2;
    uint32_t volatile *gpio_base = 0;

    // Map GPIO registers
    if (gpio_mmap((void **)&gpio_regs_virt) < 0)
    {
        printf("-- error: cannot setup mapped GPIO.\n");
        exit(1);
    }

    // Setup GPIO of LED0 to output
    gpio_fsel(GPIO_LED0, GPIO_FSEL_OUTPUT);

    // Create a local structure and set the members' values
    struct button_params button1_args;
    button1_args.pin = GPIO_PUSH_BUTTON;

    // Create the thread and pass the structure containing the values as argument
    pthread_t thread_button;
    int ret = pthread_create(&thread_button, NULL, read_button, (void *)&button1_args);
    if (ret != 0)
    {
        // Join the thread in case of error
        printf("-- error: cannot create thread.\n");
        pthread_join(thread_button, NULL);
    }

    struct blink_params blink1_args;
    blink1_args.led = GPIO_LED1;
    blink1_args.half_period = half_period;

    // Create the thread and pass the structure containing the values as argument
    pthread_t thread_led;
    ret = pthread_create(&thread_led, NULL, blink, (void *)&blink1_args);
    if (ret != 0)
    {
        // Join the thread in case of error
        printf("-- error: cannot create thread.\n");
        pthread_join(thread_led, NULL);
    }

    // Blink led at default frequency of 1Hz
    printf("-- info: start reading value of button.\n");
    uint32_t led_on = 0;
    while(1)
    {
        if(BP_ON){
            BP_ON = 0;
            gpio_write(GPIO_LED0, led_on);
            led_on = 1 - led_on;
        }
        if(BP_OFF){
            BP_OFF = 0;
        }
    }
    
    pthread_join(thread_button, NULL);
    pthread_join(thread_led, NULL);

    return 0;
}
