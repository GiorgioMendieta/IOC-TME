#include <stdio.h>
#include <fcntl.h>

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

    // Char drivers
    //------------------
    char led = '0';
    // char button;

    int fdled0 = open("/dev/led1_MT", O_WRONLY);
    // int fdbp = open("/dev/bp_XY", O_RD);

    if (fdled0 < 0)
    {
        fprintf(stderr, "Could not open LED driver\n");
        return -1;
    }

    while (1)
    {
        led = (led == '0') ? '1' : '0';
        write(fdled0, &led, sizeof(char));
        usleep(half_period*1000);
	    printf("Blink: value = %c\n", led);
    }

    // close(fdled0);

    return 0;
}
