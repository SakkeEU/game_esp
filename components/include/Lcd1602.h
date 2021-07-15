#ifndef _LCD1602_H_
#define _LCD1602_H_

#include <cstdint>
#include "i2cmanager.h"
#include "esp_err.h"


#define LCD1602_TAG "lcd1602"

class Lcd1602{
	uint8_t address_;
	esp_err_t err_;
	uint8_t backlight_;
public:
	/**
	 * @brief class constructor
	 *
	 * @param address slave address
	 */
	Lcd1602(uint8_t address);
	/**
	 * @brief class constructor
	 *
	 */
	Lcd1602(void);
	/**
	 * @brief class destructor
	 */
	~Lcd1602(void);
	
	/**
	 * @brief setup I2C manager
	 * 		  @note
	 * 		  find slave if missing
	 *
	 * @param pullup_en internal pullup on/off
	 * @param tick clock time stretch
	 */
	void init(gpio_num_t sda_gpio, gpio_num_t scl_gpio, bool pullup_en, uint32_t stretch = 200);
	/**
	 * @brief setup I2C manager, find slave if missing
	 * 		  @note
	 * 		  find slave if missing.
	 * 		  default SDA port: GPIO_NUM_4
	 * 		  default SCL port: GPIO_NUM_5
	 * 		  default stretch: 200
	 *
	 */
	void init(void);
	/**
	 * @brief deinit I2C manager
	 */
	void deinit(void);
	/**
	 * @brief setup the LCD device according to the datasheet
	 */
	void start(void);
	/**
	 * @brief print character to screen
	 * 
	 * @param pos_on_screen Position on screen
	 * @param chr_code Character code in memory
	 *        @note
	 *        custom character are stored in with code 0 to 7
	 *
	 */
	void print_chr(uint8_t pos_on_screen, uint8_t chr_code);
	/**
	 * @brief save new character in memory for this session
	 *        @note
	 *        a maximum of 8 8x5 characters can be stored
	 *        at the same time
	 * 
	 * @param pos_in mem Position in memory: 0 to 7
	 * @param chr Character description (pag.19)
	 *
	 */
	void create_chr(uint8_t pos_in_mem, uint8_t *chr);
	void backlight_on(void);
	void backlight_off(void);
	void display_on(void);
	void display_off(void);
	void cursor_on(void);
	void cursor_off(void);
private:
	void send(uint8_t word, uint8_t mode);
	void send_4bits(uint8_t word);
	void enable(uint8_t word);
	bool read_busy(uint8_t *address_counter); //doesnt work
	uint8_t read(void); //doesnt work
};
#endif
