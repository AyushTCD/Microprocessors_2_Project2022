#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "ws2812.pio.h"
#include <string.h>

char input[20];
int ind = 0;
int lives = 3;

typedef struct mos {   
    char let;       // the letter
    char *cod;      //value of that letter in morse code
}mos;

mos letter[36];    // Struct array for putting the data of alphabets with their respective morse code

void mos_init() {
      letter[0].let='A';
      letter[1].let='B';
      letter[2].let='C';
      letter[3].let='D';
      letter[4].let='E';
      letter[5].let='F';
      letter[6].let='G';
      letter[7].let='H';
      letter[8].let='I';
      letter[9].let='J';
      letter[10].let='K';
      letter[11].let='L';
      letter[12].let='M';
      letter[13].let='N';
      letter[14].let='O';
      letter[15].let='P';
      letter[16].let='Q';
      letter[17].let='R';
      letter[18].let='S';
      letter[19].let='T';
      letter[20].let='U';
      letter[21].let='V';
      letter[22].let='W';
      letter[23].let='X';
      letter[24].let='Y';
      letter[25].let='Z';
      letter[26].let='0';
      letter[27].let='1';
      letter[28].let='2';
      letter[29].let='3';
      letter[30].let='4';
      letter[31].let='5';
      letter[32].let='6';
      letter[33].let='7';
      letter[34].let='8';
      letter[35].let='9';
 
      letter[0].cod=".-";
      letter[1].cod="-...";
      letter[2].cod="-.-.";
      letter[3].cod="-..";
      letter[4].cod=".";
      letter[5].cod="..-.";
      letter[6].cod="--.";
      letter[7].cod="....";
      letter[8].cod="..";
      letter[9].cod=".---";
      letter[10].cod="-.-";
      letter[11].cod=".-..";
      letter[12].cod="--";
      letter[13].cod="-.";
      letter[14].cod="---";
      letter[15].cod=".--.";
      letter[16].cod="--.-";
      letter[17].cod=".-.";
      letter[18].cod="...";
      letter[19].cod="-";
      letter[20].cod="..-";
      letter[21].cod="...-";
      letter[22].cod=".--";
      letter[23].cod="-..-";
      letter[24].cod="-.--";
      letter[25].cod="--..";
      letter[26].cod="-----";
      letter[27].cod=".----";
      letter[28].cod="..---";
      letter[29].cod="...--";
      letter[30].cod="....-";
      letter[31].cod=".....";
      letter[32].cod="-....";
      letter[33].cod="--...";
      letter[34].cod="---..";
      letter[35].cod="----.";
}

// Used to generate a random word in level 3 and 4
char *random_word[10] = {   "RUN",
                            "BAR",
                            "SUN",
                            "BOY",
                            "SKY",
                            "AIR",
                            "NO",
                            "TEAL",
                            "SLY",
                            "GUN"
};

// corresponds to the random_word array
char *random_word_morse[10] = {     ".-. ..- -.",    // "RUN"
                                    "-... .- .-.",   // "BAR"
                                    "... ..- -.",    // "SUN"
                                    "-... --- -.--", // "BOY"
                                    "... -.- -.--",  // "SKY"
                                    ".- .. .-.",     // "AIR"
                                    "-. ---",        // "NO"
                                    "- . .- .-..",   // "TEAL"
                                    "... .-.. -.--", // "SLY"
                                    "--. ..- -."     // "GUN"
};


// Resets the watchdog timer
void asm_watchdog() { 
    // watchdog_enable(9000, 1);
 
    // for (uint i = 0; i < 5; i++) {
        watchdog_update();
    // }
}

// Decreases lives by 1. Returns 0 if all lives are gone.
int decrease_life() {  
    lives--;
    if (lives == 0) return 0;
    else return 1;
}

// Increase lives by 1
void increase_life() {
    if (lives < 3) {
        lives++;
    }
}

// Wrapper to allow the assembly code to call the gpio_init() SDK function.
void asm_gpio_init(int pin) {
    gpio_init(pin);
}

// Wrapper to allow the assembly code to call the gpio_set_dir() SDK function.
void asm_gpio_set_dir(int pin, int dir) {
    gpio_set_dir(pin, dir);
}


// Wrapper to allow the assembly code to call the gpio_get() SDK function.
int asm_gpio_get(int pin) {
    return gpio_get(pin);
}


// Wrapper to allow the assembly code to call the gpio_put() SDK function.
void asm_gpio_put(int pin, int value) {
    gpio_put(pin, value);
}

// Adds dot/dash/space to morse code string
void asm_add_input(int sample) {
    if (ind < 20){
        if (sample == 0)
            input[ind] = '.';
        else if (sample == 1)
            input[ind] = '-';
        else if (sample == 2)
            input[ind] = ' ';
        else if (sample == 3)
            input[ind] = '\0';
        ind++;
    }
}

static inline void life_to_color(){  //code for updating the led
    
    if(lives==3){
        light_green();
    }
    else if(lives==2){
        light_yellow();
    }
    else if(lives==1){
        light_orange();
    }
    else if(lives==0){
        light_red();
    }
}


void clear_input() {
    ind = 0;
    // memset(input, 0, 20*sizeof(input[0]));
}

// Must declare the main assembly entry point before use.
void main_asm();

// Level 1 (Match Characters with right sequence given - with hints). Returns 1 if completed, 0 if failed.
int level_1 () {
    lives = 3;
    clear_input();
    int num, win_streak = 0;
    printf("Level 1\n");
    
    // Players must succesfully complete 5 games without dying three time
    while (1) {
        // Output a random character and its morse code for the player to enter
        num = rand() % 36;
        printf("Input the following character in morse code: %c (%s)\n", letter[num].let, letter[num].cod);

        // Get the input assembly
        main_asm();

        // Answer is correct -> increase win streak
        if ( strcmp(input, letter[num].cod) == 0 ) {
            printf("Answer is correct\n");
            win_streak++;
        }
        // Answer is incorrect -> deduct a life and reset win streak
        else {    
            printf("Answer is incorrect\n");
            win_streak = 0;
            if (!decrease_life()) {
                printf("Game Over\n");
                return 0;
            }
            life_to_color();   //to glow up the led
        }
        // Complete 5 games in a row to complete level 1
        if (win_streak == 5) {
            printf("Level 1 Complete\n\n");
            return 1;
        }
        clear_input();
    }    
    return 1;
}


// Level 2 (Match Characters with right sequence given - no hints). Returns 1 if completed, 0 if failed.
int level_2 () {
    lives = 3;
    clear_input();
    int num, win_streak = 0;
    printf("Level 2\n");

    // Players must succesfully complete 5 games without dying three time
    while (1) {
        // Output a random character and its morse code for the player to enter
        num = rand() % 36;
        printf("Input the following character in morse code: %c\n", letter[num].let);

        // Get the input assembly
        main_asm();

        // Answer is correct -> increase win streak
        if ( strcmp(input, letter[num].cod) == 0 ) {
            printf("Answer is correct\n");
            win_streak++;
        }
        // Answer is incorrect -> deduct a life and reset win streak
        else {    
            printf("Answer is incorrect\n");
            win_streak = 0;
            if (!decrease_life()) {
                printf("Game Over\n");
                return 0;
            }
              life_to_color();   //to glow up the led
        }
        // Complete 5 games in a row to complete level 2
        if (win_streak == 5) {
            printf("Level 2 Complete\n\n");
            return 1;
        }
        clear_input();
    }    
    return 1;
}

// Level 3 (Match Characters with right sequence given - with hints). Returns 1 if completed, 0 if failed.
int level_3 () {
    lives = 3;
    clear_input();
    int num, win_streak = 0;
    printf("Level 3\n");

    // Players must succesfully complete 5 games without dying three time
    while (1) {
        // Output a random word and its morse code for the player to enter
        num = rand() % 10;
        printf("Input the following word in morse code: %s (%s)\n", random_word[num], random_word_morse[num]);

        // Get the input assembly
        main_asm();

        // Answer is correct -> increase win streak
        if ( strcmp(input, random_word_morse[num]) == 0 ) {
            printf("Answer is correct\n");
            win_streak++;
        }
        // Answer is incorrect -> deduct a life and reset win streak
        else {    
            printf("Answer is incorrect\n");
            win_streak = 0;
            if (!decrease_life()) {
                printf("Game Over\n");
                return 0;
            }
              life_to_color(); //to glow up the led
        }
        // Complete 5 games in a row to complete level 3
        if (win_streak == 5) {
            printf("Level 3 Complete\n\n");
            return 1;
        }
        clear_input();
    }    
    return 1;
}

// Level 4 (Match a word with right sequence given - no hints). Returns 1 if completed, 0 if failed.
int level_4 () {
    lives = 3;
    clear_input();
    int num, win_streak = 0;
    printf("Level 4\n");

    // Players must succesfully complete 5 games without dying three time
    while (1) {
        // Output a random word and its morse code for the player to enter
        num = rand() % 10;
        printf("Input the following word in morse code: %s\n", random_word[num]);

        // Get the input assembly
        main_asm();

        // Answer is correct -> increase win streak
        if ( strcmp(input, random_word_morse[num]) == 0 ) {
            printf("Answer is correct\n");
            win_streak++;
        }
        // Answer is incorrect -> deduct a life and reset win streak
        else {    
            printf("Answer is incorrect\n");
            win_streak = 0;
            if (!decrease_life()) {
                printf("Game Over\n");
                return 0;
            }
              life_to_color();  //to glow up the led
        }
        // Complete 5 games in a row to complete level 4
        if (win_streak == 5) {
            printf("Level 4 Complete\n\n");
            return 1;
        }
        clear_input();
    }    
    return 1;
}

//Printing out the welcome message
void welcome(){     
    printf("-------------------------------------------------\n");
    printf("--                MORSE CODE GAME              --\n");
    printf("--       USE GP-21 T0 INPUT THE SEQUENCE       --\n");
    printf("--                 Level-1:'.'                 --\n");
    printf("--                 Level-2:'..'                --\n");
    printf("--                 Level-3:'...'               --\n");
    printf("--                 Level-4:'....'              --\n");
    printf("-------------------------------------------------\n");
}


int main() {
    srand(time(NULL)); // Initialise for rand()
     
    stdio_init_all(); // Initialise all basic IO

    mos_init(); // Initialise global mos array letters 

    int result;
    while (1) {
        // Choose a level
        clear_input();
        lives = 3;
        welcome();
        main_asm();

        // Level 1
        if ( strcmp(input, ".") == 0 ) {
            result = level_1();
            if (result) {
                // Progress to level 2
                clear_input();
                char sel_level_2[] = "..";
                strcpy(input, sel_level_2);
            }
        }

        // Level 2
        if ( strcmp(input, "..") == 0 ) {
            result = level_2();
            if (result) {
                // Progress to level 3
                clear_input();
                char sel_level_3[] = "...";
                strcpy(input, sel_level_3);
            }
        }

        // Level 3
        if ( strcmp(input, "...") == 0 ) {
            result = level_3();
            if (result) {
                // Progress to level 4
                clear_input();
                char sel_level_4[] = "....";
                strcpy(input, sel_level_4);
            }
        }

        // Level 4
        if ( strcmp(input, "....") == 0 ) {
            result = level_4();
            if (result) {
                // Game completed
                clear_input();
                printf("You Won!\n\n");
            }
        }
    }
    return 0;    
}
