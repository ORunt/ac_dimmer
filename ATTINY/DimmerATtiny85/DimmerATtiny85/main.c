/*
 * DimmerATtiny85.c
 *
 * Created: 2021/07/05 19:50:03
 * Author : Cameron
 */ 

#include "cams_attiny85_lib.h"

#define LIGHTS              3   // Set how many output lights are needed (1 - 3)

#define AC_DIM_MIN_PERCENT  20  // The min percent before the light stays off
#define AC_DIM_MAX_PERCENT  95  // The max percent before the light stays on

// The pin outs for the Light PWM and zero cross pin
// DO NOT USE: PB0 and PB2. I2C uses these pins
#define LIGHT_PIN_0         PB4
#define LIGHT_PIN_1         PB1
#define LIGHT_PIN_2         PB5
#define ZERO_CROSS_PIN      PB3

// The I2C packet structure : [0x6A (Address), light_number (0 - 2), dim_value (0 - 100)]
#define I2C_PACKET_SIZE     2    // excluding the address

typedef struct{
    uint8_t zero_cross;     // Set when a zero cross happens
    uint8_t dim_trans_buf;  // The current dim value
    uint8_t dim_buf;        // The next dim value
}light_store_t;


volatile light_store_t light_store[LIGHTS] = {0};


/*
 * Calculates the timer output compare value from the Dim percentage passed in
 * @param dim: Dim value between 0 (off) and 100 (max)
 */
uint8_t Calc_Dim_CCR(uint32_t dim)
{
    dim = (dim < 100) ? dim : 100;  // Check limits
    // 10ms is max brightness
    // uint32_t t_cnt_ns = (AC_DIM_PRESCALER + 1) * 1000000000 / SystemCoreClock; // In ns
    // return dim * 100000 / t_cnt_ns;

    return (uint8_t)(((100 - dim) * F_CPU) / (PRESCALER * 10000UL));
}


/*
 * Utility function that maps a counter value to the pin out value on port B
 */
uint8_t map_pin(uint8_t pin)
{
    switch(pin)
    {
        case 0: return LIGHT_PIN_0;
        case 1: return LIGHT_PIN_1;
        case 2: return LIGHT_PIN_2;
        default: return LIGHT_PIN_0;
    }
}


/*
 * Interrupt function for when an output compare on the timers happens, this sets the PWM duty cycle
 */
void isr_light(uint8_t num)
{
    if(light_store[num].zero_cross)
    {
        light_store[num].zero_cross = 0;

        if(light_store[num].dim_trans_buf > AC_DIM_MIN_PERCENT)
        {
            setPin(map_pin(num));
        }
    }
    
    if(light_store[num].dim_trans_buf != light_store[num].dim_buf)
    {
        light_store[num].dim_trans_buf = light_store[num].dim_buf;
        SetTimerCompare((TIMx_e)num, Calc_Dim_CCR(light_store[num].dim_trans_buf));
    }
}

/*
 * Interrupt function for when a zero cross gets triggered
 */
void isr_zeroCross(uint8_t num)
{
    int i;
    
    // Make sure if all zero cross's has been cleared (prevents multiple interrupts for same zero cross)
    for (i = 0; i < LIGHTS; i++){
        if(light_store[i].zero_cross){
            return;
        }
    }
    
    for (i = 0; i < LIGHTS; i++)
    {
        // Zero Cross just happened
        light_store[i].zero_cross = 1;

        // Turn TRIACs off if it shouldn't stay on
        if(light_store[i].dim_trans_buf < AC_DIM_MAX_PERCENT)
        {
            resetPin(map_pin(i));
        }
    }
    
    // Start the counter from 0 again
    ResetAllCounters();
}


/*
 * Initialize the timers for output compare
 */
void timer_init(void)
{
    int i;
    
    for(i = 0; i < LIGHTS; i++){
        InitialiseTimer((TIMx_e)i, isr_light);
    }
}


/*
 * Initialize the GPIO outputs that the PWM will output to
 */
void gpio_init(void)
{
    int i;
    
    for(i = 0; i < LIGHTS; i++){
        setPinOutput(map_pin(i));
        resetPin(map_pin(i));
        light_store[i].dim_buf = AC_DIM_MIN_PERCENT - 1;
    }
}


/*
 * Initialize the zero cross interrupt
 */
void exti_init(void)
{
    initialiseExternalInterrupt(ZERO_CROSS_PIN, isr_zeroCross);
}


int main(void)
{
    uint8_t buf[I2C_PACKET_SIZE] = {0};
    uint8_t len = 0;
    uint8_t light_val = 0;
        
    overclock();                        // Over-clock the system clock to 20Mhz
    gpio_init();                        // Initialize the GPIO outputs that the PWM will output to
    timer_init();                       // Initialize the timers for output compare
    exti_init();                        // Initialize the zero cross interrupt
    i2c_init();                         // Initialize the I2C comms
    enableGlobalInterrupts(true);       // Enable global interrupts
    
    while(1)
    {
        // If I2C data is being transmitted, we stay in this function until the whole packet is received
        len = i2c_receive_data(&buf[0], I2C_PACKET_SIZE);
        
        if(len == I2C_PACKET_SIZE)
        {
            if(buf[0] < LIGHTS) // Make sure we don't overflow the light_store array
            {
                light_val = (buf[1] > AC_DIM_MAX_PERCENT) ? AC_DIM_MAX_PERCENT + 1 : buf[1];    // Check for max - we don't want to exceed these, otherwise the interrupts might happen out of order
                light_val = (buf[1] < AC_DIM_MIN_PERCENT) ? AC_DIM_MIN_PERCENT - 1 : light_val; // Check for min - we don't want to exceed these, otherwise the interrupts might happen out of order
                light_store[buf[0]].dim_buf = light_val;                                        // The light_store is serviced in the interrupts
            }
        }
    }
}
