#include "mgos.h"
#include "mgos_i2c.h"


static struct mgos_i2c *s_global_i2c;


static void led_timer_cb(void *arg) {
  bool val = mgos_gpio_toggle(mgos_sys_config_get_pins_led());
  LOG(LL_INFO, ("%s uptime: %.2lf, RAM: %lu, %lu free", val ? "Tick" : "Tock",
                mgos_uptime(), (unsigned long) mgos_get_heap_size(),
                (unsigned long) mgos_get_free_heap_size()));
  (void) arg;
}

static void jimena(void *arg) {
  LOG(LL_INFO, ("JIMENA"));
  (void) arg;

}

/**
 * @brief i2c master initialization
 */
void i2c_master_init(void)
{
  // By default I2C is configured for 100kHz
  // mos.yml defines pinout for I2C
  // I2C lines configured with pull-ups
  struct mgos_i2c *i2c = mgos_i2c_get_global();   // gets pointer to the I2C master structure

  if (i2c == NULL) {
    LOG(LL_DEBUG, ("I2C not configured \n"));
  } else {
    LOG(LL_INFO, ("I2C is configured \n"));
  };
}

/**
 * @brief i2c write code
 *        Master device write data to slave
 */
bool i2c_master_write_slave(uint8_t dev_addr, uint8_t* data_wr, size_t size)
{
  uint8_t reg = *data_wr;
  struct mgos_i2c *i2c = mgos_i2c_get_global();   // gets pointer to the I2C master structure
  return mgos_i2c_write_reg_n(i2c, (uint16_t) dev_addr, reg, size, ++data_wr);
}

/**
 * @brief code to master to read from slave
 *
 */
bool i2c_master_read_slave(uint8_t dev_addr, uint8_t* data_rd, size_t size)
{
  uint8_t reg = *data_rd;
  struct mgos_i2c *i2c = mgos_i2c_get_global();   // gets pointer to the I2C master structure
  return mgos_i2c_read_reg_n(i2c, (uint16_t) dev_addr, reg, size, data_rd);
}

// example
//const struct mgos_config_i2c cfg = {
//  .enable: true,
//  .freq: 400,
//  .debug: 0,
//  .sda_gpio: 13,
//  .scl_gpio: 12,
//};

const struct mgos_config_i2c cfg = {true, 400, 0, 13, 12};

struct mgos_i2c *mgos_i2c_create(const struct mgos_config_i2c *cfg);

// mgos_sys_config_get_i2c(); --> type = const struct mgos_config_i2c *

enum mgos_app_init_result mgos_app_init(void) {

  i2c_master_init();
  mgos_set_timer(500, MGOS_TIMER_REPEAT, led_timer_cb, NULL);
  mgos_set_timer(500, MGOS_TIMER_REPEAT, jimena, NULL);

  s_global_i2c = mgos_i2c_create(mgos_sys_config_get_i2c());

  LOG(LL_INFO, ("--- freq %d", mgos_i2c_get_freq(s_global_i2c)));


  return MGOS_APP_INIT_SUCCESS;
}
