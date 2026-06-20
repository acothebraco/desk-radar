#pragma once
// Portable LVGL boot/hello screen — built with pure LVGL calls so it runs both on
// the device (display.cpp) and in the native SDL simulator (sim_main.cpp).
// No Arduino / Arduino_GFX / ESP dependencies here.

void ui_boot_create(void);   // builds the M0 boot screen on the active screen
