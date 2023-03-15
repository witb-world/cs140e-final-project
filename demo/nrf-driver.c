#include "nrf.h"
#include "nrf-hw-support.h"

// enable crc, enable 2 byte
#   define set_bit(x) (1<<(x))
#   define enable_crc      set_bit(3)
#   define crc_two_byte    set_bit(2)
#   define mask_int         set_bit(6)|set_bit(5)|set_bit(4)
enum {
    // pre-computed: can write into NRF_CONFIG to enable TX.
    tx_config = enable_crc | crc_two_byte | set_bit(PWR_UP) | mask_int,
    // pre-computed: can write into NRF_CONFIG to enable RX.
    rx_config = tx_config  | set_bit(PRIM_RX)
} ;

nrf_t * nrf_init(nrf_conf_t c, uint32_t rxaddr, unsigned acked_p) {

    // allocate nrf_t struct on heap
    nrf_t *n = kmalloc(sizeof *n);
    n->config = c;
    nrf_stat_start(n);
    n->spi = pin_init(c.ce_pin, c.spi_chip);
    n->rxaddr = rxaddr;
    
    // start in power off mode
    nrf_set_pwrup_off(n);
    // first, set config
    // flush TX and RX FIFOs
    nrf_rx_flush(n);
    nrf_tx_flush(n);

    // set channel
    nrf_put8(n, NRF_RF_CH, c.channel);

    // setup address field width (58)
    nrf_put8(n, NRF_SETUP_AW, nrf_default_addr_nbytes - 2); // 0b01 for 3, 0b10 for 4, etc.

    // rf settings (58-59)
    unsigned rf_pwr = nrf_default_db << 1; 
    nrf_put8(n, NRF_RF_SETUP, rf_pwr | nrf_default_data_rate);
    unsigned rf_retr = ((nrf_default_retran_delay / 250)<<4) | nrf_default_retran_attempts;
    printk("RETR setup value: %b\n", rf_retr);
    nrf_put8(n, NRF_SETUP_RETR, rf_retr); // 2000 usec, 6 retries (58)
                                               
    // if acked_p, set up other pipes
    if (acked_p) {
      nrf_put8(n, NRF_EN_AA, 0b111111); // first 6 bits for all pipes (57)
      nrf_put8(n, NRF_EN_RXADDR, 0b111111);
      nrf_put8(n, NRF_RX_PW_P0, c.nbytes);
      nrf_put8(n, NRF_RX_PW_P2, c.nbytes);
      nrf_put8(n, NRF_RX_PW_P3, c.nbytes);
      nrf_put8(n, NRF_RX_PW_P4, c.nbytes);
      nrf_put8(n, NRF_RX_PW_P5, c.nbytes);
      nrf_set_addr(n, NRF_RX_ADDR_P0, n->rxaddr, nrf_default_addr_nbytes);
    } else {
      // otherwise, just set up pipe 1
      // reset value on NRF_EN_AA is 1: clear this out (57).
      nrf_put8(n, NRF_EN_AA, 0); 
      nrf_put8(n, NRF_EN_RXADDR, 0b10); // enable second bit for rx (57)                                
    }
    nrf_put8(n, NRF_RX_PW_P1, c.nbytes);
    // set address
    nrf_set_addr(n, NRF_RX_ADDR_P1, n->rxaddr, nrf_default_addr_nbytes);
    nrf_put8(n, NRF_CONFIG, rx_config);
    // clear interrupts
    nrf_rt_intr_clr(n);
    // finally, power on
    nrf_set_pwrup_on(n);

    gpio_set_on(c.ce_pin);
    // should be true after setup.
    if(acked_p) {
        nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
        nrf_opt_assert(n, nrf_pipe_is_enabled(n, 0));
        nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
        nrf_opt_assert(n, nrf_pipe_is_acked(n, 0));
        nrf_opt_assert(n, nrf_pipe_is_acked(n, 1));
        nrf_opt_assert(n, nrf_tx_fifo_empty(n));
    } else {
        nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
        nrf_opt_assert(n, !nrf_pipe_is_enabled(n, 0));
        nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
        nrf_opt_assert(n, !nrf_pipe_is_acked(n, 1));
        nrf_opt_assert(n, nrf_tx_fifo_empty(n));
    }
    return n;
}

int nrf_tx_send_ack(nrf_t *n, uint32_t txaddr, 
    const void *msg, unsigned nbytes) {

    // default config for acked state.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    nrf_opt_assert(n, nrf_pipe_is_enabled(n, 0));
    nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
    nrf_opt_assert(n, nrf_pipe_is_acked(n, 0));
    nrf_opt_assert(n, nrf_pipe_is_acked(n, 1));
    nrf_opt_assert(n, nrf_tx_fifo_empty(n));

    // if interrupts not enabled: make sure we check for packets.
    while(nrf_get_pkts(n))
        ;

    gpio_set_off(n->config.ce_pin);
    delay_us(200);

    nrf_bit_clr(n, NRF_CONFIG, 0); //PRIM_RX at bit zero (57)
    // set TX addr
    nrf_set_addr(n, NRF_TX_ADDR, txaddr, nrf_default_addr_nbytes);
    // set p0 pipe to send address
    uint32_t old_p0_addr = nrf_get_addr(n, NRF_RX_ADDR_P0, nrf_default_addr_nbytes);
    nrf_set_addr(n, NRF_RX_ADDR_P0, txaddr, nrf_default_addr_nbytes);

    // set payload
    nrf_putn(n, NRF_W_TX_PAYLOAD, msg, nbytes);

    // pulse CE
    gpio_set_on(n->config.ce_pin);

    delay_us(30);

    // wait until TX FIFO is empty
    // TX FIFO empty bit is bit #4, value is 0 if there is data (61)
    while (!nrf_tx_fifo_empty(n)) { /*spin*/ } 

    // check for success with TX interrupt
    int success = nrf_has_tx_intr(n);
    nrf_tx_intr_clr(n);

    // check for failure with RT interrupt
    int failure = nrf_has_max_rt_intr(n);
    nrf_rt_intr_clr(n);
    if (failure || !success) {
      panic("Failed transmission!\n");
    }

    gpio_set_off(n->config.ce_pin);
    // clear TX interrupts
    nrf_tx_intr_clr(n);
    // nrf_opt_assert(n, nrf_tx_fifo_empty(n));

    gpio_set_on(n->config.ce_pin);
    nrf_set_addr(n, NRF_RX_ADDR_P0, old_p0_addr, nrf_default_addr_nbytes);
    nrf_bit_set(n, NRF_CONFIG, 0); // set PRIM_RX back 
    delay_us(200);

    // tx interrupt better be cleared.
    nrf_opt_assert(n, !nrf_has_tx_intr(n));
    // better be back in rx mode.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    return nbytes;
}

int nrf_tx_send_noack(nrf_t *n, uint32_t txaddr, 
    const void *msg, unsigned nbytes) {

    // default state for no-ack config.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    nrf_opt_assert(n, !nrf_pipe_is_enabled(n, 0));
    nrf_opt_assert(n, nrf_pipe_is_enabled(n, 1));
    nrf_opt_assert(n, !nrf_pipe_is_acked(n, 1));
    nrf_opt_assert(n, nrf_tx_fifo_empty(n));

    // if interrupts not enabled: make sure we check for packets.
    while(nrf_get_pkts(n))
        ;

    gpio_set_off(n->config.ce_pin);
    delay_us(200);

    // set to TX mode: RX_PRIM to zero
    nrf_bit_clr(n, NRF_CONFIG, 0); //PRIM_RX at bit zero (57)
    // set TX addr
    nrf_set_addr(n, NRF_TX_ADDR, txaddr, nrf_default_addr_nbytes);

    // set payload
    nrf_putn(n, NRF_W_TX_PAYLOAD, msg, nbytes);

    // pulse CE
    gpio_set_on(n->config.ce_pin);

    delay_us(130);

    // wait until TX FIFO is empty
    // TX FIFO empty bit is bit #4, value is 0 if there is data (61)
    while (!nrf_bit_isset(n, NRF_FIFO_STATUS, 4)) { /*spin*/ } 

    gpio_set_off(n->config.ce_pin);
    // clear TX interrupts
    nrf_tx_intr_clr(n);
    // nrf_opt_assert(n, nrf_tx_fifo_empty(n));

    gpio_set_on(n->config.ce_pin);
    nrf_bit_set(n, NRF_CONFIG, 0); // set PRIM_RX back 
    delay_us(200);

    // tx interrupt better be cleared.
    nrf_opt_assert(n, !nrf_has_tx_intr(n));
    // better be back in rx mode.
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    return nbytes;
}

int nrf_get_pkts(nrf_t *n) {
    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);

    // data sheet gives the sequence to follow to get packets.
    // p63: 
    //    1. read packet through spi.
    //    2. clear IRQ.
    //    3. read fifo status to see if more packets: 
    //       if so, repeat from (1) --- we need to do this now in case
    //       a packet arrives b/n (1) and (2)
    // done when: nrf_rx_fifo_empty(n)

    int res = 0;
    //gpio_set_on(n->config.ce_pin);
   // nrf_bit_set(n, NRF_CONFIG, 0); // set PRIM_RX back 
   // delay_us(200);
    uint32_t bytes[32];
    // while FIFO not empty (set to 1 if empty - pg 61)
    while(!nrf_rx_fifo_empty(n)) {
      assert(nrf_rx_get_pipeid(n)==1);
      // read packet through SPI
      nrf_getn(n, NRF_R_RX_PAYLOAD, bytes, n->config.nbytes); 
      // assert this is meant for pipe 1
      cq_push_n(&(n->recvq), bytes, n->config.nbytes);

      // clear interrupts
      nrf_rx_intr_clr(n);
      
      // increment number of packets 
      res++;
    }

    nrf_opt_assert(n, nrf_get8(n, NRF_CONFIG) == rx_config);
    return res;
}
