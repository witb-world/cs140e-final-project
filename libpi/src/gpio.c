/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0     = (GPIO_BASE + 0x1C),
    gpio_clr0     = (GPIO_BASE + 0x28),
    gpio_lev0     = (GPIO_BASE + 0x34),

    gpio_hen0     = (GPIO_BASE + 0x64), // pin high detect enable 
    gpio_hen1     = (GPIO_BASE + 0x68), // pin high detect enable
    
    gpio_len0     = (GPIO_BASE + 0x70), // pin low detect enable  
    gpio_len1     = (GPIO_BASE + 0x74), // pin low detect enable  

    // async rising/falling edge detection
    gpio_gparen0  = (GPIO_BASE + 0x7c), // rising edge detect
    gpio_gparen1  = (GPIO_BASE + 0x80), // rising edge detect
                                        
    gpio_gpafen0  = (GPIO_BASE + 0x88), // falling edge detect
    gpio_gpafen1  = (GPIO_BASE + 0x8C), // falling edge detect
                                        
    gpio_ppud     = (GPIO_BASE + 0x94),
    gpio_ppudclk0 = (GPIO_BASE + 0x98)
};


//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    if(pin >= 32)
        return;

    unsigned fsel_addr = GPIO_BASE + (pin / 10) * 4; // GPFSEL[N] - see BCM2835 page 90.
    unsigned gpio_set_values = GET32(fsel_addr); 
    // gpio_set_values
    unsigned mask = 0x7 << ((pin % 10) * 3); // clear 3 bits, shifted by the pin #
    gpio_set_values &= ~mask;
    gpio_set_values |= (1 << (pin%10) * 3);
    PUT32(fsel_addr, gpio_set_values);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin >= 32)
        return;
  // implement this
  // use <gpio*_set0>
  PUT32(gpio_set0 + (pin / 32) * 4, 1 << (pin % 32));
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32)
        return;
  // implement this
  // use <gpio_clr0>
  PUT32(gpio_clr0 + (pin / 32) * 4, 1 << (pin % 32));
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
  // implement.
    if(pin >= 32)
        return;

    unsigned fsel_addr = GPIO_BASE + (pin / 10) * 4; // GPFSEL[N] - see BCM2835 page 90.
    unsigned gpio_set_values = GET32(fsel_addr); 
    // gpio_set_values
    unsigned mask = 0x7 << ((pin % 10) * 3); // clear 3 bits, shifted by the pin #
    gpio_set_values &= ~mask;
    // bits already cleared by the mask; we write zero to set input, so we're good now.
    PUT32(fsel_addr, gpio_set_values);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  if(pin >= 32 && (pin != 47)) {
    return DEV_VAL32(-1);
  }  
  unsigned v = 0;

  // implement.
  // get level for these GPIO pins
  v = GET32(gpio_lev0 + (pin / 32) * 4);
  // then, apply mask
  unsigned mask = (1 << pin);
  v &= mask;

  // return v >> pin;
  return DEV_VAL32(v >> pin);
}

void gpio_set_pullup(unsigned pin) {
  // dev_barrier(); // extra caution: use memory barrier before and after.
  if(pin >= 32)
    return;

  // step numbers in the following refer to BCM2835 manual, page 101.
  PUT32(gpio_ppud, 0x2); // (1): write "Enable" control sig to GPPUD

  delay_cycles(450); // (2): wait 450 clock cycles

  // (3): this step has us writing to GPPUDCLK0/1 to control the clock signal
  PUT32(gpio_ppudclk0, 1 << pin); // write `1` to pin on ppudclk

  delay_cycles(450); // (4): wait another 450 clock cycles.

  // We skip step (5)
  PUT32(gpio_ppudclk0, 0); // write `1` to pin on ppudclk to clear
  //dev_barrier(); // extra caution: use memory barrier before and after.
}

void gpio_set_pulldown(unsigned pin) {
  if(pin >= 32)
    return;
  //dev_barrier(); // extra caution: use memory barrier before and after.

  // step numbers in the following refer to BCM2835 manual, page 101.
  PUT32(gpio_ppud, 0x1); // (1): write "Enable" control sig to GPPUD with pulldown code 0b01.

  delay_cycles(450); // (2): wait 450 clock cycles

  // (3): this step has us writing to GPPUDCLK0/1 to control the clock signal
  PUT32(gpio_ppudclk0, 1 << pin); // write `1` to pin on ppudclk

  delay_cycles(450); // (4): wait another 450 clock cycles.

  // (6): this step has us writing to GPPUDCLK0/1 to control the clock signal
  PUT32(gpio_ppudclk0, 0); // write `1` to pin on ppudclk

  //dev_barrier(); // extra caution: use memory barrier before and after.

}

void gpio_pud_off(unsigned pin){
  //dev_barrier(); // extra caution: use memory barrier before and after.

  // step numbers in the following refer to BCM2835 manual, page 101.
  PUT32(gpio_ppud, 0x0); // (1): write "Disable" control sig to GPPUD

  delay_cycles(450); // (2): wait 450 clock cycles

  // (3): this step has us writing to GPPUDCLK0/1 to control the clock signal
  PUT32(gpio_ppudclk0, 1 << pin); // write `1` to pin on ppudclk

  delay_cycles(450); // (4): wait another 450 clock cycles.

  // (6): this step has us writing to GPPUDCLK0/1 to control the clock signal
  PUT32(gpio_ppudclk0, 0); // write `1` to pin on ppudclk

  //dev_barrier(); // extra caution: use memory barrier before and after.

}


void gpio_set_function(unsigned pin, gpio_func_t fn) {
  // note to self: this is copied from gpio_set_output... 
  if(pin >= 32 && pin != 47)
    return;
  if (fn > 7 || fn < 0)
    return;

  unsigned fsel_addr = GPIO_BASE + (pin / 10) * 4; // GPFSEL[N] - see BCM2835 page 90.
  unsigned gpio_set_values = GET32(fsel_addr); 
  // gpio_set_values
  unsigned mask = 0x7 << ((pin % 10) * 3); // clear 3 bits, shifted by the pin #
  gpio_set_values &= ~mask;
  gpio_set_values |= (fn << (pin%10) * 3);
  PUT32(fsel_addr, gpio_set_values);
}

// enable rising edge detection on `pin`
// TODO: review if we enable IRQs here or elsewhere.
 // void gpio_int_rising_edge(unsigned pin) {
 //   put32(Enable_IRQs_2, GPIO_INT0 % 32);
 //   if (pin < 32)
 //     or32(gpio_hen0, 1 << pin);
 //   else if (pin <= 53)
 //     or32(gpio_hen1, 1 << (pin % 32));
 //   else
 //     return; 
 // }
 // 
 // void gpio_int_falling_edge(unsigned pin) {
 //   put32(Enable_IRQs_2, GPIO_INT0 % 32);
 //   if (pin < 32)
 //     or32(gpio_len0, 1 << pin);
 //   else if (pin <= 53)
 //     or32(gpio_len1, 1 << (pin % 32));
 //   else
 //     return; 
 // }
 // 
 // int gpio_event_detected(unsigned pin) {
 //   if (pin < 32)
 //     unsigned lev = get32(gpio_gplev0);
 //   else if (pin <= 53)
 //     unsigned lev = get32(gpio_gplev1);
 //   else
 //     return; 
 // 
 //   return lev & (1 << (pin % 32));
 // }
 // 
 // void gpio_event_clear(unsigned pin) {
 //   if (pin < 32)
 //     put32(gpio_gplev0, 1 << pin);
 //   else if (pin <= 53)
 //     put32(gpio_gplev1, 1 << (pin %32));
 //   else
 //     return;
 // }
