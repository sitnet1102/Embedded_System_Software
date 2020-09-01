#include "ku_common.h"


void tasklet_func(unsigned long recv_data) {
	struct ku_list *tmp;
	tmp = (struct ku_list *) recv_data;
	int note = 0;
	int i = 0;
	printk("LED ON\n");
	// turn on the led
	
	if(tmp->mode == 1) {
		note = 1911;
	} else if(tmp->mode == 2) {
		note = 1012;
	}
	
	if(note != 0) {
		gpio_set_value(LED1, 1);
	} else {
		gpio_set_value(LED1, 0);
	}

	for(i = 0; i < 100; i++) {
		gpio_set_value(SPEAKER, 1);
		udelay(note);
		gpio_set_value(SPEAKER, 0);
		udelay(note);
	}
}


static irqreturn_t tasklet_isr(int irq, void* data) {
	tasklet_schedule(&my_tasklet);
	return IRQ_HANDLED;
}


