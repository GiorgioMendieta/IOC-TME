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

// Structure to send parameters to threads
struct blink_params
{
    int led;
    int half_period;
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
static void
gpio_write(uint32_t pin, uint32_t val)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;

    if (val == 1)
    {
        // Set GPIO 1
        gpio_regs_virt->gpset[reg] = (1 << bit);
    }
    else
    {
        // Clear GPIO 0
        gpio_regs_virt->gpclr[reg] = (1 << bit);
    }
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

int main(int argc, char **argv)
{
    // Initial setup
    /*-------------------------------------------*/
    int period1, half_period1;
    int period2, half_period2;

    // default = 1Hz (1000 ms)
    period1 = 1000;
    period2 = 1000;

    // Use same frequency for both leds
    if (argc == 2)
    {
        period1 = atoi(argv[1]);
        period2 = atoi(argv[1]);
    }
    // Use two different frequencies for leds
    if (argc == 3)
    {
        period1 = atoi(argv[1]);
        period2 = atoi(argv[2]);
    }
    // Half of the time on, the other half off
    half_period1 = period1 / 2;
    half_period2 = period2 / 2;

    // Map GPIO registers
    /*-------------------------------------------*/
    uint32_t volatile *gpio_base = 0;
    if (gpio_mmap((void **)&gpio_regs_virt) < 0)
    {
        printf("-- error: cannot setup mapped GPIO.\n");
        exit(1);
    }

    // Blink LEDs
    /*-------------------------------------------*/

    printf("-- info: start blinking.\n");

    // Create thread to blink another led
    // (https://hpc-tutorials.llnl.gov/posix/passing_args/)

    // Create a local structure and set the members' values
    struct blink_params blink1_args;
    blink1_args.led = GPIO_LED1;
    blink1_args.half_period = half_period1;

    // Create the thread and pass the structure containing the values as argument
    pthread_t thread1;
    int ret = pthread_create(&thread1, NULL, blink, (void *)&blink1_args);
    if (ret != 0)
    {
        // Join the thread in case of error
        printf("-- error: cannot create thread.\n");
        pthread_join(thread1, NULL);
    }

    // Create a local structure and set the members' values
    struct blink_params blink2_args;
    blink2_args.led = GPIO_LED0;
    blink2_args.half_period = half_period2;

    // Create the thread and pass the structure containing the values as argument
    pthread_t thread2;
    ret = pthread_create(&thread2, NULL, blink, (void *)&blink2_args);
    if (ret != 0)
    {
        // Join the thread in case of error
        printf("-- error: cannot create thread.\n");
        pthread_join(thread2, NULL);
    }

    // Join the threads
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}
