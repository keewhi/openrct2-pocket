// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "NativeMenuArt.h"
#include "Panel.h"

#include <algorithm>
#include <array>
#include <openrct2/SpriteIds.h>
#include <optional>
#include <string>

namespace OpenRCT2::Ui::Handheld::QuickWheel
{
    // Hold L2 in the park: eight destinations around the cursor, picked by D-pad direction
    // (diagonals by pressing two). Releasing L2 takes the highlighted one. A tap (released before
    // kHoldMs) leaves it open for A / B instead, so it works as a held or a tapped menu.
    enum class Target
    {
        build,
        repeat,
        rides,
        staff,
        map,
        pause,
        finances,
        guests
    };
    struct Slot
    {
        Target target;
        int dx, dy;
        const char* label;
        uint32_t sprite;
        uint8_t frames;
    };
    // Clockwise from the top. Build is up, the park's people are on the left and right.
    inline constexpr std::array<Slot, 8> kSlots{ {
        { Target::build, 0, -1, "Build", SPR_TOOLBAR_CONSTRUCT_RIDE, 1 },
        { Target::repeat, 1, -1, "Again", SPR_TOOLBAR_CONSTRUCT_RIDE, 1 },
        { Target::rides, 1, 0, "Rides", SPR_TAB_RIDE_0, 16 },
        { Target::staff, 1, 1, "Staff", SPR_TAB_STAFF_OPTIONS_0, 7 },
        { Target::map, 0, 1, "Park map", SPR_TOOLBAR_MAP, 1 },
        { Target::pause, -1, 1, "Pause", SPR_TOOLBAR_PAUSE, 1 },
        { Target::finances, -1, 0, "Finances", SPR_TAB_FINANCES_SUMMARY_0, 8 },
        { Target::guests, -1, -1, "Guests", SPR_TAB_GUESTS_0, 8 },
    } };
    inline constexpr uint32_t kHoldMs = 250;

    struct State
    {
        bool open{};
        bool tapped{}; // Released quickly: stays open until A, B or L2 again.
        uint64_t opened{};
        std::optional<size_t> selected;
        int lastX{}, lastY{};

        void Open(uint64_t now)
        {
            *this = {};
            open = true;
            opened = now;
        }
        void Close()
        {
            *this = {};
        }
        // The slots form a 3x3 grid around an empty centre. Each D-pad press steps one cell (Up then
        // Left reaches the top-left), stepping over the centre, so every slot is reachable with
        // single presses; a true diagonal (both directions down) still jumps straight to a corner.
        void Step(int x, int y)
        {
            if (x == 0 && y == 0)
                return;
            int cx = 0, cy = 0;
            if (selected)
            {
                cx = kSlots[*selected].dx;
                cy = kSlots[*selected].dy;
            }
            int nx = std::clamp(cx + x, -1, 1), ny = std::clamp(cy + y, -1, 1);
            if (nx == 0 && ny == 0)
            {
                nx = std::clamp(nx + x, -1, 1);
                ny = std::clamp(ny + y, -1, 1);
            }
            Select(nx, ny);
        }
        void Select(int x, int y)
        {
            for (size_t i = 0; i < kSlots.size(); ++i)
                if (kSlots[i].dx == x && kSlots[i].dy == y)
                    selected = i;
        }
    };

    // repeatLabel names the Build tool "Again" reopens; paused swaps Pause for Resume.
    inline void Draw(
        Drawing::RenderTarget& rt, ScreenSize screen, const State& state, const std::string& repeatLabel, uint32_t repeatSprite,
        bool paused)
    {
        const ScreenCoordsXY centre{ screen.width / 2, screen.height / 2 - 6 };
        constexpr ScreenSize cell{ 116, 44 };
        // Stipple the park so the wheel reads as a layer above it, not part of the HUD.
        for (int y = rt.y; y < rt.y + rt.height; ++y)
            for (int x = rt.x + ((y & 1) ^ (rt.x & 1)); x < rt.x + rt.width; x += 2)
                rt.bits[(y - rt.y) * rt.LineStride() + x - rt.x] = static_cast<Drawing::PaletteIndex>(PocketPalette::border);
        for (size_t i = 0; i < kSlots.size(); ++i)
        {
            const auto& slot = kSlots[i];
            const bool diagonal = slot.dx != 0 && slot.dy != 0;
            const int rx = diagonal ? 126 : 162, ry = diagonal ? 84 : 112;
            const auto pos = centre + ScreenCoordsXY{ slot.dx * rx - cell.width / 2, slot.dy * ry - cell.height / 2 };
            const bool on = state.selected == i;
            DrawCard(rt, pos, cell, on);
            if (on)
                DrawFocusRing(rt, pos, cell);
            const bool isRepeat = slot.target == Target::repeat;
            const uint32_t sprite = isRepeat && repeatSprite ? repeatSprite : slot.sprite;
            DrawNativeMenuArt(
                rt, pos + ScreenCoordsXY{ 6, 6 }, { 30, 32 }, { sprite, isRepeat ? uint8_t{ 1 } : slot.frames }, on);
            const std::string label = slot.target == Target::pause && paused ? "Resume" : slot.label;
            if (isRepeat && !repeatLabel.empty())
            {
                DrawStrongText(
                    rt, pos + ScreenCoordsXY{ 40, 8 }, std::string(on ? "{BLACK}" : "{WHITE}") + label, cell.width - 46);
                DrawModernEllipsised(
                    rt, pos + ScreenCoordsXY{ 40, 24 }, cell.width - 46, std::string(on ? "{BLACK}" : "{MUTED}") + repeatLabel);
            }
            else
                DrawStrongText(
                    rt, pos + ScreenCoordsXY{ 40, CapCentre(cell.height) }, std::string(on ? "{BLACK}" : "{WHITE}") + label,
                    cell.width - 46);
        }
        // The hub says how to finish, in the words of the gesture being used.
        constexpr ScreenSize hub{ 116, 40 };
        const auto hubPos = centre - ScreenCoordsXY{ hub.width / 2, hub.height / 2 };
        DrawCard(rt, hubPos, hub);
        const auto name = [&](size_t i) {
            return kSlots[i].target == Target::pause && paused ? std::string("Resume") : std::string(kSlots[i].label);
        };
        const std::string top = state.selected ? name(*state.selected) : std::string("Quick menu");
        const std::string bottom = state.tapped ? (state.selected ? "A choose / B close" : "D-pad to point")
            : state.selected                    ? "Release L2"
                                                : "Point with D-pad";
        const int tw = TextWidth(top, true), bw = TextWidth(bottom);
        DrawStrongText(rt, hubPos + ScreenCoordsXY{ (hub.width - tw) / 2, 7 }, "{WHITE}" + top);
        DrawModernText(rt, hubPos + ScreenCoordsXY{ (hub.width - bw) / 2, 23 }, "{MUTED}" + bottom);
        DrawFooter(
            rt, screen,
            state.tapped
                ? std::vector<ControlHint>{ { "D-pad", "Move" }, { "A", "Choose" }, { "Y", "Photo" }, { "B", "Close" } }
                : std::vector<ControlHint>{
                      { "D-pad", "Move" }, { "L2", "Release to choose" }, { "Y", "Photo" }, { "B", "Close" } });
    }
} // namespace OpenRCT2::Ui::Handheld::QuickWheel
