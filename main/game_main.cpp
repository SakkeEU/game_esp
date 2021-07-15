#include <cstdint>
#include "Lcd1602.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#define MAIN_TAG "MAIN"
extern "C" {
	void app_main(void);
}
void app_main(void){
    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    vTaskDelay(5000/portTICK_RATE_MS);
    
    Lcd1602 lcd;
    lcd.init();
    lcd.start();
    lcd.backlight_on();
    
	for(;;){
		vTaskDelay(1000000/portTICK_RATE_MS);
	}
}
