#include "i2cmanager.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

//the bit transmitted after the address determines
//read or write status in i2c
#define WRITE(addr)		((addr << 1) | 0)
#define READ(addr)		((addr << 1) | 1)

#define I2CMANAGER_MAX_TICK_WAIT (100 / portTICK_RATE_MS)
#define I2CMANAGER_ACK 1
#define I2CMANAGER_NACK 0

/**
 * INIT/DEINIT
 */
esp_err_t i2cmanager_init(gpio_num_t sda_gpio, gpio_num_t scl_gpio, gpio_pullup_t pullup_en, uint32_t stretch){
	esp_err_t err;
	const i2c_config_t cfg = {
		.mode 				= I2C_MODE_MASTER,
		.sda_io_num 		= sda_gpio,
		.sda_pullup_en		= pullup_en,
		.scl_io_num			= scl_gpio,
		.scl_pullup_en		= pullup_en,
		.clk_stretch_tick 	= stretch
	};
	err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER);
    ESP_LOGD(I2CMANAGER_TAG, "install %s:%d", ((err == 0) ? "succeeded" : "failed"), err);
    if(!err){ 		
		err = i2c_param_config(I2C_NUM_0, &cfg);
		ESP_LOGD(I2CMANAGER_TAG, "config %s:%d", ((err == 0) ? "succeeded" : "failed"), err);
	}
	return err;
}
void i2cmanager_deinit(void){
	i2c_driver_delete(I2C_NUM_0);
}
/**
 * INIT/DEINIT - END
 */
/**
 * WRITE/READ
 */
esp_err_t i2cmanager_write_byte(uint8_t address, uint8_t *data){
	esp_err_t err;
	
	i2c_cmd_handle_t handle = i2c_cmd_link_create();
	i2c_master_start(handle);
	i2c_master_write_byte(handle, WRITE(address), I2CMANAGER_ACK);
	i2c_master_write_byte(handle, *data, I2CMANAGER_ACK);
	i2c_master_stop(handle);
	err = i2c_master_cmd_begin(I2C_NUM_0, handle, I2CMANAGER_MAX_TICK_WAIT);
	ESP_LOGV(I2CMANAGER_TAG, "write %s:%d", ((err == 0) ? "succeeded" : "failed"), err);
	i2c_cmd_link_delete(handle);
	
	return err;
}

esp_err_t i2cmanager_read_byte(uint8_t address, uint8_t *data){
	esp_err_t err;
	
	i2c_cmd_handle_t handle = i2c_cmd_link_create();
	i2c_master_start(handle);
	i2c_master_write_byte(handle, READ(address), I2CMANAGER_ACK);
	i2c_master_read_byte(handle, data, I2C_MASTER_NACK);
	i2c_master_stop(handle);
	err = i2c_master_cmd_begin(I2C_NUM_0, handle, I2CMANAGER_MAX_TICK_WAIT);
	ESP_LOGV(I2CMANAGER_TAG, "read %s:%d", ((err == 0) ? "succeeded" : "failed"), err);
	i2c_cmd_link_delete(handle);
	
	return err;
}
/**
 * WRITE/READ - END
 */
/**
 * SEARCH
 */
esp_err_t i2cmanager_search_slave(uint8_t *address){
	//if no error occurs, address will be the first slave found or 0xFF
	esp_err_t err = ESP_OK;
	
	*address = 0x00;
	uint8_t slave_found = 0;
	while(*address <= 0x7F && slave_found == 0 && !err){
		i2c_cmd_handle_t handle = i2c_cmd_link_create();
		i2c_master_start(handle);
		i2c_master_write_byte(handle, WRITE(*address), I2CMANAGER_ACK);
		i2c_master_stop(handle);
		err = i2c_master_cmd_begin(I2C_NUM_0, handle, I2CMANAGER_MAX_TICK_WAIT);
		i2c_cmd_link_delete(handle);
		if(!err){
			slave_found = 1;
			ESP_LOGD(I2CMANAGER_TAG, "search %#x: found!", *address);
		}else if(err == ESP_FAIL){ //no slave in this address, reset error to stay in the cycle
			ESP_LOGV(I2CMANAGER_TAG, "search %#x:not found", *address);
			(*address)++;
			err = ESP_OK;
		}else //something went wrong
			ESP_LOGD(I2CMANAGER_TAG, "search error:%d", err);
	}
	if(slave_found == 0)
		*address = 0xFF;
	return err;
}
esp_err_t i2cmanager_search_all_slaves(uint8_t *array_address, uint8_t *array_max){
	esp_err_t err = ESP_OK;
	ESP_LOGI(I2CMANAGER_TAG, "search all slaves");
	
	uint8_t address = 0;
	uint8_t counter = 0;
	while(address <= 0x7F && !err){
		i2c_cmd_handle_t handle = i2c_cmd_link_create();
		i2c_master_start(handle);
		i2c_master_write_byte(handle, WRITE(address), I2CMANAGER_ACK);
		i2c_master_stop(handle);
		err = i2c_master_cmd_begin(I2C_NUM_0, handle, I2CMANAGER_MAX_TICK_WAIT);
		i2c_cmd_link_delete(handle);
		if(!err){
			if(counter < *array_max){
				ESP_LOGD(I2CMANAGER_TAG, "search %#x: found!", address);
				array_address[counter] = address;
				counter++;
			}else{
				err = ESP_FAIL;
				break;
			}
		}else if(err == ESP_FAIL){ //no slave in this address, reset error to stay in the cycle
			ESP_LOGV(I2CMANAGER_TAG, "search %#x:not found", address);
			err = ESP_OK;
		}else //something went wrong
			ESP_LOGD(I2CMANAGER_TAG, "search error:%d", err);
		address++;
	}
	*array_max = counter;
	return err;
}
/**
 * SEARCH - END
 */
