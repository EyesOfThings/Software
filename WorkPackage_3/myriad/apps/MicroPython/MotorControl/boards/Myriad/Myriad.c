/**
 * @file      Cherokey4WDMyriad.cpp
 * @copyright All code copyright Movidius Ltd 2012, all rights reserved 
 *            For License Warranty see: common/license.txt   
 *
 * Created on: 20 Oct 2015
 * Author: dexmont
 *
 */

// 1: Includes
// ----------------------------------------------------------------------------
#include <boards/Myriad/Myriad.h>
#include <rtems.h>

// 2: Class / Functions definitions
// ----------------------------------------------------------------------------

#ifdef MYRIAD

// Store the counter for the software pwm
u64 Myriad_SWPWM_timer_count = 0;

// Store the id of the timer used for software pwm
int Myriad_SWPWM_timer_id = 0;

// Store the number of software pwm used
u8 Myriad_SWPWM_pin_count = 0;

/**
 * Store the number of pwm pins used
 */
u8 Myriad_pwm_pin_count = 0;

// Store the software pwm data
Myriad_SWPWM_counter_data Myriad_SWPWM_pin[MYRIAD_SWPWM_MAX_PIN_COUNT];

/**
 * Store the data of the pwm pins
 */
Myriad_PWM_data Myriad_PWM_pins[MYRIAD_PWM_MAX_PIN_COUNT];

/**
 * Gets the PWM block associated to the pin, if the pin as no PWM block returns -1
 * @param pin_id
 * @return
 */
sint8 getPWMBlock(uint32 pin_id)
{
    sint8 block = -1;

    // Test the pin id for the different blocks
    switch (pin_id)
    {
        case 27:
            // Set the block
            block = 1;
            break;

        case 33:
            // Set the block
            block = 0;
            break;

        case 44:
            // Set the block
            block = 2;
            break;

        case 45:
            // Set the block
            block = 0;
            break;

        case 46:
            // Set the block
            block = 1;
            break;

        case 49:
            // Set the block
            block = 3;
            break;

        case 50:
            // Set the block
            block = 4;
            break;

        case 51:
            // Set the block
            block = 5;
            break;

        case 62:
            // Set the block
            block = 3;
            break;

        case 63:
            // Set the block
            block = 4;
            break;

        case 74:
            // Set the block
            block = 4;
            break;
        case 75:
            // Set the block
            block = 5;
            break;

        case 77:
            // Set the block
            block = 0;
            break;

        case 82:
            // Set the block
            block = 3;
            break;

        case 84:
            // Set the block
            block = 2;
            break;

        default:
            // Other pins do not support pwm
            block = -1;
            break;
    }

    return block;
}

sint8 getPinIndex(uint32 pin_id)
{
    sint8 index = -1;

    // Search on the registered pins
    for (int idx = 0; idx < Myriad_pwm_pin_count; ++idx)
    {
        // Test if the current pin is the desired
        if (Myriad_PWM_pins[idx].pin_id == pin_id)
        {
            // Store the index of the pin
            index = idx;
            break;
        }
    }

    return index;
}

/**
 * Sets the duty cycle for the specified PWM pin, the duty cycle is a value between 0 and 255
 * @param pin_id
 * @param duty_cyle
 */
void setPWMDutyCycle(uint32 pin_id, uint32 duty_cyle)
{
    // Get the cycle length in percent form
    float tick_high_ratio = duty_cyle / 256.0f;

    // Get the index of the pin on the pwm pin table
    sint8 pin_index = getPinIndex(pin_id);

    // Test if the pin is not registered
    if (pin_index < 0)
    {
        // Do nothing and return
        return;
    }

    // Get the count on high
    int tick_count_high = Myriad_PWM_pins[pin_index].pwm_count * tick_high_ratio;

    // Get the count on low
    int tick_count_low = Myriad_PWM_pins[pin_index].pwm_count - tick_count_high;

    // Test if there is hardware pwm
    if (Myriad_PWM_pins[pin_index].block > -1)
    {
        // Set the pwm values
        DrvGpioSetPwm(Myriad_PWM_pins[pin_index].block, 0, 0, tick_count_high, tick_count_low);

        // Enable the pwm
        DrvGpioEnPwm(Myriad_PWM_pins[pin_index].block, 1);
    }
    else
    {    // We are using software pwm

        // Look for the pin id
        for (int idx = 0; idx < Myriad_SWPWM_pin_count; idx++)
        {
            // Test if the pin id is the requested
            if (Myriad_SWPWM_pin[idx].pin_id == pin_id)
            {
                // Update the time on high and low
                Myriad_SWPWM_pin[idx].time_high = tick_count_high;
                Myriad_SWPWM_pin[idx].time_low = tick_count_low;

                // end the search
                break;
            }
        }
    }
}

void setPWMTimeHigh(uint32 pin_id, uint32 pwm_count)
{
    sint8 pin_index = getPinIndex(pin_id);

    // Test if the pin is not registered
    if (pin_index < 0)
    {
        // Do nothing and return
        return;
    }

    // Get the count on high
    int tick_count_high = pwm_count;

    // Get the count on low
    int tick_count_low = Myriad_PWM_pins[pin_index].pwm_count - tick_count_high;

    // Test if there is hardware pwm
    if (Myriad_PWM_pins[pin_index].block > -1)
    {
        // Set the pwm values
        DrvGpioSetPwm(Myriad_PWM_pins[pin_index].block, 0, 0, tick_count_high, tick_count_low);

        // Enable the pwm
        DrvGpioEnPwm(Myriad_PWM_pins[pin_index].block, 1);
    }
    else
    {    // We are using software pwm

        // Look for the pin id
        for (int idx = 0; idx < Myriad_SWPWM_pin_count; idx++)
        {
            // Test if the pin id is the requested
            if (Myriad_SWPWM_pin[idx].pin_id == pin_id)
            {
                // Update the time on high and low
                Myriad_SWPWM_pin[idx].time_high = tick_count_high;
                Myriad_SWPWM_pin[idx].time_low = tick_count_low;

                // end the search
                break;
            }
        }
    }
}

/**
 * Sets tick count for the period of the pwm for the specified pin. When used on hardware pwm pins no update is performed until the duty cycle is reset
 * @param pin_id
 * @param period
 */
void setPWMPeriod(uint32 pin_id, uint64 period)
{
    // Get the index of the pin
    sint8 pin_index = getPinIndex(pin_id);

    // Test if the pin is not registered
    if (pin_index < 0)
    {
        // Do nothing and return
        return;
    }

    // Update the period length
    Myriad_PWM_pins[pin_index].pwm_count = period;
}

void setPinAsSWPWM(uint32 pin_id, uint32 pwm_count)
{

#ifdef BOARD_DEBUG
    printf("setting pin %d as software pwm\n", pin_id);
#endif

    // Test for the maximum amount of sw pwms
    if (Myriad_SWPWM_pin_count < MYRIAD_SWPWM_MAX_PIN_COUNT)
    {
        // Test if no interrupt has been setup
        if (Myriad_SWPWM_pin_count == 0)
        {
            // Test if the current interrupt level is higher than the requested
            if (swcLeonGetPIL() >= MYRIAD_SWPWM_IRQ_PRIORITY)
            {
                // Update the current interrupt level
                swcLeonSetPIL(MYRIAD_SWPWM_IRQ_PRIORITY - 1);
            }

            // Setup the timer to call our handler function
            DrvTimerStartOperation(MYRIAD_SWPWM_IRQ_INTERVAL, MyriadSWPWMIRQHandle, REPEAT_FOREVER,
            MYRIAD_SWPWM_IRQ_PRIORITY, &Myriad_SWPWM_timer_id);
        }

        // Get the index of the pin if any (test if it is already registered)
        int pin_index = getPinIndex(pin_id);

        // Test if the pin is not already registered
        if (pin_index < 0)
        {
            // Reset the variables for the pin
            Myriad_SWPWM_pin[Myriad_SWPWM_pin_count].pin_id = pin_id;
            Myriad_SWPWM_pin[Myriad_SWPWM_pin_count].expire_time = 0;
            Myriad_SWPWM_pin[Myriad_SWPWM_pin_count].time_high = 0;
            Myriad_SWPWM_pin[Myriad_SWPWM_pin_count].time_low = 0;
            Myriad_SWPWM_pin[Myriad_SWPWM_pin_count].state = 0;

            // Increase the count of used pins
            Myriad_SWPWM_pin_count++;
        }
    }
    else
    {
        printf("Maximum number of software pwm pins reached\n");
    }
}

/**
 * Interrupt handler for software pwm
 * @param timerNumber
 * @param param2
 * @return
 */
u32 MyriadSWPWMIRQHandle(u32 timerNumber, u32 param2)
{
    u32 idx;

    // Increase the timer count
    Myriad_SWPWM_timer_count++;

    // Process each swpwm pin
    for (idx = 0; idx < Myriad_SWPWM_pin_count; idx++)
    {
        // Test if the waiting time has expired
        if (Myriad_SWPWM_timer_count > Myriad_SWPWM_pin[idx].expire_time)
        {
            // Toggle the state value
            Myriad_SWPWM_pin[idx].state = 1 - Myriad_SWPWM_pin[idx].state;

            // Toggle according to the state
            if (Myriad_SWPWM_pin[idx].state == 0)
            {
                // Set the pin as low
                DrvGpioSetPinLo(Myriad_SWPWM_pin[idx].pin_id);

                // Update the expire time
                Myriad_SWPWM_pin[idx].expire_time = Myriad_SWPWM_timer_count + Myriad_SWPWM_pin[idx].time_low;
            }
            else
            {
                // Set the pin as high
                DrvGpioSetPinHi(Myriad_SWPWM_pin[idx].pin_id);

                // Update the expire time
                Myriad_SWPWM_pin[idx].expire_time = Myriad_SWPWM_timer_count + Myriad_SWPWM_pin[idx].time_high;
            }
        }
    }

    return 0;
}

void pinMode(uint32 pin_id, uint32 mode)
{
    // Set the pin mode
    DrvGpioSetMode(pin_id, mode);
}

uint32 digitalRead(uint32 pin_id)
{
    uint32 state;

    // Set the pin to the given state
    state = DrvGpioGetPin(pin_id);

    //TODO unregister the pin if it was being used as pwm

#ifdef BOARD_DEBUG
    printf("Reading pin %d\n", pin_id);
#endif

    return state;
}

void digitalWrite(uint32 pin_id, uint32 state)
{
    // Set the pin to the given state
    DrvGpioSetPin(pin_id, state);

    //TODO unregister the pin if it was being used as pwm

#ifdef BOARD_DEBUG
    printf("setting pin %d to state %d\n", pin_id, state);
#endif
}

sint32 getPinModePWM(uint32 pin_id)
{
    sint32 mode = -1;

    // Test the pin id, each possible pin could require a different mode
    switch (pin_id)
    {
        case 27:
            // Set the mode
            mode = D_GPIO_MODE_5;
            break;

        case 33:
            // Set the mode
            mode = D_GPIO_MODE_3;
            break;

        case 44:
            // Set the mode
            mode = D_GPIO_MODE_5;
            break;

        case 45:
            // Set the mode
            mode = D_GPIO_MODE_2;
            break;

        case 46:
            // Set the mode
            mode = D_GPIO_MODE_2;
            break;

        case 49:
            // Set the mode
            mode = D_GPIO_MODE_3;
            break;

        case 50:
            // Set the mode
            mode = D_GPIO_MODE_3;
            break;

        case 51:
            // Set the mode
            mode = D_GPIO_MODE_3;
            break;

        case 62:
            // Set the mode
            mode = D_GPIO_MODE_4;
            break;

        case 63:
            // Set the mode
            mode = D_GPIO_MODE_4;
            break;

        case 74:
            // Set the mode
            mode = D_GPIO_MODE_5;
            break;

        case 75:
            // Set the mode
            mode = D_GPIO_MODE_5;
            break;

        case 77:
            // Set the mode
            mode = D_GPIO_MODE_1;
            break;

        case 82:
            // Set the mode
            mode = D_GPIO_MODE_4;
            break;

        case 84:
            // Set the mode
            mode = D_GPIO_MODE_1;
            break;

        default:
            // set the mode as invalid
            mode = -1;

            break;
    }

    return mode;
}

uint32 registerPinPWM(uint32 pin_id)
{
    uint32 status = ERROR;

    // Test if the pin is already registered
    if (getPinIndex(pin_id) > -1)
    {
        // Set the status as error
        status = ERROR;
    }
    else if (Myriad_pwm_pin_count < MYRIAD_PWM_MAX_PIN_COUNT)    // Test if there is still some space on the array
    {
        // Store the data of the pin
        Myriad_PWM_pins[Myriad_pwm_pin_count].pin_id = pin_id;
        Myriad_PWM_pins[Myriad_pwm_pin_count].pwm_count = MYRIAD_HWPWM_DEFAULT_TICK_COUNT;
        Myriad_PWM_pins[Myriad_pwm_pin_count].block = getPWMBlock(pin_id);

        // Test if no block was found (no hardware pwm exists on that pin)
        if (Myriad_PWM_pins[Myriad_pwm_pin_count].block < 0)
        {
            // Create a software pwm pin
            setPinAsSWPWM(pin_id, MYRIAD_SWPWM_DEFAULT_TICK_COUNT);
        }
        else
        {
            // Set the pin mode
            DrvGpioSetMode(pin_id, D_GPIO_DIR_OUT | getPinModePWM(pin_id));
        }

        // Increase the count of registered pins
        Myriad_pwm_pin_count++;
    }

    return status;
}

uint32 registerPinSWPWM(uint32 pin_id, uint32 pwm_count)
{
    uint32 status = ERROR;

    // Test if the pin is already registered
    if (getPinIndex(pin_id) > -1)
    {
        // Set the status as error
        status = ERROR;
    }
    else if (Myriad_pwm_pin_count < MYRIAD_PWM_MAX_PIN_COUNT)    // Test if there is still some space on the array
    {
        // Store the data of the pin
        Myriad_PWM_pins[Myriad_pwm_pin_count].pin_id = pin_id;
        Myriad_PWM_pins[Myriad_pwm_pin_count].pwm_count = pwm_count;
        Myriad_PWM_pins[Myriad_pwm_pin_count].block = -1;

        // Create a software pwm pin
        setPinAsSWPWM(pin_id, pwm_count);

        // Increase the count of registered pins
        Myriad_pwm_pin_count++;
    }

    // Set the pin as output
    DrvGpioSetMode(pin_id, D_GPIO_DIR_OUT | D_GPIO_MODE_7);

    return status;
}

void analogWrite(uint32 pin_id, uint32 value)
{
    // Test if the pin is not already registered
    if (getPinIndex(pin_id) < 0)
    {
        // Register the pin
        registerPinPWM(pin_id);
    }

    // Update the duty cycle
    setPWMDutyCycle(pin_id, value);
}

#endif
