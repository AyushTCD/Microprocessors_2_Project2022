.syntax unified                 @ Specify unified assembly syntax
.cpu    cortex-m0plus           @ Specify CPU type is Cortex M0+
.thumb                          @ Specify thumb assembly for RP2040
.global main_asm                @ Provide program starting address to the linker
.align 4                        @ Specify code alignment

.equ    SLEEP_TIME,     200     @ Specify the amount of ms that we want to sleep for in the loop
.equ    LED_GPIO_PIN,   25      @ Specifiy the physical GPIO pin that the LED is connected to
.equ    LED_GPIO_OUT,   1       @ Specify the direction that we want to set the GPIO pin to
.equ    LED_VALUE_ON,   1       @ Specify the value that turns the LED "on"
.equ    LED_VALUE_OFF,  0       @ Specify the value that turns the LED "off"
.equ    GP21, 21                @ Specifiy the physical GPIO pin that the pushbutton is connected to

.equ    GP20,           20      @ Specifiy the physical GPIO pin that the pushbutton is connected to
.equ    GP22,           22      @ Specifiy the physical GPIO pin that the pushbutton is connected to
.equ    DOT_TIME,       1000    @ LED flashes for 1 second for dot
.equ    DASH_TIME,      2000    @ LED flashes for 3 second for dash
.equ    DOT,            0       @ Specify 0 to signify a dot '.'
.equ    DASH,           1       @ Specify 1 to signify a dash '_'
.equ    SPACE,          2       @ Specify 2 to signify a space ' '
.equ    NULL,           3       @ Specify 3 to signify a space '\0'

.equ    GPIO_IN, 0              @ Specify the direction that we want to set the GPIO pin to
.equ    PUSHBUTTON_ON, 0        @ Specifiy the button has been pressed 

@ Entry point to the ASM portion of the program
main_asm:
    push    {lr}

    @ init LED
    movs    r0, #LED_GPIO_PIN           @ This value is the GPIO LED pin on the PI PICO board
    bl      asm_gpio_init               @ Call the subroutine to initialise the GPIO pin specified by r0
    movs    r0, #LED_GPIO_PIN           @ This value is the GPIO LED pin on the PI PICO board
    movs    r1, #LED_GPIO_OUT           @ We want this GPIO pin to be setup as an output pin
    bl      asm_gpio_set_dir            @ Call the subroutine to set the GPIO pin specified by r0 to state specified by r1

    @ button for dots and dashes
    movs    r0, #GP21                   @ This value is the GPIO pin 21 on the PI PICO board
    bl      asm_gpio_init               @ Call the subroutine to initialise the GPIO pin specified by r0
    movs    r0, #GP21                   @ This value is the GPIO pin 21 on the PI PICO board
    movs    r1, #GPIO_IN                @ We want this GPIO pin to be setup as an input pin
    bl      asm_gpio_set_dir            @ Call the subroutine to set the GPIO pin specified by r0 to state specified by r1

    @ button for space
    movs    r0, #GP20                   @ This value is the GPIO pin 21 on the PI PICO board
    bl      asm_gpio_init               @ Call the subroutine to initialise the GPIO pin specified by r0
    movs    r0, #GP20                   @ This value is the GPIO pin 21 on the PI PICO board
    movs    r1, #GPIO_IN                @ We want this GPIO pin to be setup as an input pin
    bl      asm_gpio_set_dir            @ Call the subroutine to set the GPIO pin specified by r0 to state specified by r1


    @ button for exiting
    movs    r0, #GP22                   @ This value is the GPIO pin 21 on the PI PICO board
    bl      asm_gpio_init               @ Call the subroutine to initialise the GPIO pin specified by r0
    movs    r0, #GP22                   @ This value is the GPIO pin 21 on the PI PICO board
    movs    r1, #GPIO_IN                @ We want this GPIO pin to be setup as an input pin
    bl      asm_gpio_set_dir            @ Call the subroutine to set the GPIO pin specified by r0 to state specified by r1

loop:

    ldr     r0, =SLEEP_TIME             @ Set the value of SLEEP_TIME we want to wait for
    bl      sleep_ms                    @ Sleep until SLEEP_TIME has elapsed then poll the status of the pushbutton
    
    movs    r0, #GP21                   @ Set the GPIO pin 21 to r0 for use by asm_gpio_get
    bl      asm_gpio_get                @ Get the current value of the GPIO pin 21 (returns to r0)
    cmp     r0, #PUSHBUTTON_ON          @ The LED will only toggle its state after the button has been pressed
    beq     sub_toggle_dot             @ If button is pressed: Call the subroutine to toggle the current LED GPIO pin value

    movs    r0, #GP20                   @ Set the GPIO pin 21 to r0 for use by asm_gpio_get
    bl      asm_gpio_get                @ Get the current value of the GPIO pin 21 (returns to r0)
    cmp     r0, #PUSHBUTTON_ON          @ The LED will only toggle its state after the button has been pressed
    beq     sub_toggle_space             @ If button is pressed: Call the subroutine to toggle the current LED GPIO pin value

    movs    r0, #GP22                   @ Set the GPIO pin 21 to r0 for use by asm_gpio_get
    bl      asm_gpio_get                @ Get the current value of the GPIO pin 21 (returns to r0)
    cmp     r0, #PUSHBUTTON_ON          @ The LED will only toggle its state after the button has been pressed
    @beq     sub_toggle_space            @ If button is pressed: Call the subroutine to toggle the current LED GPIO pin value
    beq     exit_loop

    b       loop                        @ Repeat the loop


exit_loop:
    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_get
    movs    r1, #LED_VALUE_ON           @ The LED is currently "off" so we want to turn it "on"
    bl      asm_gpio_put 

    movs    r0, #NULL                   @ move a '\0' to r0
    bl      asm_add_input               @ Return what is stored in r0 to the morse string in C file
   
    bl      asm_watchdog
    pop     {pc}

@ Subroutine to toggle the LED GPIO pin value
sub_toggle:
    push    {lr}                        @ Store the link register to the stack as we will call nested subroutines
    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_get
    bl      asm_gpio_get                @ Get current the value of the LED GPIO pin (returns to r0)
    cmp     r0, #LED_VALUE_OFF          @ Check if the LED GPIO pin value is "off"
    beq     led_set_on                  @ If it is "off" then then jump code to to turn it on
led_set_off:    
    movs    r1, #LED_VALUE_OFF          @ The LED is currently "on" so we want to turn it "off"
    b       led_set_state               @ Jump to portion of code where we set the state of the LED
led_set_on:
    movs    r1, #LED_VALUE_ON           @ The LED is currently "off" so we want to turn it "on"
led_set_state:
    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_put
    bl      asm_gpio_put                @ Update the the value of the LED GPIO pin (based on value in r1)
    pop     {pc}                        @ Pop the link register from the stack to the program counter

@ Subroutine to toggle the LED GPIO pin value
sub_toggle_dot:
    push    {lr}                        @ Store the link register to the stack as we will call nested subroutines
    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_get
    movs    r1, #LED_VALUE_ON           @ The LED is currently "off" so we want to turn it "on"
    bl      asm_gpio_put                @ Update the the value of the LED GPIO pin (based on value in r1)
    ldr     r0, =DOT_TIME               @ Set the value of DOT_TIME we want to wait for
    bl      sleep_ms                    @ Sleep until DOT_TIME has elapsed then poll the status of the pushbutton
    
    @ Toggle to dot if the button is pressed longer than DOT_TIME seconds
    movs    r0, #GP21                   @ Set the GPIO pin 21 to r0 for use by asm_gpio_get
    bl      asm_gpio_get                @ Get the current value of the GPIO pin 21 (returns to r0)
    cmp     r0, #PUSHBUTTON_ON          @ The LED will only toggle its state after the button has been pressed
    beq     sub_toggle_dash             @ If button is pressed: Call the subroutine to toggle the current LED GPIO pin value

    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_get
    movs    r1, #LED_VALUE_OFF          @ The LED is currently "on" so we want to turn it "off"
    bl      asm_gpio_put                @ Update the the value of the LED GPIO pin (based on value in r1)
    
    movs    r0, #DOT                    @ move a dot to r0
    bl      asm_add_input               @ Return what is stored in r0 to the morse string in C file
    
    b       loop                        @ Jump to portion of code where we wait for input
   
    pop     {pc}                        @ Pop the link register from the stack to the program counter


@ Subroutine to toggle the LED GPIO pin value
sub_toggle_dash:
    push    {lr}                        @ Store the link register to the stack as we will call nested subroutines
    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_get
    movs    r1, #LED_VALUE_ON           @ The LED is currently "off" so we want to turn it "on"
    bl      asm_gpio_put                @ Update the the value of the LED GPIO pin (based on value in r1)
    ldr     r0, =DASH_TIME               @ Set the value of DASH_TIME we want to wait for
    bl      sleep_ms                    @ Sleep until DASH_TIME has elapsed then poll the status of the pushbutton
    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_get
    movs    r1, #LED_VALUE_OFF          @ The LED is currently "on" so we want to turn it "off"
    bl      asm_gpio_put                @ Update the the value of the LED GPIO pin (based on value in r1)
    
    movs    r0, #DASH                   @ move a dash to r0
    bl      asm_add_input               @ Return what is stored in r0 to the morse string in C file

    b       loop                        @ Jump to portion of code where we wait for input

    pop     {pc}                        @ Pop the link register from the stack to the program counter


@ Subroutine to toggle the LED GPIO pin value
sub_toggle_space:
    push    {lr}                        @ Store the link register to the stack as we will call nested subroutines
    movs    r0, #LED_GPIO_PIN           @ Set the LED GPIO pin number to r0 for use by asm_gpio_get
    movs    r1, #LED_VALUE_OFF           @ The LED is currently "off" so we want to turn it "on"
    bl      asm_gpio_put                @ Update the the value of the LED GPIO pin (based on value in r1)
    ldr     r0, =DOT_TIME               @ Set the value of DOT_TIME we want to wait for
    bl      sleep_ms                    @ Sleep until DOT_TIME has elapsed then poll the status of the pushbutton
    
    movs    r0, #SPACE                  @ move a space to r0
    bl      asm_add_input               @ Return what is stored in r0 to the morse string in C file

    b       loop                        @ Jump to portion of code where we wait for input

    pop     {pc}                        @ Pop the link register from the stack to the program counter


@ Set data alignment
.data
    .align 4
