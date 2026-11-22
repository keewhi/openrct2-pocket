// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "ConceptStyle.h"
#include "Currency.h"
#include "Goal.h"
#include "Layout.h"
#include "MenuFeel.h"
#include "MenuThemes.h"

#include <SDL.h>
#include <ctime>
#include <openrct2/Game.h>
#include <openrct2/GameState.h>
#include <openrct2/localisation/Localisation.Date.h>

namespace OpenRCT2::Ui::Handheld
{
    struct HudPresentation
    {
        std::string cash, date, guests;
        bool paused{}, negativeCash{}, noMoney{};
        uint16_t speed = 1;
        int cashTrend{}; // +1 counting up after income, -1 after spending, 0 settled.
        // Wall clock and battery, when the HUD clock is on. battery is -1 without one.
        bool showClock{};
        std::string clock;
        int battery = -1;
        bool charging{};
    };

    // Battery level, read at most every 20 seconds: it changes slowly and the query is a sysfs
    // read on Linux handhelds.
    struct BatteryReading
    {
        int percent = -1;
        bool charging{};
    };
    inline BatteryReading ReadBattery(uint64_t now)
    {
        static BatteryReading cached;
        static uint64_t readAt{};
        static bool ever{};
        if (ever && now - readAt < 20000)
            return cached;
        ever = true;
        readAt = now;
        int percent = -1;
        const auto state = SDL_GetPowerInfo(nullptr, &percent);
        cached = { state == SDL_POWERSTATE_NO_BATTERY || state == SDL_POWERSTATE_UNKNOWN ? -1 : percent,
                   state == SDL_POWERSTATE_CHARGING || state == SDL_POWERSTATE_CHARGED };
        return cached;
    }
    inline std::string WallClock()
    {
        const auto now = std::time(nullptr);
        const auto minute = now / 60;
        static std::time_t cachedMinute = -1;
        static std::string cachedText;
        if (minute == cachedMinute)
            return cachedText;
        cachedMinute = minute;
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        char text[8];
        std::strftime(text, sizeof(text), "%H:%M", &local);
        cachedText = text;
        return cachedText;
    }

    // The cash readout counts toward the real balance instead of jumping, and briefly tints
    // green or gold after a noticeable change. Purely presentational: the park value is untouched.
    class CashTicker
    {
        money64 _from{}, _target{};
        uint64_t _start{};
        bool _ready{};
        int _trend{};

    public:
        static constexpr uint64_t kMs = 450;
        static constexpr money64 kNoticeable = 500; // £50 in money64 units.
        money64 Value(money64 actual, uint64_t now, bool reduceMotion)
        {
            if (!_ready || reduceMotion)
            {
                _from = _target = actual;
                _ready = true;
                _trend = 0;
                return actual;
            }
            if (actual != _target)
            {
                const auto shown = Shown(now);
                const auto delta = actual - shown;
                if (delta >= kNoticeable || delta <= -kNoticeable)
                    _trend = delta > 0 ? 1 : -1;
                _from = shown;
                _target = actual;
                _start = now;
            }
            if (now - _start >= kMs)
                _trend = 0;
            return Shown(now);
        }
        money64 Shown(uint64_t now) const
        {
            if (now - _start >= kMs)
                return _target;
            const double t = static_cast<double>(now - _start) / kMs;
            const double eased = 1.0 - (1.0 - t) * (1.0 - t) * (1.0 - t);
            return _from + static_cast<money64>((_target - _from) * eased);
        }
        int Trend() const
        {
            return _trend;
        }
    };
    inline CashTicker cashTicker;
    inline HudPresentation NativeHud()
    {
        const auto& state = getGameState();
        const auto& date = state.date;
        const bool noMoney = state.park.flags.has(ParkFlag::noMoney);
        const auto shown = noMoney ? state.park.cash
                                   : cashTicker.Value(state.park.cash, MenuFeel::Clock(), MenuFeel::reduceMotion);
        
        static int32_t cachedYear = -1;
        static int32_t cachedMonth = -1;
        static int32_t cachedDay = -1;
        static std::string cachedDate;
        const int32_t year = date.GetYear();
        const int32_t month = date.GetMonth();
        const int32_t day = date.GetDay();
        if (year != cachedYear || month != cachedMonth || day != cachedDay)
        {
            cachedYear = year;
            cachedMonth = month;
            cachedDay = day;
           const auto monthName = FormatStringID(DateGameShortMonthNames[month]);
            cachedDate = std::to_string(day + 1) + " " + monthName + " Y" + std::to_string(year + 1);
        }
        
        static uint32_t cachedGuests = UINT32_MAX;
        static std::string cachedGuestsText;
        if (state.park.numGuestsInPark != cachedGuests)
        {
            cachedGuests = state.park.numGuestsInPark;
            cachedGuestsText = std::to_string(cachedGuests);
        }
        
        HudPresentation model{ noMoney ? "No money" : FormatMoney(shown),
            cachedDate,
            cachedGuestsText,
                               GameIsPaused(),
                               state.park.cash < 0,
                               noMoney,
                               static_cast<uint16_t>(gGameSpeed >= 1 && gGameSpeed <= 8 ? 1u << (gGameSpeed - 1) : 0),
                               noMoney ? 0 : cashTicker.Trend(),
                               false,
                               {},
                               -1,
                               false };
        if (preferences.hudClock)
        {
            model.showClock = true;
            model.clock = WallClock();
            const auto battery = ReadBattery(MenuFeel::Clock());
            model.battery = battery.percent;
            model.charging = battery.charging;
        }
        return model;
    }
    inline void DrawHud(Drawing::RenderTarget& rt, ScreenSize screen, const HudPresentation& model)
    {
        using namespace Concept;
        const ScreenCoordsXY p{ Layout::margin, Layout::hudTop };
        const auto cells = Layout::HudLayout(screen, model.showClock);
        Box(rt, p, { cells.width, Layout::hudHeight });
        for (int x : { cells.cashEnd, cells.dateEnd, cells.guestsEnd, cells.clockEnd })
            GfxDrawLine(
                rt, { p + ScreenCoordsXY{ x, 6 }, p + ScreenCoordsXY{ x, 17 } },
                static_cast<Drawing::PaletteIndex>(PocketPalette::separator));
        DrawStrongText(
            rt, p + ScreenCoordsXY{ 8, 7 },
            std::string(
                model.noMoney                                   ? "{MUTED}"
                    : model.cashTrend > 0                       ? "{POSITIVE}"
                    : model.negativeCash || model.cashTrend < 0 ? "{GOLD}"
                                                                : "{WHITE}")
                + model.cash,
            cells.cashEnd - 16);
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ cells.cashEnd + 8, 7 }, cells.dateEnd - cells.cashEnd - 16, "{WHITE}" + model.date);
        // Small palette-safe people glyph. It uses the same line vocabulary as Pocket tool icons.
        const auto guest = p + ScreenCoordsXY{ cells.dateEnd + 7, 5 };
        for (int i : { 0, 5 })
        {
            GfxDrawLine(
                rt, { guest + ScreenCoordsXY{ i + 1, 1 }, guest + ScreenCoordsXY{ i + 3, 1 } },
                static_cast<Drawing::PaletteIndex>(PocketPalette::secondary));
            for (int x = i; x < i + 5; ++x)
                GfxDrawLine(
                    rt, { guest + ScreenCoordsXY{ x, 5 }, guest + ScreenCoordsXY{ x, 11 } },
                    static_cast<Drawing::PaletteIndex>(PocketPalette::secondary));
        }
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ cells.dateEnd + 23, 7 }, cells.guestsEnd - cells.dateEnd - 30, "{WHITE}" + model.guests);
        if (model.showClock)
        {
            DrawModernEllipsised(
                rt, p + ScreenCoordsXY{ cells.guestsEnd + 7, 7 }, cells.clockEnd - cells.guestsEnd - 30,
                "{WHITE}" + model.clock);
            if (model.battery >= 0)
            {
                // A battery outline with a nub, filled to the level; red when low, green on charge.
                const auto b = p + ScreenCoordsXY{ cells.clockEnd - 22, 8 };
                const auto ink = static_cast<Drawing::PaletteIndex>(PocketPalette::secondary);
                GfxDrawLine(rt, { b, b + ScreenCoordsXY{ 13, 0 } }, ink);
                GfxDrawLine(rt, { b + ScreenCoordsXY{ 0, 7 }, b + ScreenCoordsXY{ 13, 7 } }, ink);
                GfxDrawLine(rt, { b, b + ScreenCoordsXY{ 0, 7 } }, ink);
                GfxDrawLine(rt, { b + ScreenCoordsXY{ 13, 0 }, b + ScreenCoordsXY{ 13, 7 } }, ink);
                GfxDrawLine(rt, { b + ScreenCoordsXY{ 14, 2 }, b + ScreenCoordsXY{ 14, 5 } }, ink);
                const int fill = std::clamp(model.battery, 0, 100) * 11 / 100;
                const auto level = static_cast<Drawing::PaletteIndex>(
                    model.charging            ? PocketPalette::positive
                        : model.battery <= 15 ? PocketPalette::destructive
                                              : PocketPalette::primary);
                for (int y = 2; y <= 5 && fill > 0; ++y)
                    GfxDrawLine(rt, { b + ScreenCoordsXY{ 2, y }, b + ScreenCoordsXY{ 1 + fill, y } }, level);
            }
        }
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ cells.clockEnd + 7, 7 }, cells.width - cells.clockEnd - 14,
            model.paused      ? "{GOLD}Pause"
                : model.speed ? "{WHITE}" + std::to_string(model.speed) + "x"
                              : "{MUTED}-");
    }

    // The goal ticker: a slim card under the right of the HUD, out of the way of tool panels
    // (which dock on the left), with a thin bar along its foot.
    inline constexpr int kGoalWidth = 268, kGoalHeight = 22;
    inline ScreenCoordsXY GoalOrigin(ScreenSize screen)
    {
        return { screen.width - Layout::margin - kGoalWidth, Layout::hudTop + Layout::hudHeight + 4 };
    }
    inline void DrawGoal(Drawing::RenderTarget& rt, ScreenSize screen, const Goal::Progress& goal)
    {
        const auto p = GoalOrigin(screen);
        Concept::Box(rt, p, { kGoalWidth, kGoalHeight });
        const auto ink = goal.failed ? "{DANGER}" : goal.done ? "{POSITIVE}" : "{WHITE}";
        DrawModernText(rt, p + ScreenCoordsXY{ 7, 5 }, "{MUTED}Goal");
        const int asideWidth = goal.aside.empty() ? 0 : TextWidth(goal.aside) + 8;
        DrawModernEllipsised(rt, p + ScreenCoordsXY{ 40, 5 }, kGoalWidth - 48 - asideWidth, ink + goal.text);
        if (!goal.aside.empty())
            DrawModernText(
                rt, p + ScreenCoordsXY{ kGoalWidth - asideWidth, 5 }, (goal.asideWarns ? "{CAUTION}" : "{MUTED}") + goal.aside);
        if (!goal.done && !goal.failed)
        {
            const int track = kGoalWidth - 8;
            const auto foot = p + ScreenCoordsXY{ 4, kGoalHeight - 3 };
            GfxDrawLine(
                rt, { foot, foot + ScreenCoordsXY{ track, 0 } }, static_cast<Drawing::PaletteIndex>(PocketPalette::separator));
            const int filled = track * goal.percent / 100;
            // Two pixels high so the fill reads at a glance on the device's small screen.
            for (int y = 0; y < 2 && filled > 0; ++y)
                GfxDrawLine(
                    rt, { foot + ScreenCoordsXY{ 0, y - 1 }, foot + ScreenCoordsXY{ filled, y - 1 } },
                    static_cast<Drawing::PaletteIndex>(
                        goal.percent >= 100 ? PocketPalette::positive : PocketPalette::selection));
        }
    }
} // namespace OpenRCT2::Ui::Handheld
