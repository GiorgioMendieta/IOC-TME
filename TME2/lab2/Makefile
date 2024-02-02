CARD_NUMB       = 25
ROUTER          = peri
LOGIN           = thirion_mendieta
LAB             = lab2

MODULE          = module

CROSSDIR        = /users/enseig/franck/E-IOC
# Répertoire du code source
KERNELDIR       = /dsk/l1/misc/linux-rpi-3.18.y2/linux-rpi-3.18.y
CROSS_COMPILE   = $(CROSSDIR)/arm-bcm2708hardfp-linux-gnueabi/bin/bcm2708hardfp-
        
obj-m           += $(MODULE).o
default:;       make -C $(KERNELDIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
clean:;         make -C $(KERNELDIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean
upload:;        scp -P622$(CARD_NUMB) $(MODULE).ko pi@$(ROUTER):$(LOGIN)/$(LAB)