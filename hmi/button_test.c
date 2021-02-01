/*******************************************************************************

    Pushbutton unit test.

    COPYRIGHT NOTICE: (c) 2016 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#include  <stdbool.h>
#include  <stdint.h>
#include  <string.h>
#include  "contract.h"
#include  "button.h"




NEW_BUTTON(button_pressed_0, 0);
NEW_BUTTON(button_pressed_1, 1);
NEW_BUTTON(button_was_pressed_0, 0);
NEW_BUTTON(button_was_pressed_1, 1);
NEW_BUTTON(button_was_released_0, 0);
NEW_BUTTON(button_was_released_1, 1);
NEW_BUTTON(button_missed_0, 0);
NEW_BUTTON(button_missed_1, 1);

/*
 * Stimulus / Response test vectors
 *
 */
//                                          012345678901234567890123456789012345678901234567890123456789
static const char CLEAN_0_STIM[]         = "111110000000000000000000000000111111111111111111111111100000";
static const char PRESSED_CLEAN_0_RESP[] = "000000000000000000000000111111111111111111111111100000000000";
static const char EDGE_P_CLEAN_0_RESP[]  = "000000000000000000000000100000000000000000000000000000000000";
static const char EDGE_R_CLEAN_0_RESP[]  = "000000000000000000000000000000000000000000000000010000000000";
static const char MISSED_CLEAN_0_RESP[]  = "000000000000000000000000000000000000000000000000010000000000";

//                                          012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
static const char NOISY_0_STIM[]         = "111110100010011011000100000000000000001100000000000000000000000001110010110001100110111110111111111111111111111111100000";
static const char PRESSED_NOISY_0_RESP[] = "000000000000000000000000000000000000000000000000000000000001111111111111111111111111111111111111111111111111100000000000";
static const char EDGE_P_NOISY_0_RESP[]  = "000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000";
static const char EDGE_R_NOISY_0_RESP[]  = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000000";
static const char MISSED_NOISY_0_RESP[]  = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000000";



void _buttonTestUpdateAll_0(uint32_t button_sample) {
    buttonUpdate(button_pressed_0, button_sample);
    buttonUpdate(button_was_pressed_0, button_sample);
    buttonUpdate(button_was_released_0, button_sample);
    buttonUpdate(button_missed_0, button_sample);
}

void _buttonTestUpdateAll_1(uint32_t button_sample) {
    buttonUpdate(button_pressed_1, button_sample);
    buttonUpdate(button_was_pressed_1, button_sample);
    buttonUpdate(button_was_released_1, button_sample);
    buttonUpdate(button_missed_1, button_sample);
}

bool _buttonTestVerify(bool actual, char expected) {
    if (expected == 'x')  { // don't care
        return (true);
    }
    return (actual == ((expected == '1') ? true : false));
}

int button_UNIT_TEST(void) {
    uint32_t      stim;
    char          resp_press, resp_edge_p, resp_edge_r, resp_missed;
    int           i;
    button_obj_t  save_button_0, save_button_1;
    bool          pass = true;

    // clean released -> pressed -> released
    for (i=0; i<strlen(CLEAN_0_STIM); ++i) {
        stim = (CLEAN_0_STIM[i] == '1') ? 1 : 0;
        _buttonTestUpdateAll_0(stim);     // active low button stimulus
        _buttonTestUpdateAll_1(1 - stim); // invert stimulus for active high button

        resp_press  = PRESSED_CLEAN_0_RESP[i];
        resp_edge_p = EDGE_P_CLEAN_0_RESP[i];
        resp_edge_r = EDGE_R_CLEAN_0_RESP[i];
        resp_missed = MISSED_CLEAN_0_RESP[i];

        pass &= _buttonTestVerify(buttonPressed(button_pressed_0), resp_press);
        pass &= _buttonTestVerify(buttonPressed(button_pressed_1), resp_press);

        pass &= _buttonTestVerify(buttonWasPressed(button_was_pressed_0), resp_edge_p);
        pass &= _buttonTestVerify(buttonWasPressed(button_was_pressed_1), resp_edge_p);

        pass &= _buttonTestVerify(buttonWasReleased(button_was_released_0), resp_edge_r);
        pass &= _buttonTestVerify(buttonWasReleased(button_was_released_1), resp_edge_r);

        // save restore button_missed so that checking buttonMissed() does not clear the state bits when button.missed = 0
        save_button_0 = *button_missed_0;
        save_button_1 = *button_missed_1;
        pass &= _buttonTestVerify(buttonMissed(button_missed_0), resp_missed);
        pass &= _buttonTestVerify(buttonMissed(button_missed_1), resp_missed);
        if (resp_missed == '0') {
            *button_missed_0 = save_button_0;
            *button_missed_1 = save_button_1;
        }
    }

    // noisy released -> pressed -> released
    for (i=0; i<strlen(NOISY_0_STIM); ++i) {
        stim = (NOISY_0_STIM[i] == '1') ? 1 : 0;
        _buttonTestUpdateAll_0(stim);     // active low button stimulus
        _buttonTestUpdateAll_1(1 - stim); // invert stimulus for active high button

        resp_press  = PRESSED_NOISY_0_RESP[i];
        resp_edge_p = EDGE_P_NOISY_0_RESP[i];
        resp_edge_r = EDGE_R_NOISY_0_RESP[i];
        resp_missed = MISSED_NOISY_0_RESP[i];

        pass &= _buttonTestVerify(buttonPressed(button_pressed_0), resp_press);
        pass &= _buttonTestVerify(buttonPressed(button_pressed_1), resp_press);

        pass &= _buttonTestVerify(buttonWasPressed(button_was_pressed_0), resp_edge_p);
        pass &= _buttonTestVerify(buttonWasPressed(button_was_pressed_1), resp_edge_p);

        pass &= _buttonTestVerify(buttonWasReleased(button_was_released_0), resp_edge_r);
        pass &= _buttonTestVerify(buttonWasReleased(button_was_released_1), resp_edge_r);

        // save restore button_missed so that checking buttonMissed() does not clear the state bits when button.missed = 0
        save_button_0 = *button_missed_0;
        save_button_1 = *button_missed_1;
        pass &= _buttonTestVerify(buttonMissed(button_missed_0), resp_missed);
        pass &= _buttonTestVerify(buttonMissed(button_missed_1), resp_missed);
        if (resp_missed == '0') {
            *button_missed_0 = save_button_0;
            *button_missed_1 = save_button_1;
        }
    }

    return ((int) !pass);             /* return zero if all tests pass */
}









