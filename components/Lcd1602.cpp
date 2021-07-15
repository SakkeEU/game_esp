#include "Lcd1602.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
//mem
#define LCD_SET_CGRAM 0x40
#define LCD_SET_DDRAM 0x80
//misc
#define LCD_HOME 0x02
#define LCD_CLEAR_DISPLAY 0x01
//entry mode control
#define LCD_SHIF_DISPLAY_OFF 0x00
#define LCD_SHIFT_DISPLAY_ON 0x01
#define LCD_MOVE_LEFT 0x00
#define LCD_MOVE_RIGHT 0x02
#define LCD_ENTRY_MODE_CONTROL 0x04
//display control
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00
#define LCD_DISPLAY_CONTROL 0x08
//modes
#define LCD_COMMAND 0x00
#define LCD_WRITE 0x01
#define LCD_READ_BUSY 0x02
#define LCD_READ 0x03
#define LCD_BACKLIGHT_ON 0x08
#define LCD_BACKLIGHT_OFF 0x00
//setup control
#define LCD_SET_2LINES 0x08
#define LCD_FUNCTION_SET 0x20
//enable
#define LCD_ENABLE 0x04
#define LCD_NOT_ENABLE 0xFB

/**
 * USUAL STUFF 
 */
Lcd1602::Lcd1602(uint8_t address){
		address_ = address;
		backlight_ = LCD_BACKLIGHT_ON;
		err_ = ESP_OK;
}
Lcd1602::Lcd1602(){
		address_ = 0x00;
		backlight_ = LCD_BACKLIGHT_ON;
		err_ = ESP_OK;
}
Lcd1602::~Lcd1602(void){}
/**
 * USUAL STUFF - END 
 */
/**
 * INIT/DEINIT
 */
void Lcd1602::init(gpio_num_t sda_gpio, gpio_num_t scl_gpio, bool pullup_en, uint32_t stretch){
	//sugarcoating the pullup_en type. not really necessary.
	if(pullup_en)
		err_ = i2cmanager_init(sda_gpio, scl_gpio, GPIO_PULLUP_ENABLE, stretch);
	else
		err_ = i2cmanager_init(sda_gpio, scl_gpio, GPIO_PULLUP_DISABLE, stretch);
	ESP_LOGI(LCD1602_TAG, "%s", (!err_) ? "i2c initialization succeded!" : "i2c initialization not succeded!.");
	//no address slave? let's find it!
	if(address_ == 0x00 && err_ == ESP_OK){
		ESP_LOGI(LCD1602_TAG, "slave address not initialized.");
		ESP_LOGI(LCD1602_TAG, "searching for first slave available...");
		err_ = i2cmanager_search_slave(&address_);
		if(address_ != 0xFF && err_ == ESP_OK)
			ESP_LOGI(LCD1602_TAG, "slave found! address = %#x", address_);
		else
			ESP_LOGI(LCD1602_TAG, "no slave found.");
	}	
}
void Lcd1602::init(void){
	init(GPIO_NUM_4, GPIO_NUM_5, true);
}
void Lcd1602::deinit(void){
	i2cmanager_deinit();
}
void Lcd1602::start(void){
	//operation described in the HD44780U datasheet (pag.46)
	vTaskDelay(50/portTICK_RATE_MS); //wait for more than 40ms
	send_4bits(0x03 << 4);
	ESP_LOGD(LCD1602_TAG, "first init.");
	
	vTaskDelay(10/portTICK_RATE_MS); //wait for more than 4.1ms
	send_4bits(0x03 << 4);
	ESP_LOGD(LCD1602_TAG, "second init.");
	
	ets_delay_us(120);  //wait for more than 100us
	send_4bits(0x03 << 4);
	ESP_LOGD(LCD1602_TAG, "third init.");
	
	//set 4bit interface
	//vTaskDelay(50/portTICK_RATE_MS);
	send_4bits(0x02 << 4);
	ESP_LOGD(LCD1602_TAG, "interface set.");
	
	//(pag.42)
	send(LCD_FUNCTION_SET | LCD_SET_2LINES, LCD_COMMAND); //set function + n# lines 
	send(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF, LCD_COMMAND); //set display and cursor
	send(LCD_ENTRY_MODE_CONTROL | LCD_MOVE_RIGHT | LCD_SHIF_DISPLAY_OFF, LCD_COMMAND); //entry mode
	send(LCD_HOME, LCD_COMMAND); //home
	ESP_LOGI(LCD1602_TAG, "starting routine done.");
}
/*
 * INIT/DEINIT - END
 */
/*
 * COMMANDS
 */
//MEMO::::::
//MEMORIZE NEW CHARS IN CGRAM FROM POS 0<<3 T0 7<<3
//DISPLAY NEW CHARS FROM DDRAM POS 0 TO 7
//MEMO::::::
void Lcd1602::print_chr(uint8_t pos_on_screen, uint8_t chr_code){
	send(LCD_SET_DDRAM | pos_on_screen, LCD_COMMAND);
		send(chr_code, LCD_WRITE);
}
void Lcd1602::create_chr(uint8_t pos_in_mem, uint8_t *chr){
	send(LCD_SET_CGRAM | (pos_in_mem << 3), LCD_COMMAND);
	for(int i = 0; i < 8; i++)
		send(chr[i], LCD_WRITE);
}
void Lcd1602::backlight_on(void){
	backlight_ = LCD_BACKLIGHT_ON;
	send(0,0);
}
void Lcd1602::backlight_off(void){
	backlight_ = LCD_BACKLIGHT_OFF;
	send(0,0);
}
void Lcd1602::display_on(void){
	send(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON, LCD_COMMAND);
}
void Lcd1602::display_off(void){
	send(LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF, LCD_COMMAND);
}
void Lcd1602::cursor_on(void){
	send(LCD_DISPLAY_CONTROL | LCD_CURSOR_ON, LCD_COMMAND);
}
void Lcd1602::cursor_off(void){
	send(LCD_DISPLAY_CONTROL | LCD_CURSOR_OFF, LCD_COMMAND);
}
/*
 * COMMANDS - END
 */
/*
 * PRIVATE
 */
void Lcd1602::send(uint8_t word, uint8_t mode){
	uint8_t word_h = (word & 0xF0) | mode | backlight_;
	uint8_t word_l = ((word << 4) & 0xF0) | mode | backlight_;
	err_ = i2cmanager_write_byte(address_, &word_h);
	ESP_LOGV(LCD1602_TAG, "%s", (!err_) ? "send_h succeded!" : "send_h failed!");
	enable(word_h);
	err_ = i2cmanager_write_byte(address_, &word_l);
	ESP_LOGV(LCD1602_TAG, "%s", (!err_) ? "send_l succeded!" : "send_l failed!");
	enable(word_l);
}
void Lcd1602::send_4bits(uint8_t word){
	err_ = i2cmanager_write_byte(address_, &word);
	ESP_LOGV(LCD1602_TAG, "%s", (!err_) ? "send_4bit succeded!" : "send_4bit failed!");
	enable(word);
}
void Lcd1602::enable(uint8_t word){
	word = word | LCD_ENABLE;
	err_ = i2cmanager_write_byte(address_, &word);
	ESP_LOGV(LCD1602_TAG, "%s", (!err_) ? "enable_h succeded!" : "enable_h failed!");
	ets_delay_us(1); //enable pulse needs to be >450ns (pag.49)
	word = word & LCD_NOT_ENABLE;
	err_ = i2cmanager_write_byte(address_, &word);
	ESP_LOGV(LCD1602_TAG, "%s", (!err_) ? "enable_l succeded!" : "enable_l failed!");
	ets_delay_us(50); //commands need >37us to settle (pag.24)
}
bool Lcd1602::read_busy(uint8_t *address_counter){return false;}
uint8_t Lcd1602::read(void){return 0;}
/*
 * PRIVATE - END
 */
