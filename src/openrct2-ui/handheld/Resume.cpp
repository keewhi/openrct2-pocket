// SPDX-License-Identifier: GPL-3.0-only
#include "Resume.h"

#include "MenuFeel.h"
#include "MenuThemes.h"
#include "SaveSlots.h"

#include <openrct2/Context.h>
#include <openrct2/Diagnostic.h>
#include <openrct2/GameState.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/PlatformEnvironment.h>
#include <openrct2/config/Config.h>
#include <openrct2/scenario/Scenario.h>

namespace OpenRCT2::Ui::Handheld::Resume
{
    namespace
    {
        constexpr uint64_t kThrottleMs = 60000;
        // SDL sends "minimised" and "entering the background" back to back; save once for both.
        constexpr uint64_t kBackgroundThrottleMs = 2000;
        uint64_t lastSave{};
        bool everSaved{};
        bool discarded{};

        void LoadPreferences()
        {
            MenuThemes::settings.Load(GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user));
        }
    } // namespace

    std::filesystem::path Path()
    {
        const auto saves = GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user, DirId::saves);
        return std::filesystem::u8path(saves) / "autosave" / "pocket-resume.park";
    }

    bool Wanted()
    {
        LoadPreferences();
        return preferences.saveOnSleep && gLegacyScene == LegacyScene::playing;
    }

    bool Save()
    {
        if (discarded || !Wanted())
            return false;
        // Same atomic write as the save slots: a write cut short by power loss leaves the
        // previous resume save in place rather than a damaged file.
        const bool saved = WriteSaveAtomically(Path(), [](const std::filesystem::path& temporary) {
            SaveFlags flags{};
            flags.set(SaveFlag::exportObjects, Config::Get().general.savePluginData);
            return ScenarioSave(getGameState(), temporary.u8string(), flags) != 0;
        });
        if (saved)
        {
            lastSave = MenuFeel::Clock();
            everSaved = true;
        }
        else
            LOG_WARNING("Pocket: could not write the resume save");
        return saved;
    }

    void OnEvent(const SDL_Event& event)
    {
        bool sleeping = false;
        switch (event.type)
        {
            case SDL_KEYDOWN:
                sleeping = event.key.keysym.scancode == SDL_SCANCODE_POWER || event.key.keysym.scancode == SDL_SCANCODE_SLEEP;
                break;
            case SDL_APP_WILLENTERBACKGROUND:
                // Android may stop a backgrounded app without another word, so this save skips
                // the minute throttle: a focus-loss save earlier on must not leave it stale.
                if (!everSaved || MenuFeel::Clock() - lastSave >= kBackgroundThrottleMs)
                    Save();
                return;
            case SDL_WINDOWEVENT:
                sleeping = event.window.event == SDL_WINDOWEVENT_FOCUS_LOST || event.window.event == SDL_WINDOWEVENT_MINIMIZED;
                break;
            default:
                break;
        }
        if (!sleeping || (everSaved && MenuFeel::Clock() - lastSave < kThrottleMs))
            return;
        Save();
    }

    bool BeforeQuit()
    {
        // Not throttled: this is the last chance. After a deliberate discard there is nothing to
        // keep, so the game may close without asking.
        return discarded || Save();
    }

    void Discard()
    {
        // Final for this session: the system closing the game a moment later (SDL_QUIT) or the
        // screen losing focus must not write the park straight back.
        discarded = true;
        std::error_code error;
        std::filesystem::remove(Path(), error);
    }

    bool LaunchContinue(const std::filesystem::path& newest)
    {
        static bool attempted;
        if (attempted)
            return false;
        attempted = true;
        LoadPreferences();
        return preferences.resumeOnLaunch && !newest.empty() && GetContext()->LoadParkFromFile(newest.u8string());
    }
} // namespace OpenRCT2::Ui::Handheld::Resume
