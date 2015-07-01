/* 
 *   Copyright (C) 2015 Quentin Gibeaux
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 *   emiT esreveR watchface
 */

#include <pebble.h>

#include "effect_layer.h"
#include "effects.h"

#define COLOR_H  GColorFromRGBA(255,20,0,255)
#define INITIAL_COLOR GColorBlack

/* --------------------------- Function signatures ---------------------------*/
/* --------------- Global variables ------------------------------------------*/
static Window *g_main_window;
static TextLayer * g_time_layer;
static EffectLayer* effect_layer;

static GFont s_custom_font;

/** ----------------------------------------------------------------------------
 * Update all fields related to time
 */
static void update_time() {
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    static char time[] = "00:00:00";
    static char colon[] = ":";
    static int i_hour = 0;
    static int i_minute = 0;
    static int i_second = 0;
    static int sec_now = 0;
    static int sec_end = 0;
    static int diff = 0;
    static int hour_max;

    // Write the current hours and minutes into the buffer
    if(clock_is_24h_style() == true) {
        // Use 24 hour format
	hour_max = 24;
    } else {
        // Use 12 hour format
	hour_max = 12;
    }

    sec_now = 0;
    sec_now += tick_time->tm_hour*60*60;
    sec_now += tick_time->tm_min*60;
    sec_now += tick_time->tm_sec;
    sec_end = hour_max*60*60;
    diff = (sec_end - sec_now)%(hour_max*60*60);
    i_second = diff % 60;
    diff -= i_second;
    diff /= 60;
    i_minute = diff % 60;
    diff -= i_minute;
    diff /= 60;
    i_hour = diff;

    snprintf ( time, 9*sizeof(char), "%02d%s%02d%s%02d", i_hour, colon, i_minute, colon, i_second);

    // Display the new texts
    text_layer_set_text(g_time_layer, time);
}

/** ----------------------------------------------------------------------------
 * @brief called when time changes
 * @param tick
 * @param units_changed
 */
static void tick_handler( struct tm *tick, TimeUnits units_changed ){
    update_time();
}

/** ----------------------------------------------------------------------------
 * @brief creates a text layer
 * @param frame
 * @param color
 * @param init_text
 * @param font
 * @param parent_layer
 * @return 
 */
static TextLayer *init_text_layer( GRect frame, 
        GColor color, 
        const char *init_text, 
        GFont font, 
        Layer *parent_layer ){

    TextLayer *layer;
    layer = text_layer_create(frame);
    text_layer_set_background_color(layer, GColorClear);
    text_layer_set_text_color(layer, color);
    text_layer_set_text(layer, init_text);

    text_layer_set_font(layer, font);
    text_layer_set_text_alignment(layer, GTextAlignmentCenter);

    layer_add_child(parent_layer, text_layer_get_layer(layer));

    return layer;
}

/** ----------------------------------------------------------------------------
 * 
 * @param window
 */
static void main_window_load(Window *window){

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);   
    s_custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_BOLD_35));

    g_time_layer = init_text_layer( GRect(0, 62, bounds.size.w, 50) , GColorWhite, "--------", s_custom_font, window_layer);
    text_layer_set_text_alignment(g_time_layer, GTextAlignmentCenter);

    effect_layer = effect_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
    effect_layer_add_effect(effect_layer, effect_mirror_horizontal, NULL);
    effect_layer_add_effect(effect_layer, effect_mirror_vertical, NULL);
    layer_add_child(window_get_root_layer(window), effect_layer_get_layer(effect_layer));

    update_time();
}

/** ----------------------------------------------------------------------------
 * 
 * @param window
 */
static void main_window_unload(Window *window){
    text_layer_destroy(g_time_layer);
    effect_layer_destroy(effect_layer);
}

/** ----------------------------------------------------------------------------
 * 
 */
static void init(){

    WindowHandlers handlers = {
        .load = main_window_load,
        .unload = main_window_unload
    };

    g_main_window = window_create();

    window_set_window_handlers( g_main_window, handlers );
    window_set_background_color( g_main_window, GColorBlack );

    window_stack_push(g_main_window, true);

    // init timer
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

/** ----------------------------------------------------------------------------
 * 
 */
static void deinit(){
    window_destroy(g_main_window);
    tick_timer_service_unsubscribe();
}

/** ----------------------------------------------------------------------------
 * 
 * @return 
 */
int main(void) {
    init();
    app_event_loop();
    deinit();
}

