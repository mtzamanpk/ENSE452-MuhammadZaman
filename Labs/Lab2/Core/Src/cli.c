/**
* FILENAME: cli.c
*
* DESCRIPTION:
*
* In this file, you will find the functions required to make lab 2 functional,
* usart_send_string, is a helper function i made to send the strings/messages via USART
* cli_process_comand, is a function made to process commands that are required of the lab
* cli_prompt, is just a small ui tweak that was made to give the user a better viewing experience
* cli_welcome, is the welcome message to tell the user information needed when starting the program
* cli_run, is the main logic for running the program in cli, where it takes all
* the previous functions and uses them as needed
*
* AUTHOR: Muhammad Zaman 200449177
*/


#include "usart.h"
#include <string.h>

char input_buffer[100];
int buffer_index = 0;

// Helper function to send strings via UART
void uart_send_string(const char *str) {
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

// Process command function
void cli_process_command(const char *cmd) {
    uart_send_string("\r\n");  // Start new line before processing command output

    if (strcmp(cmd, "LED ON") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        uart_send_string("LED turned ON\r\n");
    } else if (strcmp(cmd, "LED OFF") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        uart_send_string("LED turned OFF\r\n");
    } else if (strcmp(cmd, "STATUS") == 0) {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5)) {
            uart_send_string("LED is ON\r\n");
        } else {
            uart_send_string("LED is OFF\r\n");
        }
    } else if (strcmp(cmd, "HELP") == 0) {
        uart_send_string("Available commands:\r\n");
        uart_send_string("LED ON - Turns the LED on\r\n");
        uart_send_string("LED OFF - Turns the LED off\r\n");
        uart_send_string("LED STATUS - Gets the current LED state\r\n");
        uart_send_string("HELP - Displays this help message\r\n");
    } else {
        uart_send_string("Unknown command: ");
        uart_send_string(cmd);
        uart_send_string("\r\n");
    }
}

// Function to show the prompt
void cli_prompt(void) {
    uart_send_string("> ");
}

// Function to display welcome message
void cli_welcome(void) {
    uart_send_string("Welcome to the Lab 2 CLI!\r\n");
    uart_send_string("Type 'HELP' for a list of commands.\r\n\r\n");
    cli_prompt();  // Show initial prompt after the welcome message
}

// Main CLI function
void cli_run(void) {

	char received;
    HAL_UART_Receive(&huart2, (uint8_t*)&received, 1, HAL_MAX_DELAY);

    // Check for new line (command end)
    if (received == '\r' || received == '\n') {
        input_buffer[buffer_index] = '\0';  // Null-terminate the string
        cli_process_command(input_buffer);  // Process the command
        buffer_index = 0;                   // Reset buffer
        cli_prompt();                       // Show prompt again
    } else if (received == '\b') {  // Backspace handling
        if (buffer_index > 0) {
            buffer_index--;
            uart_send_string("\b \b");  // Erase the character from the terminal
        }
    } else {
        input_buffer[buffer_index++] = received;  // Add to buffer
        HAL_UART_Transmit(&huart2, (uint8_t*)&received, 1, HAL_MAX_DELAY);  // Echo character

        // Prevent buffer overflow
        if (buffer_index >= sizeof(input_buffer)) {
            buffer_index = 0;
            uart_send_string("\r\nError: Input buffer overflow.\r\n");
            cli_prompt();
        }
    }
}
