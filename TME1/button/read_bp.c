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
// GPIO ACCES
//------------------------------------------------------------------------------

#define BCM2835_PERIPH_BASE 0x20000000
#define BCM2835_GPIO_BASE (BCM2835_PERIPH_BASE + 0x200000)

#define GPIO_LED0 4
#define GPIO_LED1 17
#define GPIO_PUSH_BUTTON 18

#define GPIO_FSEL_INPUT 0
#define GPIO_FSEL_OUTPUT 1

// There is one extra element in the array for reserved space in the address space for each configuration register
struct gpio_s
{
    uint32_t gpfsel[7];
    uint32_t gpset[3]; // GPIO Pin Output Set Registers (GPSETn)
    uint32_t gpclr[3]; // GPIO Pin Output Clear Registers (GPCLRn)
    uint32_t gplev[3]; // GPIO Pin Level Registers (GPLEVn)
    uint32_t gpeds[3];
    uint32_t gpren[3];
    uint32_t gpfen[3];
    uint32_t gphen[3];
    uint32_t gplen[3];
    uint32_t gparen[3];
    uint32_t gpafen[3];
    uint32_t gppud[1];
    uint32_t gppudclk[3];
    uint32_t test[1];
};

// Structure to send parameters to threads
// struct blink_params
// {
//     int led;
//     int half_period;
// };

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

// void *blink(void *threadarg)
// {
//     // Each thread receives a unique instance of the structure
//     struct blink_params *args;
//     args = (struct blink_params *)threadarg;
//     int led = args->led;
//     int half_period = args->half_period;

//     // Setup GPIO to output
//     gpio_fsel(led, GPIO_FSEL_OUTPUT);

//     // Begin alternating the value
//     uint32_t is_on = 0;
//     while (1)
//     {
//         gpio_write(led, is_on);
//         delay(half_period);
//         is_on = 1 - is_on; // is_on = not is_on
//     }
// }

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

    // Setup GPIO of Push Button to input
    gpio_fsel(GPIO_PUSH_BUTTON, GPIO_FSEL_INPUT);

    // Blink led at default frequency of 1Hz
    printf("-- info: start reading value of button.\n");

    // Begin alternating the value
    uint32_t button_status = 0;
    while (1)
    {
        button_status = gpio_read(GPIO_PUSH_BUTTON);
        delay(half_period);
    }

    return 0;
}
