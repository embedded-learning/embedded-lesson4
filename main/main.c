#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/task.h"
#include "soc/adc_channel.h"
#include "esp_err.h"

#define ADC_CHANNEL ADC1_GPIO1_CHANNEL

void errorHandler(esp_err_t status);

void app_main(void) {
    adc_oneshot_unit_handle_t adcUnit;
    adc_cali_handle_t adcHandle;

    adc_oneshot_unit_init_cfg_t unitConfig = {.unit_id = ADC_UNIT_1 };
    adc_oneshot_chan_cfg_t channelConfig = { .atten=ADC_ATTEN_DB_12, .bitwidth=ADC_BITWIDTH_DEFAULT};
    adc_cali_curve_fitting_config_t caliConfig = { .unit_id=unitConfig.unit_id, .atten=channelConfig.atten, .bitwidth=channelConfig.bitwidth};

    ESP_ERROR_CHECK(adc_oneshot_new_unit( &unitConfig, &adcUnit));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adcUnit, ADC_CHANNEL, &channelConfig));
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&caliConfig, &adcHandle));

    while (1) {
        int outRaw;
        int calibratedVoltage;

        esp_err_t adcReadStatus =  adc_oneshot_read(adcUnit, ADC_CHANNEL, &outRaw);
        esp_err_t adcCaliStatus = adc_cali_raw_to_voltage(adcHandle,outRaw, &calibratedVoltage);

        if(adcReadStatus != ESP_OK ) {
            errorHandler(adcReadStatus);
            continue;
        } else if (adcCaliStatus != ESP_OK) {
            errorHandler(adcCaliStatus);
            continue;
        }
        float convertedVoltageValue = (outRaw / 4095.0) * 3.1;
        float fromMvtoV = calibratedVoltage / 1000.00;
        float compareResult = (fromMvtoV - convertedVoltageValue) /  convertedVoltageValue * 100;
        printf("ADC Value= %d, Voltage Value= %f, Calibrated Voltage = %.3f, Compre=%.1f %%\n", outRaw,convertedVoltageValue, fromMvtoV, compareResult);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

}

void errorHandler(esp_err_t status){
    printf("%s\n", esp_err_to_name(status));
    vTaskDelay(pdMS_TO_TICKS(10));
}

