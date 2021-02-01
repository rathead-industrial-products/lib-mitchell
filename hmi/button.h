/**
 *
 *  @file  button.h
 *  @brief Pushbutton object.
 *
 *  Mechanical pushbutton reader, debouncer, and state tracker.
 *
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _button_H_
#define _button_H_

#include  <stdbool.h>
#include  <stdint.h>


// ==== Typedefs ====

/// Button Object
typedef struct button_obj_t {
    uint16_t      debounce_tracker;     ///< used by debounce algorithm, i.e. number of consecutive 1's or 0's
    uint16_t      active_level  :1;     ///< defines the "pressed" logic level as 1 or 0
    uint16_t      is_pressed    :1;     ///< set if (debounced) button is currently pressed
    uint16_t      was_pressed   :1;     ///< set on released -> pressed transition
    uint16_t      was_released  :1;     ///< set on pressed -> released transition
    uint16_t      missed        :1;     ///< a button state was missed - set if was_pressed & was_released
} button_obj_t;



// ==== Defines ====

/// The rate at which the button GPIO bit is sampled and buttonUpdate() is called in mSec
#define BUTTON_UPDATE_INTERVAL          1       // ms

/// Button state must be stable this long before debounced response.
#define BUTTON_DEBOUNCE_TIME            20      // ms

/// New storage allocator for the 'name' button object. 'name' is a constant pointer with static scope.
/// State bits and the debounce tracker are set to their inactive levels.
#define NEW_BUTTON(name, active_level)                                        \
static button_obj_t name##button_obj_struct = { (active_level) ? 0 : 0xffff, active_level, 0, 0, 0, 0 }; \
static button_obj_t *const name = &name##button_obj_struct



// ==== Button Functions ====

/// Debounce button_sample and update the button object.
void buttonUpdate(button_obj_t *const button, uint32_t const button_sample);

/// Return true if the button is currently pressed.
bool buttonPressed(button_obj_t *const button);

/// Return true if the button has been pressed since the last read of the button state.
bool buttonWasPressed(button_obj_t *const button);

/// Return true if the button has been released since the last read of the button state.
bool buttonWasReleased(button_obj_t *const button);

/// Return true if the button has been pressed and released (or released and pressed) since the last read of the button state.
bool buttonMissed(button_obj_t *const button);




#endif  /* _button_H_ */


