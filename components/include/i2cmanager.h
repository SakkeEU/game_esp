#ifndef _I2CMANAGER_H_
#define _I2CMANAGER_H_

#include <stdint.h>
#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"{
#endif

#define I2CMANAGER_TAG "i2cmanager"
/**
 * @brief I2C driver install & parameters configuration
 *
 * @param sda_gpio sda gpio number
 * @param scl_gpio scl gpio number
 * @param pullup_en enable/disable internal pullup
 * @param tick clock time strerch
 *
 * @return
 *     - ESP_OK   Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Driver install error
 */
esp_err_t i2cmanager_init(gpio_num_t sda_gpio, gpio_num_t scl_gpio, gpio_pullup_t pullup_en, uint32_t stretch);
/**
 * @brief I2C driver delete
 */
void i2cmanager_deinit(void);
/**
 * @brief write data to I2C bus
 *
 * @param address slave address
 * @param data data to write
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2cmanager_write_byte(uint8_t address, uint8_t *data);
/**
 * @brief read data from I2C bus
 *
 * @param address slave address
 * @param data read data container
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2cmanager_read_byte(uint8_t address, uint8_t *data);
/**
 * @brief search slave with lower address
 * 		  @note
 * 		  if return value is ESP_OK and slave address is 0xFF
 * 		  no slave was found.
 *
 * @param address slave address container
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2cmanager_search_slave(uint8_t *address);
/**
 * @brief search all slaves
 *
 * @param array_address slave addresses container
 * @param array_max array dimesion
 * 		  @note
 * 		  array_max is used to return the #n of slaves found
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL array_addres could not contain all the slaves found
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2cmanager_search_all_slaves(uint8_t *array_address, uint8_t *array_max); //NOTE:NOT FULLY TESTED
#ifdef __cplusplus
} //extern "C"
#endif
#endif //include guard
