/* 
 * Dekatron spinner for the OG-4 tube. The code provides some functions for
 * incrementing, reading and writing the dekatron. Recommended target: LPC1768.
 * 
 * Stelian Saracut 2023
 */

#include "mbed.h"

#define G1 p22 // first guide electrode
#define G2 p21 // second guide electrode
#define K0 p23 // connect to current sense resistor for cathode 0

#define PULSE_RATE 50ms
#define PULSE_WIDTH_US 250
#define SETTLING_TIME_US 250
#define MAX_ATTEMPTS 100
#define CATHODE_COUNT 10

// increment / decrement the dekatron
void count(DigitalOut& g1, DigitalOut& g2, const bool forward = true)
{
    if (forward)
    {
        g1 = 1;
        wait_us(PULSE_WIDTH_US);
        g2 = 1;
        wait_us(PULSE_WIDTH_US);
        g1 = 0;
        wait_us(PULSE_WIDTH_US);
        g2 = 0;
        wait_us(PULSE_WIDTH_US);
    } else
    {
        g2 = 1;
        wait_us(PULSE_WIDTH_US);
        g1 = 1;
        wait_us(PULSE_WIDTH_US);
        g2 = 0;
        wait_us(PULSE_WIDTH_US);
        g1 = 0;
        wait_us(PULSE_WIDTH_US);
    }
}

// set the dekatron to 0
// returns 0 on success, 1 on failure
uint32_t reset(DigitalOut& g1, DigitalOut& g2, DigitalIn& k0)
{
    uint32_t error = 1;

    for (uint32_t i = 0; i < MAX_ATTEMPTS; i++)
    {
        count(g1, g2);

        wait_us(SETTLING_TIME_US);

        if (k0)
        {
            error = 0;

            break;
        }
    }
    return error;
}

// set the dekatron to a given value
// returns 0 on success, 1 on failure
uint32_t set(DigitalOut& g1, DigitalOut& g2, DigitalIn& k0, const uint32_t number)
{
    uint32_t error = 1;

    error = reset(g1, g2, k0);
    if (!error)
    {
        for (uint32_t i = 0; i < number; i++)
        {
            count(g1, g2);
        }
    }

    return error;
}

// read the dekatron by incrementing it until it reaches 0
// returns dekatron value on success, -1 on failure
int32_t destructiveRead(DigitalOut& g1, DigitalOut& g2, DigitalIn& k0)
{
    int32_t value = 0;
    uint32_t error = 1;

    for (value = 0; value < MAX_ATTEMPTS; value++)
    {
        count(g1, g2);

        wait_us(SETTLING_TIME_US);

        if (k0)
        {
            error = 0;

            break;
        }
    }

    if (error)
    {
        value = -1;
    } else
    {
        value = (CATHODE_COUNT - value - 1) % CATHODE_COUNT;
    }

    return value;
}

// read the dekatron by incrementing it until it reaches 0
// write back the read value so the read appears to be non-destructive
// returns dekatron value on success, -1 on failure
int32_t read(DigitalOut& g1, DigitalOut& g2, DigitalIn& k0)
{
    int32_t returnValue = -1;
    int32_t reading = -1;

    reading = destructiveRead(g1, g2, k0);
    if (reading != -1)
    {
        returnValue = set(g1, g2, k0, reading);
    }

    if (returnValue != -1)
    {
        returnValue = reading;
    }

    return returnValue;
}

int main()
{
    DigitalOut led1(LED1); // pulse led
    DigitalOut led2(LED2); // initial reset error led (lights up if error occurs)
    DigitalOut led3(LED3); // initial set error led (lights up if error occurs)

    DigitalOut g1(G1);
    DigitalOut g2(G2);

    DigitalIn k0(K0);

    led2 = reset(g1, g2, k0);
    led3 = set(g1, g2, k0, 9);

    ThisThread::sleep_for(500ms);

    read(g1, g2, k0);

    while (true) {
        led1 = !led1;
 
        for (uint32_t i = 0; i < 10; i++)
        {
            set(g1, g2, k0, i);

            ThisThread::sleep_for(PULSE_RATE);
        }
    }
}
