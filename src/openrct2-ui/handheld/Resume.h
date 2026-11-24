// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <SDL.h>
#include <filesystem>

namespace OpenRCT2::Ui::Handheld::Resume
{
    // Keeping a park safe when the device, not the player, ends the session.
    //
    // Handheld frontends close a game with SIGTERM (which SDL turns into SDL_QUIT) when the
    // player exits through the system menu or the battery runs out, and power off or suspend the
    // device without asking. With "Save when the device sleeps" on, the park is written to
    // autosave/pocket-resume.park:
    // - on SDL_QUIT, before the game closes;
    // - when the power or sleep key arrives (many handhelds expose the power button as a key),
    //   when the window loses focus or is minimised, or the app is sent to the background.
    // The title's Continue picks the newest park, so this one is offered next launch.
    // Saving during play is throttled to once a minute: a power key can repeat, and focus can
    // flicker while a frontend overlay opens. Going to the background (Android) always saves,
    // because the system may then end the process without telling the game.
    std::filesystem::path Path();
    // A park is being played (not the title or an editor) and the preference is on.
    bool Wanted();
    bool Save();
    // From the SDL event loop; never consumes the event.
    void OnEvent(const SDL_Event& event);
    // On SDL_QUIT, just before the game closes. True when the park is saved.
    bool BeforeQuit();
    // The player quit without saving on purpose: drop the resume save so Continue does not bring
    // back what they chose to discard.
    void Discard();

    // Continue last park on launch: the title calls this once; true if a park is loading.
    bool LaunchContinue(const std::filesystem::path& newest);
} // namespace OpenRCT2::Ui::Handheld::Resume
