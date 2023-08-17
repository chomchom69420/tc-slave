#include "timer.h"
#include <Wire.h>
#include <unordered_map>

#define D0_ADDR_SUFFIX 0b000
#define D1_ADDR_SUFFIX 0b001
#define D2_ADDR_SUFFIX 0b010
#define IODIR 0x00
#define GPIO 0x09

uint8_t d0_addr, d1_addr, d2_addr;
uint8_t error;

// This map is for common anode configuration with g as MSB and a as LSB
std::unordered_map<int, uint8_t> digit_map = {
    {0, 0x3F},
    {1, 0x06},
    {2, 0x5B},
    {3, 0x4F},
    {4, 0x66},
    {5, 0x6D},
    {6, 0x7D},
    {7, 0x07},
    {8, 0x7F},
    {9, 0x4F}
};

void timer_hardware_init()
{
    // Init addresses
    d0_addr = (0b0100 << 4) | (D0_ADDR_SUFFIX << 1) | 0b0;
    d1_addr = (0b0100 << 4) | (D1_ADDR_SUFFIX << 1) | 0b0;
    d2_addr = (0b0100 << 4) | (D2_ADDR_SUFFIX << 1) | 0b0;

    // Initialize I2C
    Wire.begin();

    // Set all IO expanders to output mode
    Wire.beginTransmission(d0_addr);
    Wire.write(IODIR);
    Wire.write(0x00); // setting all pins to ouput
    Wire.endTransmission();
    Wire.beginTransmission(d0_addr);
    Wire.write(GPIO);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(d1_addr);
    Wire.write(IODIR);
    Wire.write(0x00); // setting all pins to ouput
    Wire.endTransmission();
    Wire.beginTransmission(d1_addr);
    Wire.write(GPIO);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(d2_addr);
    Wire.write(IODIR);
    Wire.write(0x00); // setting all pins to ouput
    Wire.endTransmission();
    Wire.beginTransmission(d2_addr);
    Wire.write(GPIO);
    Wire.write(0x00);
    Wire.endTransmission();
}

void timer_update()
{
    int time = signals_get_secondary_time_remaining();
    Wire.beginTransmission(d0_addr);
    Wire.write(GPIO);
    Wire.write(digit_map[time % 10]);
    Wire.endTransmission();

    Wire.beginTransmission(d1_addr);
    Wire.write(GPIO);
    Wire.write(digit_map[(time / 10) % 10]);
    Wire.endTransmission();

    Wire.beginTransmission(d2_addr);
    Wire.write(GPIO);
    Wire.write(digit_map[(time / 100) % 10]);
    Wire.endTransmission();
}