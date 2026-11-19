// SPDX-License-Identifier: GPL-3.0-only
// Integration checks against SDL virtual controllers; no original game assets required.
#include "../../src/openrct2-ui/handheld/ControllerInput.h"
#include "../../src/openrct2-ui/handheld/ListFocus.h"

#include <SDL.h>
#include <cstdlib>
#include <iostream>

using namespace OpenRCT2::Ui::Handheld;

static void Check(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << " (" << SDL_GetError() << ")\n";
        std::exit(1);
    }
}

static void Pump(ControllerInput& input)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
        input.handleEvent(e);
}

int main()
{
    ListFocus focus;
    focus.move(-1, 5);
    Check(focus.index() == 4, "list focus wraps backwards");
    focus.move(1, 5);
    Check(focus.index() == 0, "list focus wraps forwards");
    focus.move(1, 0);
    Check(focus.index() == 0, "empty lists are safe");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    Check(SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) == 0, "SDL initialises");
    {
        ControllerInput input;
        Check(input.update(true, 100).actions.empty(), "no device is safe");
        const auto index = SDL_JoystickAttachVirtual(
            SDL_JOYSTICK_TYPE_GAMECONTROLLER, SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX, 0);
        Check(index >= 0, "virtual device attached");
        auto* joystick = SDL_JoystickOpen(index);
        Check(joystick != nullptr, "virtual joystick opened");
        Pump(input);
        input.update(true, 100);
        Pump(input);
        input.update(true, 110);

        auto press = [&](SDL_GameControllerButton button, bool down) {
            Check(SDL_JoystickSetVirtualButton(joystick, button, down ? 1 : 0) == 0, "set virtual button");
            SDL_JoystickUpdate();
            Pump(input);
        };

        press(SDL_CONTROLLER_BUTTON_A, true);
        auto frame = input.update(true, 120);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::confirm, "A emits exactly one mapped action");
        Check(input.update(true, 130).actions.empty(), "held A does not repeat");
        press(SDL_CONTROLLER_BUTTON_A, false);
        input.update(true, 140);

        press(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, true);
        frame = input.update(true, 190);
        Check(frame.panX == 6 && frame.panY == 0, "D-pad pans at time-based speed");
        Check(frame.navigateX == 1, "D-pad emits immediate menu step");
        Check(input.update(true, 200).navigateX == 0, "menu repetition waits for initial delay");
        frame = input.update(true, 1190);
        Check(frame.panX == 6, "long stalls are clamped");
        press(SDL_CONTROLLER_BUTTON_DPAD_LEFT, true);
        Check(input.update(true, 1240).panX == 0, "opposed directions cancel");
        press(SDL_CONTROLLER_BUTTON_DPAD_LEFT, false);
        input.update(false, 1250);
        Check(input.update(true, 1300).panX == 0, "focus regain cannot resume held movement");
        press(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, false);
        input.update(true, 1310);
        press(SDL_CONTROLLER_BUTTON_DPAD_UP, true);
        Check(input.update(true, 1360).panY == -6, "release rearms movement");
        press(SDL_CONTROLLER_BUTTON_DPAD_UP, false);

        press(SDL_CONTROLLER_BUTTON_B, true);
        input.update(false, 1370);
        Check(input.update(true, 1380).actions.empty(), "focus loss discards queued actions");
        press(SDL_CONTROLLER_BUTTON_B, false);
        input.update(true, 1390);
        press(SDL_CONTROLLER_BUTTON_B, true);
        frame = input.update(true, 1400);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::back, "B rearms after release");
        press(SDL_CONTROLLER_BUTTON_B, false);

        SDL_Event raw{};
        raw.type = SDL_JOYBUTTONDOWN;
        raw.jbutton.which = SDL_JoystickInstanceID(joystick);
        raw.jbutton.button = 0;
        Check(input.handleEvent(raw), "raw duplicate consumed");
        Check(input.update(true, 1410).actions.empty(), "raw duplicate does not execute an action");
        raw.type = SDL_CONTROLLERBUTTONDOWN;
        raw.cbutton.which = 99999;
        raw.cbutton.button = SDL_CONTROLLER_BUTTON_A;
        input.handleEvent(raw);
        Check(input.update(true, 1420).actions.empty(), "unselected controller ignored");
        raw.type = SDL_MOUSEBUTTONDOWN;
        Check(!input.handleEvent(raw), "mouse input preserved");
        raw.type = SDL_KEYDOWN;
        Check(!input.handleEvent(raw), "keyboard input preserved");
        raw.key.keysym.scancode = SDL_SCANCODE_AC_BACK;
        raw.key.repeat = 0;
        Check(input.handleEvent(raw), "Android Back key consumed");
        frame = input.update(true, 1421);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::back, "Android Back key emits back");
        raw.key.repeat = 1;
        input.handleEvent(raw);
        Check(input.update(true, 1422).actions.empty(), "Android Back key repeat ignored");
        raw.type = SDL_KEYUP;
        Check(input.handleEvent(raw), "Android Back key release consumed");

        SDL_setenv("OPENRCT2_HANDHELD_SWAP_AB", "1", 1);
        press(SDL_CONTROLLER_BUTTON_B, true);
        frame = input.update(true, 1430);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::confirm, "Brick A/B swap emits confirm");
        press(SDL_CONTROLLER_BUTTON_B, false);
        SDL_setenv("OPENRCT2_HANDHELD_SWAP_AB", "0", 1);
        press(SDL_CONTROLLER_BUTTON_START, true);
        press(SDL_CONTROLLER_BUTTON_BACK, true);
        frame = input.update(true, 1440);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::quit, "quit chord suppresses menu/build");
        Check(input.update(true, 1450).actions.empty(), "quit chord cannot repeat");
        press(SDL_CONTROLLER_BUTTON_START, false);
        press(SDL_CONTROLLER_BUTTON_BACK, false);
        input.update(true, 1460);
        press(SDL_CONTROLLER_BUTTON_START, true);
        frame = input.update(true, 1470);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::openMenu, "Start opens menu");
        input.suspend();
        press(SDL_CONTROLLER_BUTTON_BACK, true);
        frame = input.update(true, 1480);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::quit, "staggered quit works after menu suspension");
        press(SDL_CONTROLLER_BUTTON_START, false);
        press(SDL_CONTROLLER_BUTTON_BACK, false);
        input.update(true, 1490);

        press(SDL_CONTROLLER_BUTTON_LEFTSTICK, true);
        frame = input.update(true, 1491);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::cameraMode, "L3 emits camera context action");
        press(SDL_CONTROLLER_BUTTON_LEFTSTICK, false);
        input.update(true, 1492);
        auto trigger = [&](SDL_GameControllerAxis axis, int value) {
            Check(SDL_JoystickSetVirtualAxis(joystick, axis, value) == 0, "set virtual trigger");
            SDL_JoystickUpdate();
            Pump(input);
        };
        trigger(SDL_CONTROLLER_AXIS_TRIGGERLEFT, 32767);
        frame = input.update(true, 1493);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::slopeDown, "L2 emits native slope down");
        Check(input.update(true, 1494).actions.empty(), "Held trigger cannot repeat slope");
        input.suspend();
        trigger(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 32767);
        Check(input.update(true, 1495).actions.empty(), "Held triggers cannot spill after modal handoff");
        trigger(SDL_CONTROLLER_AXIS_TRIGGERLEFT, -32768);
        trigger(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, -32768);
        input.update(true, 1496);
        trigger(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 32767);
        frame = input.update(true, 1497);
        Check(frame.actions.size() == 1 && frame.actions[0] == Action::slopeUp, "R2 rearms slope up after release");
        trigger(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, -32768);
        input.update(true, 1498);
        press(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, true);
        SDL_JoystickDetachVirtual(index);
        Pump(input);
        frame = input.update(true, 1500);
        Check(frame.panX == 0 && frame.actions.empty(), "disconnect clears movement/actions");
        SDL_JoystickClose(joystick);
    }
    SDL_Quit();
    std::cout << "PASS: SDL virtual-controller input checks\n";
}
