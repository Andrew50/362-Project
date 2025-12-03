//////////////////////////////////////////////////////////////////////////
// BELOW IS TO DISPLAY MUSIC NOTES////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

#define WIDTH 32
#define HEIGHT 16
#define NUM_GROUPS 8 // 8 bars (4 pixels wide)
#define MAX_HEIGHT 16 // 32/2

// Matrix values correspond to the height of the bars
// 0: A, 1: B, 2: C, 3: D, 4: E, 5: F, 6: G, 7: volume
uint8_t bar_height[NUM_GROUPS];

// MAKE THIS A GLOBAL VARIABLE... REPLACE THE 0s WITH TEST VARIABLES FOR NOW
uint16_t vals[7] = {5000, 5000, 5000, 5000, 5000, 5000, 5000};

// GPIO pins
const int R1 = 2; // R1
const int B1 = 3; // B2
const int G1 = 4; // G1
const int R2 = 5; // R2
const int B2 = 6; // B2
const int G2 = 7; // G2

const int CLK_PIN = 8; // clock
const int OE_PIN  = 9; // output enable
const int LAT_PIN = 10; // latch

const int PIN_A = 11; // row address A
const int PIN_B = 12; // row address B
const int PIN_C = 13; // row address C
const int PIN_D = 14; // row address D

// Color displayed for each group
bool r_out = 0;
bool g_out = 0;
bool b_out = 0;

// Function list
int gpio_main(void);
void display_note(uint8_t row);
void set_row(uint8_t row);
void clock_pulse(void);
void latch_pulse(void);
void color_displayed(uint8_t group);
uint8_t map_sensor_to_height(uint16_t sensor_val);
void update_bar_heights(void);

int gpio_main(void) {
    stdio_init_all();

    // Inititalize all pins
    gpio_init(R1);
    gpio_init(B1);
    gpio_init(G1);
    gpio_init(R2);
    gpio_init(B2);
    gpio_init(G2);
    gpio_init(CLK_PIN);
    gpio_init(OE_PIN);
    gpio_init(LAT_PIN);
    gpio_init(PIN_A);
    gpio_init(PIN_B);
    gpio_init(PIN_C);
    gpio_init(PIN_D);

    // Set all GPIO pins as outputs
    gpio_set_dir(R1, GPIO_OUT);
    gpio_set_dir(B1, GPIO_OUT);
    gpio_set_dir(G1, GPIO_OUT);
    gpio_set_dir(R2, GPIO_OUT);
    gpio_set_dir(B2, GPIO_OUT);
    gpio_set_dir(G2, GPIO_OUT);
    gpio_set_dir(CLK_PIN, GPIO_OUT);
    gpio_set_dir(OE_PIN, GPIO_OUT);
    gpio_set_dir(LAT_PIN, GPIO_OUT);
    gpio_set_dir(PIN_A, GPIO_OUT);
    gpio_set_dir(PIN_B, GPIO_OUT);
    gpio_set_dir(PIN_C, GPIO_OUT);
    gpio_set_dir(PIN_D, GPIO_OUT);

    // Start with output disabled (active low)
    gpio_put(OE_PIN, 1);
    gpio_put(CLK_PIN, 0);
    gpio_put(LAT_PIN, 0);

    /* // TEST HEIGHTS CHANGE WHEN GIVEN SENSOR VALUES //////////////
    bar_height[0] = 3;
    bar_height[1] = 5;
    bar_height[2] = 7;
    bar_height[3] = 8;
    bar_height[4] = 4;
    bar_height[5] = 6;
    bar_height[6] = 8;
    bar_height[7] = 2;
    // TEST HEIGHTS CHANGE WHEN GIVEN SENSOR VALUES ////////////// */

    while (true) {
        update_bar_heights();

        for (uint8_t row = 0; row < HEIGHT; row++) {
            display_note(row);
        }
        // UPDATE BAR HEIGHT HERE WHEN GIVEN SENSOR VALUES
    }
}

// HELPER FUNCTIONS BELOW
void set_row(uint8_t row)
{
    gpio_put(PIN_A, (row >> 0) & 1);
    gpio_put(PIN_B, (row >> 1) & 1);
    gpio_put(PIN_C, (row >> 2) & 1);
    gpio_put(PIN_D, (row >> 3) & 1);
}

void clock_pulse(void)
{
    gpio_put(CLK_PIN, 1);
    gpio_put(CLK_PIN, 0);
}

void latch_pulse(void)
{
    gpio_put(LAT_PIN, 1);
    gpio_put(LAT_PIN, 0);
}

// Choose the color of each “note” bar
void color_displayed(uint8_t group) {
    // Reset
    r_out = 0;
    g_out = 0;
    b_out = 0;

    if (group == 0)  // Note A
    {
        r_out = 1; g_out = 0; b_out = 0; // red
    }
    else if (group == 1) // Note B
    {
        r_out = 1; g_out = 1; b_out = 0; // yellow
    }
    else if (group == 2) // Note C
    {
        r_out = 0; g_out = 1; b_out = 0; // green
    }
    else if (group == 3) // Note D
    {
        r_out = 0; g_out = 1; b_out = 1; // light blue
    }
    else if (group == 4) // Note E
    {
        r_out = 0; g_out = 0; b_out = 1; // dark blue
    }
    else if (group == 5) // Note F
    {
        r_out = 1; g_out = 0; b_out = 1; // purple
    }
    else if (group == 6) // Note G
    {
        r_out = 1; g_out = 0; b_out = 0; // red (again)
    }
    else if (group == 7) // Volume bar
    {
        r_out = 1; g_out = 1; b_out = 1; // white
    }
}

// Scan and display one row of the bars
void display_note(uint8_t row)
{
    // Disable display
    gpio_put(OE_PIN, 1);

    // Set rows
    set_row(row);

    // Top and bottom of the display mirror each other
    uint8_t dist_top = (HEIGHT - 1) - row; // RBG1
    uint8_t dist_bot = row; // RBG2

    // 32 pixels wide
    for (int col = 0; col < WIDTH; col++) {

        uint8_t group = col / 4; // Each bar is 4 pixels long

        bool top_on = false;
        bool bot_on = false;

        if (group < NUM_GROUPS) {
            uint8_t h = bar_height[group]; // 0-16

            // Know to light up the pixels in between the top bar height and center
            if (dist_top < h)
            {
                top_on = true;
            }
            // Know to light up the pixels in between the bottom bar height and center
            if (dist_bot < h)
            {
                bot_on = true;
            }
        }

        if (top_on || bot_on)
        {
            color_displayed(group);
        } 
        else
        {
            r_out = 0;
            g_out = 0;
            b_out = 0;
        }

        // Top half uses R1/G1/B1
        gpio_put(R1, top_on ? r_out : 0);
        gpio_put(G1, top_on ? g_out : 0);
        gpio_put(B1, top_on ? b_out : 0);

        // Bottom half uses R2/G2/B2 (mirror)
        gpio_put(R2, bot_on ? r_out : 0);
        gpio_put(G2, bot_on ? g_out : 0);
        gpio_put(B2, bot_on ? b_out : 0);

        clock_pulse();
    }

    // Pulse the latch output
    latch_pulse();

    // Turn the display on
    gpio_put(OE_PIN, 0);
    sleep_us(100);
}

uint8_t map_sensor_to_height(uint16_t sensor_val)
{
    const uint16_t SENSOR_OFF = 1700; // sensors are off if reading 1700 or less
    const uint16_t SENSOR_MAX = 4100; // largest value that can be read by the sensors
    const uint16_t ACTIVE_RANGE = SENSOR_MAX - SENSOR_OFF; // 2400 pixels create the "range"
    const uint16_t STEP = ACTIVE_RANGE / MAX_HEIGHT; // 2400 / 16 = 150

    // Below or equal to SENSOR_OFF means no pixels are lit
    if (sensor_val <= SENSOR_OFF) {
        return (0);
    }

    // Out of bounds case
    if (sensor_val > SENSOR_MAX) {
        sensor_val = SENSOR_MAX;
    }

    uint16_t active_val = sensor_val - SENSOR_OFF; // value in the 2400 range

    // Height of the bar based on the 16 150 pixel groups
    uint8_t height = active_val / STEP;

    // Out of bounds case
    if (height > MAX_HEIGHT) {
        height = MAX_HEIGHT;
    }

    return height;
}

void update_bar_heights(void)
{
    // 0-6 are notes A-G
    for (int i = 0; i < 7; i++) {
        uint16_t val = vals[i];
        bar_height[i] = map_sensor_to_height(val);
    }

    // 7 is the volume bar -- average of all 7 note sensor values
    uint32_t sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += vals[i];
    }
    uint16_t avg_sensor = (uint16_t)(sum / 7);
    bar_height[7] = map_sensor_to_height(avg_sensor);
}