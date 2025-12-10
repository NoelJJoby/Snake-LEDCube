#include "libopencm3/stm32/rcc.h"  //Needed to enable clocks for particular GPIO ports
#include "libopencm3/stm32/gpio.h" //Needed to define things on the GPIO
#include "libopencm3/stm32/adc.h"  //Needed to convert analogue signals to digital
#include "libopencm3/stm32/usart.h"
#include <stdio.h>

#define JOYSTICK_A_PORT GPIOA
#define JOYSTICK_B_PORT GPIOC
#define JOYSTICK_A_DIR_1_PIN GPIO1
#define JOYSTICK_A_DIR_2_PIN GPIO0
#define JOYSTICK_B_DIR_1_PIN GPIO1
#define JOYSTICK_B_DIR_2_PIN GPIO0

#define LEDCUBE_PORT GPIOB
#define VCC_PIN GPIO5
#define TX_PIN GPIO6
#define RX_PIN GPIO7
#define USART_PORT USART1

#define MAX_SNAKE_LENGTH 512

typedef struct
{
    int x;
    int y;
    int z;
} Vector3;

struct Snake
{
    int len;
    int ate;
    Vector3 parts[MAX_SNAKE_LENGTH];
};

struct Food
{
    int ate;
    Vector3 pos;
};

//
int cube_setup(void);
int joystick_setup(void);
uint32_t joystick_read(int);
void update_joystick_input(void);

void send_grid_to_cube(void);
void reset_grid(void);
void draw_snake_to_grid(void);
void move_snake(int x, int y, int z);
void set_led_in_grid(Vector3 pos);
void randomize_food_loc(void);
void check_ate(void);
void update_snake_and_food(void);

int isVector3Equal(Vector3 pos1, Vector3 pos2);

int joystick_channels[] = {1, 2, 6, 7};
int direction_input[4];
/*
0  -- Y-axis -1=up, 1=down
1  -- X-axis -1=left, 1=right
2  -- Z-axis -1=forward, 1=backward
4  -- unused yet
*/

struct Snake snake;
struct Food food;
__uint8_t grid[8][8];

// Cube Specific Functions

int joystick_setup(void)
{
    // clocks for joysticks
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_ADC12); // Enable clock for ADC registers 1 and 2

    /// Joystick A
    // Direction 1 Pin
    gpio_mode_setup(JOYSTICK_A_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, JOYSTICK_A_DIR_1_PIN);

    // Direction 2 Pin
    gpio_mode_setup(JOYSTICK_A_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, JOYSTICK_A_DIR_2_PIN);

    /// Joystick B
    // Direction 1 Pin
    gpio_mode_setup(JOYSTICK_B_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, JOYSTICK_B_DIR_1_PIN);

    // Direction 2 Pin
    gpio_mode_setup(JOYSTICK_B_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, JOYSTICK_B_DIR_2_PIN);

    //// Setup ADC registers
    adc_power_off(ADC1); // Turn off ADC register 1 whist we set it up

    adc_set_clk_prescale(ADC1, ADC_CCR_CKMODE_DIV1);                   // Setup a scaling, none is fine for this
    adc_disable_external_trigger_regular(ADC1);                        // We don’t need to externally trigger the register...
    adc_set_right_aligned(ADC1);                                       // Make sure it is right aligned to get more usable values
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_61DOT5CYC); // Set up sample time
    adc_set_resolution(ADC1, ADC_CFGR1_RES_12_BIT);                    // Get a good resolution

    adc_power_on(ADC1); // Finished setup, turn on ADC register 1
    return 0;
}

uint32_t joystick_read(int channel)
{
    // Joystick A Right (up-down) 1 958-3019 4000 press
    // Joystick A Right  (left-right) 2  954-3030
    // Joystick B LEFT   (up-down) 6 981-3002 4000 press
    // Joystick B LEFt   (left -right) 7 942-3030

    uint8_t channelArray[] = {channel};              // Define a channel that we want to look at
    adc_set_regular_sequence(ADC1, 1, channelArray); // Set up the channel
    adc_start_conversion_regular(ADC1);              // Start converting the analogue signal
    while (!(adc_eoc(ADC1)))
        ;                          // Wait until the register is ready to read data
    return adc_read_regular(ADC1); // Read the value from the register and channel
}

void update_joystick_input()
{
    int value;
    for (int i = 0; i < 4; i++)
    {
        value = joystick_read(joystick_channels[i]);
        value = (value - 1000) / 1000;
        if (value < 0.5)
        {
            direction_input[i] = -1;
        }
        else if (value > 1.5)
        {
            direction_input[i] = 1;
        }
        else
        {
            direction_input[i] = 0;
        }
    }
}

int cube_setup(void)
{

    // clocks for cube
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_mode_setup(LEDCUBE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, VCC_PIN);
    // GPIO Port Name, GPIO Mode, GPIO Push Up Pull Down Mode, GPIO Pin Number
    gpio_set_output_options(LEDCUBE_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, VCC_PIN);
    // GPIO Port Name, GPIO Pin Driver Type, GPIO Pin Speed, GPIO Pin Number

    // Setting the VCC_PIN to be high always
    gpio_set(LEDCUBE_PORT, VCC_PIN);

    // TX Pin
    gpio_mode_setup(LEDCUBE_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, TX_PIN);
    gpio_set_af(LEDCUBE_PORT, GPIO_AF7, TX_PIN);

    // RX Pin
    gpio_mode_setup(LEDCUBE_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, RX_PIN);
    gpio_set_af(LEDCUBE_PORT, GPIO_AF7, RX_PIN);

    // Setting USART Parameters
    usart_set_baudrate(USART_PORT, 9600);
    usart_set_databits(USART_PORT, 8);
    usart_set_stopbits(USART_PORT, USART_STOPBITS_1);
    usart_set_mode(USART_PORT, USART_MODE_TX_RX);
    usart_set_parity(USART_PORT, USART_PARITY_NONE);
    usart_set_flow_control(USART_PORT, USART_FLOWCONTROL_NONE);
    usart_enable_rx_interrupt(USART_PORT);
    usart_enable_tx_interrupt(USART_PORT);
    usart_enable(USART_PORT);

    return 0;
}

// Snake Game Functions

void send_grid_to_cube(void)
{
    usart_send_blocking(USART_PORT, 0xF2);
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            usart_send_blocking(USART_PORT, grid[y][x]);
        }
    }
}

void reset_grid(void)
{
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            grid[x][y] = 0;
        }
    }
}

void set_led_in_grid(Vector3 pos)
{
    grid[pos.y][pos.x] += (1 << pos.z);
}

void draw_snake_to_grid(void)
{
    for (int i = 0; i < snake.len; i++)
    {
        set_led_in_grid(snake.parts[i]);
    }
}

void move_snake(int x, int y, int z)
{
    for (int i = snake.len - 1; i > 0; i--)
    {
        snake.parts[i] = snake.parts[i - 1];
    }
    snake.parts[0].x += x;
    snake.parts[0].y += y;
    snake.parts[0].z -= z;
}

void randomize_food_loc()
{
    int InSnakeBody = 1;
    while (InSnakeBody)
    {
        InSnakeBody = 0;
        food.pos.x = rand() % 8;
        food.pos.y = rand() % 8;
        food.pos.z = rand() % 8;
        for (int i = 0; i < snake.len; i++)
        {
            if (isVector3Equal(food.pos, snake.parts[i]))
            {
                InSnakeBody = 1;
            }
        }
    }
}

int isVector3Equal(Vector3 pos1, Vector3 pos2)
{
    if (pos1.x != pos2.x)
    {
        return 0;
    }

    if (pos1.y != pos2.y)
    {
        return 0;
    }

    if (pos1.z != pos2.z)
    {
        return 0;
    }

    return 1;
}

void check_ate()
{
    if (isVector3Equal(food.pos, snake.parts[0]))
    {
        snake.ate = 1;
    }
}

void update_snake_and_food()
{
    if (snake.ate == 1)
    {
        snake.len += 1;
        randomize_food_loc();
        snake.ate = 0;
    }
}

// Need to add Death Mec

int main(void)
{

    joystick_setup();
    cube_setup();

    snake.len = 3;
    snake.ate = 0;
    snake.parts[0].x = 3;
    snake.parts[0].y = 3;
    snake.parts[0].z = 0;

    snake.parts[1].x = 3;
    snake.parts[1].y = 4;
    snake.parts[1].z = 0;

    snake.parts[2].x = 3;
    snake.parts[2].y = 4;
    snake.parts[2].z = 1;

    randomize_food_loc();

    while (true)
    {
        update_joystick_input();

        check_ate();
        update_snake_and_food();

        move_snake(direction_input[1], direction_input[0], direction_input[2]);

        reset_grid();
        draw_snake_to_grid();
        send_grid_to_cube();
    }
}
