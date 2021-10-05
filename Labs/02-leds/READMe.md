# Lab 2: Kryštof Buroň

Link to my `Digital-electronics-2` GitHub repository:

   [https://github.com/christ-0ff](https://github.com/christ-0ff)


### Active-low and active-high LEDs

1. Complete tables according to the AVR manual.

| **DDRB** | **Description** |
| :-: | :-- |
| 0 | Input pin |
| 1 | Output pin |

| **PORTB** | **Description** |
| :-: | :-- |
| 0 | Output low value |
| 1 | Output high value |

| **DDRB** | **PORTB** | **Direction** | **Internal pull-up resistor** | **Description** |
| :-: | :-: | :-: | :-: | :-- |
| 0 | 0 | input | no | Tri-state, high-impedance |
| 0 | 1 | input | yes | Pxn will source current if ext. pulled low. |
| 1 | 0 | output | no | Output Low (Sink) |
| 1 | 1 | output | no | Output High (Source) |

2. Part of the C code listing with syntax highlighting, which blinks alternately with a pair of LEDs; let one LED is connected to port B and the other to port C:

```c
/* Defines -----------------------------------------------------------*/
#define LED_GREEN   PB5     // AVR pin where green LED is connected
#define LED_RED     PC5        // AVR pin where red LED is connected   
#define BLINK_DELAY 500

int main(void)
{
    // Green LED at port B
    // Set pin as output in Data Direction Register...
    DDRB = DDRB | (1<<LED_GREEN);
    // ...and turn LED off in Data Register
    PORTB = PORTB & ~(1<<LED_GREEN);

    // Configure the second LED at port C
    DDRC = DDRC | (1<<LED_RED);
    // ...and turn LED on in Data Register
    PORTC = PORTC | (1<<LED_RED);
     
    // Infinite loop
    while (1)
    {
        // Pause several milliseconds
         _delay_ms(BLINK_DELAY);
         PORTB = PORTB ^ (1<<LED_GREEN);
         PORTC = PORTC ^ (1<<LED_RED);
         
         _delay_ms(BLINK_DELAY);
         PORTC = PORTC ^ (1<<LED_RED);
         PORTB = PORTB ^ (1<<LED_GREEN);
         
    }

    // Will never reach this
    return 0;
}
```


### Push button

1. Part of the C code listing with syntax highlighting, which toggles LEDs only if push button is pressed. Otherwise, the value of the LEDs does not change. Let the push button is connected to port D:

```c
/* Defines -----------------------------------------------------------*/
#define LED_GREEN   PB5     // AVR pin where green LED is connected
#define LED_RED     PC5
#define BUTTON      PD0     // AVR pin where green LED is connected
#define BLINK_DELAY 500

int main(void)
{
    // Green LED at port B
    // Set pin as output in Data Direction Register...
    DDRB = DDRB | (1<<LED_GREEN);
    // ...and turn LED off in Data Register
    PORTB = PORTB & ~(1<<LED_GREEN);

    // Configure the second LED at port C
    DDRC = DDRC | (1<<LED_RED);    
    PORTC = PORTC | (1<<LED_RED);
    
    // Configure Push button at port D and enable internal pull-up resistor
    DDRD = DDRD & ~(1<<BUTTON);
    PORTD = PORTD | (1<<BUTTON);
    
    // Infinite loop
    while (1)
    {
        // Pause several milliseconds

        if(bit_is_clear(PIND, BUTTON))
        {
                    _delay_ms(BLINK_DELAY);
                    PORTB = PORTB ^ (1<<LED_GREEN);
                    PORTC = PORTC ^ (1<<LED_RED);
                    
                    _delay_ms(BLINK_DELAY);
                    PORTC = PORTC ^ (1<<LED_RED);
                    PORTB = PORTB ^ (1<<LED_GREEN);
                    loop_until_bit_is_clear(PIND, BUTTON);
                    
        }
    }

    // Will never reach this
    return 0;
}
```


### Knight Rider

1. Scheme of Knight Rider application, i.e. connection of AVR device, five LEDs, resistors, one push button, and supply voltage. The image can be drawn on a computer or by hand. Always name all components and their values!

   ![KNIGHTRIDER](images/KNIGHTRIDER.png)
