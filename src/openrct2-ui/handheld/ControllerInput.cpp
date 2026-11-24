/*****************************************************************************
 * Copyright (c) 2014-2026 OpenRCT2 developers
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "ControllerInput.h"

#include <SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string_view>
#include <utility>

namespace OpenRCT2::Ui::Handheld
{
    bool IsEnabled()
    {
        static const bool enabled = [] {
            const auto* value = std::getenv("OPENRCT2_HANDHELD");
            return value != nullptr && std::string_view(value) == "1";
        }();
        return enabled;
    }

    std::optional<Action> ButtonAction(uint8_t button)
    {
        const bool swapped = std::getenv("OPENRCT2_HANDHELD_SWAP_AB") != nullptr
            && std::string_view(std::getenv("OPENRCT2_HANDHELD_SWAP_AB")) == "1";
        switch (button)
        {
            case SDL_CONTROLLER_BUTTON_A:
                return swapped ? Action::back : Action::confirm;
            case SDL_CONTROLLER_BUTTON_B:
                return swapped ? Action::confirm : Action::back;
            case SDL_CONTROLLER_BUTTON_X:
                return Action::contextPrimary;
            case SDL_CONTROLLER_BUTTON_Y:
                return Action::contextSecondary;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                return Action::previousOption;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                return Action::nextOption;
            case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                return Action::cameraMode;
            case SDL_CONTROLLER_BUTTON_START:
                return Action::openMenu;
            case SDL_CONTROLLER_BUTTON_BACK:
                return Action::openBuild;
            default:
                return {};
        }
    }
    std::string_view ActionKey(Action action)
    {
        for (const auto& [button, label] :
             std::array<std::pair<uint8_t, std::string_view>, 8>{ { { SDL_CONTROLLER_BUTTON_A, "A" },
                                                                    { SDL_CONTROLLER_BUTTON_B, "B" },
                                                                    { SDL_CONTROLLER_BUTTON_X, "X" },
                                                                    { SDL_CONTROLLER_BUTTON_Y, "Y" },
                                                                    { SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "L1" },
                                                                    { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "R1" },
                                                                    { SDL_CONTROLLER_BUTTON_START, "START" },
                                                                    { SDL_CONTROLLER_BUTTON_BACK, "SELECT" } } })
            if (ButtonAction(button) == action)
                return label;
        return action == Action::slopeDown ? "L2" : action == Action::slopeUp ? "R2" : "";
    }
    ControllerInput::~ControllerInput()
    {
        if (_controller != nullptr)
            SDL_GameControllerClose(_controller);
        if (_initialised)
            SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    void ControllerInput::suspend()
    {
        _armed = false;
        _actions.clear();
        _panX = _panY = 0;
        _lastTick = 0;
        _navigationX = _navigationY = 0;
        _repeatAt = 0;
    }

    void ControllerInput::refresh()
    {
        if (!_initialised)
        {
            if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0)
                return;
            _initialised = true;
        }
        if (_controller != nullptr && !SDL_GameControllerGetAttached(_controller))
        {
            SDL_GameControllerClose(_controller);
            _controller = nullptr;
            _instance = -1;
            suspend();
            _scan = true;
        }
        if (_controller == nullptr && _scan)
        {
            _scan = false;
            for (int i = 0; i < SDL_NumJoysticks(); ++i)
            {
                if (!SDL_IsGameController(i))
                    continue;
                _controller = SDL_GameControllerOpen(i);
                if (_controller != nullptr)
                {
                    _instance = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(_controller));
                    SDL_Log("Handheld controller: %s (instance %d)", SDL_GameControllerName(_controller), _instance);
                    suspend();
                    break;
                }
            }
        }
    }

    bool ControllerInput::handleEvent(const SDL_Event& event)
    {
        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST || event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    suspend();
                return false;
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
                _scan = true;
                suspend();
                return true;
            case SDL_CONTROLLERBUTTONDOWN:
                if (_armed && event.cbutton.which == _instance)
                {
                    if (const auto action = ButtonAction(event.cbutton.button))
                        _actions.push_back(*action);
                }
                return true;
            case SDL_CONTROLLERBUTTONUP:
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Android's system Back key (trapped so it does not close the app) is Back.
                // Some Android handhelds send it instead of, or as well as, a pad button.
                if (event.key.keysym.scancode != SDL_SCANCODE_AC_BACK)
                    return false;
                if (event.type == SDL_KEYDOWN && event.key.repeat == 0 && _armed)
                    _actions.push_back(Action::back);
                return true;
            case SDL_CONTROLLERAXISMOTION:
                if (event.caxis.which == _instance
                    && (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT
                        || event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
                {
                    auto& held = event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? _leftTrigger : _rightTrigger;
                    const bool pressed = event.caxis.value > 16000;
                    if (_armed && pressed && !held)
                        _actions.push_back(
                            event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? Action::slopeDown : Action::slopeUp);
                    held = pressed;
                }
                return true;
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            case SDL_JOYHATMOTION:
            case SDL_JOYAXISMOTION:
            case SDL_JOYBALLMOTION:
                // SDL emits both raw and mapped events. Consume both; use only mapped input.
                return true;
            default:
                return false;
        }
    }

    Frame ControllerInput::update(bool focused, uint32_t now)
    {
        refresh();
        Frame result;
        if (!focused || _controller == nullptr)
        {
            suspend();
            return result;
        }
        auto held = [this](SDL_GameControllerButton button) { return SDL_GameControllerGetButton(_controller, button) != 0; };
        const auto* stickSetting = std::getenv("OPENRCT2_HANDHELD_LEFT_STICK");
        const bool useStick = stickSetting == nullptr || std::string_view(stickSetting) != "0";
        auto stickDirection = [this, useStick](SDL_GameControllerAxis axis) {
            constexpr int deadZone = 16000;
            const int value = useStick ? SDL_GameControllerGetAxis(_controller, axis) : 0;
            return (value > deadZone) - (value < -deadZone);
        };
        const int stickX = stickDirection(SDL_CONTROLLER_AXIS_LEFTX);
        const int stickY = stickDirection(SDL_CONTROLLER_AXIS_LEFTY);
        // Emergency quit must work even if Start already opened and suspended a menu.
        const bool quitChord = held(SDL_CONTROLLER_BUTTON_START) && held(SDL_CONTROLLER_BUTTON_BACK);
        if (quitChord && !_quitHeld)
        {
            _quitHeld = true;
            suspend();
            result.actions = { Action::quit };
            return result;
        }
        if (!quitChord)
            _quitHeld = false;
        if (!_armed)
        {
            bool any = false;
            for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
                any |= held(static_cast<SDL_GameControllerButton>(i));
            any |= SDL_GameControllerGetAxis(_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16000
                || SDL_GameControllerGetAxis(_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000;
            _armed = !any && stickX == 0 && stickY == 0;
            _lastTick = now;
            return result;
        }
        result.leftTrigger = _leftTrigger; // Tracked from axis events (works for virtual pads too).
        const auto elapsed = std::min<uint32_t>(now - _lastTick, 50);
        _lastTick = now;
        result.actions = std::exchange(_actions, {});
        const bool right = held(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        const bool left = held(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        const bool down = held(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        const bool up = held(SDL_CONTROLLER_BUTTON_DPAD_UP);
        const bool useDpad = right || left || down || up;
        const int x = useDpad ? right - left : stickX;
        const int y = useDpad ? down - up : stickY;
        result.heldX = x;
        result.heldY = y;
        result.backHeld = held(
            ButtonAction(SDL_CONTROLLER_BUTTON_B) == Action::back ? SDL_CONTROLLER_BUTTON_B : SDL_CONTROLLER_BUTTON_A);
        for (const auto button : { SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y })
            if (ButtonAction(button) == Action::contextSecondary && held(button))
                result.secondaryHeld = true;
        if (x != _navigationX || y != _navigationY)
        {
            result.navigateX = x;
            result.navigateY = y;
            _navigationX = x;
            _navigationY = y;
            _repeatAt = now + 320;
        }
        else if ((x != 0 || y != 0) && static_cast<int32_t>(now - _repeatAt) >= 0)
        {
            result.navigateX = x;
            result.navigateY = y;
            result.navigateRepeat = true;
            _repeatAt = now + 95;
        }
        const float speed = 120.0f * (x != 0 && y != 0 ? 0.70710678f : 1.0f);
        _panX = x == 0 ? 0 : _panX + x * speed * elapsed / 1000.0f;
        _panY = y == 0 ? 0 : _panY + y * speed * elapsed / 1000.0f;
        result.panX = static_cast<int32_t>(_panX);
        result.panY = static_cast<int32_t>(_panY);
        _panX -= result.panX;
        _panY -= result.panY;
        return result;
    }
} // namespace OpenRCT2::Ui::Handheld
