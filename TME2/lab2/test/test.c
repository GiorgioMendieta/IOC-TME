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
    half_period = (period / 2)*1000;

    // Char drivers
    //------------------
    char led = '0';
    char button;

    int fdled0 = open("/dev/led1_MT", O_WRONLY);
    int fdbp = open("/dev/bouton_MT", O_RDONLY);

    if (fdled0 < 0)
    {
        fprintf(stderr, "Could not open LED driver\n");
        return -1;
    }

    if (fdbp < 0)
    {
        fprintf(stderr, "Could not open Button driver\n");
        return -1;
    }

    while (1)
    {
        if(button == '0')
        {
            led = (led == '0') ? '1' : '0';
            write(fdled0, &led, sizeof(char));
            usleep(half_period);
	        printf("Blink: value = %c\n", led);
            printf("Button off\n");
        }
        read(fdbp, &button, sizeof(char));
    }

    return 0;
}
