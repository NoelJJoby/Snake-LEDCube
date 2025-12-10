#include "libopencm3/stm32/rcc.h"  //Needed to enable clocks for particular GPIO ports
#include "libopencm3/stm32/gpio.h" //Needed to define things on the GPIO
#include "libopencm3/stm32/adc.h"  //Needed to convert analogue signals to digital
#include "stdio.h"

int main(void)
{
    rcc_periph_clock_enable(RCC_GPIOA); // Enable clock for GPIO Port A
    rcc_periph_clock_enable(RCC_GPIOC); // Enable clock for GPIO Port C

    rcc_periph_clock_enable(RCC_ADC12); // Enable clock for ADC registers 1 and 2

    /// Joystick A
    // A1
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

    // A0
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);

    /// Joystick C
    // C1
    gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

    // C0
    gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);

    //// Setup ADC registers
    adc_power_off(ADC1); // Turn off ADC register 1 whist we set it up

    adc_set_clk_prescale(ADC1, ADC_CCR_CKMODE_DIV1);                   // Setup a scaling, none is fine for this
    adc_disable_external_trigger_regular(ADC1);                        // We don’t need to externally trigger the register...
    adc_set_right_aligned(ADC1);                                       // Make sure it is right aligned to get more usable values
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_61DOT5CYC); // Set up sample time
    adc_set_resolution(ADC1, ADC_CFGR1_RES_12_BIT);                    // Get a good resolution

    adc_power_on(ADC1); // Finished setup, turn on ADC register 1


    int joystickChannels[] = {1, 2, 6, 7};

    while (true)
    {
        // Code to read joystick input
        for (i = 0; i < 4; i++)
        {
            uint8_t channelArray[] = {joystickChannels[i]};                    // Define a channel that we want to look at
            adc_set_regular_sequence(ADC1, 1, channelArray); // Set up the channel
            adc_start_conversion_regular(ADC1);              // Start converting the analogue signal
            while (!(adc_eoc(ADC1)))
                ;                                    // Wait until the register is ready to read data
            uint32_t value = adc_read_regular(ADC1); // Read the value from the register and channel

            
            printf("Channel%d ", joystickChannels[i]);
            printf(" value is: %ld\n", value);
        }
    }

    return 0;
}