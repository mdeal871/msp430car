//******************************************************************************
//
//  Description: This file contains the Function prototypes
//
//  Jim Carlson / Mason Deal
//  Mar 2026
//  Built with Code Composer Studio version 12.8.1.00005
//******************************************************************************

// Functions

// Main
void main(void);

// State Machines
void Carlson_StateMachine(void);
void Run_Project6(void);
void Line_Follow(void);

// Wheels
void Wheels_Off(void);
void Forward_On(void);
void Forward_Off(void);
void R_Forward(void);
void L_Forward(void);
void R_Off(void);
void L_Off(void);
void Reverse_On(void);
void Reverse_Off(void);
void R_Reverse(void);
void L_Reverse(void);

// Project 4 function prototypes
void Wheels_Off(void);
void Run_CIRCLE(void);
void Run_FIGUREEIGHT(void);
void Run_TRIANGLE(void);


// Initialization
void Init_Conditions(void);
void Init_Timers(void);
void Init_Switch_Interrupts(void);

// Interrupts
void enable_interrupts(void);
__interrupt void Timer0_B0_ISR(void);
__interrupt void switch_interrupt(void);

// Analog to Digital Converter
void Init_ADC(void);
void ADC_Process(void);
unsigned int Read_Thumbwheel(void);

// Clocks
void Init_Clocks(void);
void Set_SMCLK_16MHz(void);
void Set_SMCLK_8MHz(void);

// LED Configurations
void Init_LEDs(void);
void IR_LED_control(char selection);
void Backlite_control(char selection);

  // LCD
void Display_Process(void);
void Display_Update(char p_L1,char p_L2,char p_L3,char p_L4);
void enable_display_update(void);
void update_string(char *string_data, int string);
void Init_LCD(void);
void lcd_clear(void);
void lcd_putc(char c);
void lcd_puts(char *s);

void lcd_power_on(void);
void lcd_write_line1(void);
void lcd_write_line2(void);
//void lcd_draw_time_page(void);
//void lcd_power_off(void);
void lcd_enter_sleep(void);
void lcd_exit_sleep(void);
//void lcd_write(unsigned char c);
//void out_lcd(unsigned char c);

void Backlite_On(void);
void Backlite_Off(void);
void Backlite_Flash(void);

void Write_LCD_Ins(char instruction);
void Write_LCD_Data(char data);
void ClrDisplay(void);
void ClrDisplay_Buffer_0(void);
void ClrDisplay_Buffer_1(void);
void ClrDisplay_Buffer_2(void);
void ClrDisplay_Buffer_3(void);

void SetPostion(char pos);
void DisplayOnOff(char data);
void lcd_BIG_mid(void);
void lcd_BIG_bot(void);
void lcd_120(void);

void lcd_4line(void);
void lcd_out(char *s, char line, char position);
void lcd_rotate(char view);

//void lcd_write(char data, char command);
void lcd_write(unsigned char c);
void lcd_write_line1(void);
void lcd_write_line2(void);
void lcd_write_line3(void);

void lcd_command( char data);
void LCD_test(void);
void LCD_iot_meassage_print(int nema_index);

// Menu System
void Menu_Process(void);
void Menu_Main(void);
void Menu_Resistors(void);
void Menu_Shapes(void);
void Menu_IOT(void);
void Menu_ADC_Monitor(void);
void Menu_Serial_Monitor(void);
void Menu_Song(void);

// Ports
void Init_Ports(void);
void Init_Port1(void);
void Init_Port2(void);
//void Init_Port3(char smclk);
void Init_Port3(void);
void Init_Port4(void);
void Init_Port5(void);
void Init_Port6(void);

// SPI
void Init_SPI_B1(void);
void SPI_B1_write(char byte);
void spi_rs_data(void);
void spi_rs_command(void);
void spi_LCD_idle(void);
void spi_LCD_active(void);
void SPI_test(void);
void WriteIns(char instruction);
void WriteData(char data);

// Switches
void Init_Switches(void);
void switch_control(void);
void enable_switch_SW1(void);
void enable_switch_SW2(void);
void disable_switch_SW1(void);
void disable_switch_SW2(void);
void Switches_Process(void);
void Init_Switch(void);
void Switch_Process(void);
void Switch1_Process(void);
void Switch2_Process(void);
void menu_act(void);
void menu_select(void);

// Timers
void Init_Timers(void);
void Init_Timer_B0(void);
void Init_Timer_B1(void);
void Init_Timer_B2(void);
void Init_Timer_B3(void);

void usleep(unsigned int usec);
void usleep10(unsigned int usec);
void five_msec_sleep(unsigned int msec);
void measure_delay(void);
void out_control_words(void);

// Serial Communication
void Serial_Process(void);
void Serial_Transmit_Message(void);
void Serial_Toggle_BaudRate(void);
void Serial_Process_UCA1_RX(void);
void Serial_Process_UCA0_RX(void);

// IOT Wheels State Machine
void Run_IOT_Wheels(void);
void Reset_IOT_Wheels(void);

// IOT AT Command Helpers
void Send_IOT_Command(char *command);
void IOT_Init_Sequence(void);
void IOT_Connect_WiFi(char *ssid, char *password);
void IOT_Start_Server(unsigned int port);
void IOT_Parse_Response(void);
void IOT_Command_Dispatch(char *cmd);

// IOT Network Management
void IOT_Ping(void);                    // Send ping to www.google.com
void IOT_Check_Reconnect(void);         // Auto-reconnect if WiFi dropped
void IOT_Emergency_Stop(void);          // Immediately halt all motion

// Demo Day Menu (single menu, phase-driven internally)
void Menu_Demo(void);                  // Calibration -> WiFi course -> Black line
void Demo_Reset_Phase(void);           // Reset internal phase to 0 (calibration)
void Demo_Update_Seconds(void);        // Update demo_seconds from iot_elapsed
void Demo_Set_Arrived(unsigned char pad_num);  // Set "Arrived 0X" display
void Demo_Clear_Arrived(void);         // Clear arrived line when off pad
void Demo_Set_Last_Cmd(char dir, unsigned int ms); // Update last cmd display

// Black line state machine entry / exit
void Run_Blackline(void);              // Main black line state machine (intercept + follow)
void Blackline_Exit_Command(void);     // Jump to exit spin state (TA command)
void Blackline_Emergency_Exit(void);   // Immediately stop blackline SM + motors

// Drive-up positioning state machine (^BR / ^BL commands)
void Run_Driveup(void);                // Pad 8 -> board alignment state machine
void Reset_Driveup(void);              // Stop driveup SM and clear state
