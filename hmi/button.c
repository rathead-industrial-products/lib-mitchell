/*******************************************************************************

    Pushbutton Object


    COPYRIGHT NOTICE: (c) 2016 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#include  <stdbool.h>
#include  <stdint.h>
#include  "button.h"



// ==== Button Functions ====

void  buttonUpdate(button_obj_t *const button, uint32_t const button_sample) {

    /* Debounce button by waiting until it has changed state and been stable for a period of time. */
    if (button->is_pressed) {
        if (button_sample == button->active_level)  { button->debounce_tracker  = 0; }
        else                                        { button->debounce_tracker += BUTTON_UPDATE_INTERVAL; }
    }
    else /* button released */ {
        if (button_sample != button->active_level)  { button->debounce_tracker  = 0; }
        else                                        { button->debounce_tracker += BUTTON_UPDATE_INTERVAL; }
    }

    if (button->debounce_tracker >= BUTTON_DEBOUNCE_TIME) {
        if (button_sample == button->active_level) {  // button pressed
            button->is_pressed = 1;
            button->was_pressed = 1;                  // set on released -> pressed transition
        }
        else {                                        // button released
            button->is_pressed = 0;
            button->was_released = 1;                 // set on pressed -> released transition

        }
    }

    if (button->was_pressed && button->was_released) {
        button->missed = 1;                           // button cycled through pressed and released without being read
    }
}

static void _buttonReset(button_obj_t *const button) {
    button->was_pressed  = 0;
    button->was_released = 0;
    button->missed       = 0;
}

bool buttonPressed(button_obj_t *const button) {
    _buttonReset(button);
    return (button->is_pressed == 1);
}

bool buttonWasPressed(button_obj_t *const button) {
    uint32_t bwp = button->was_pressed;
    _buttonReset(button);
    return (bwp == 1);
}

bool buttonWasReleased(button_obj_t *const button) {
    uint32_t bwr = button->was_released;
    _buttonReset(button);
    return (bwr == 1);
}

bool buttonMissed(button_obj_t *const button) {
    uint32_t missed = button->missed;
    _buttonReset(button);
    return (missed == 1);
}

