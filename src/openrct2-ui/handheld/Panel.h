// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "ConceptStyle.h"
#include "ListScroll.h"
#include "MenuFeel.h"
#include "ModernText.h"
#include "NativeMenuArt.h"

#include <array>
#include <cmath>
#include <openrct2-ui/interface/Window.h>
#include <openrct2/SpriteIds.h>
#include <openrct2/drawing/ColourMap.h>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/drawing/Line.h>
#include <openrct2/drawing/PaletteIndex.h>
#include <openrct2/drawing/Rectangle.h>
#include <openrct2/drawing/Text.h>
#include <sstream>
#include <string_view>

namespace OpenRCT2::Ui::Handheld
{
    inline constexpr int32_t kPanelRowPitch = 32;

    // Stable entries in OpenRCT2's default palette; never use cycling water colours.
    // The palette has no continuous RGB navy ramp, so combine ink blue with slate edges.
    namespace Theme
    {
        inline Drawing::PaletteIndex ink = static_cast<Drawing::PaletteIndex>(PocketPalette::panel);
        inline Drawing::PaletteIndex body = static_cast<Drawing::PaletteIndex>(PocketPalette::body);
        inline Drawing::PaletteIndex raised = static_cast<Drawing::PaletteIndex>(PocketPalette::raised);
        inline Drawing::PaletteIndex edge = static_cast<Drawing::PaletteIndex>(PocketPalette::edge);
        inline Drawing::PaletteIndex highlight = static_cast<Drawing::PaletteIndex>(PocketPalette::highlight);
        inline Drawing::PaletteIndex gold = static_cast<Drawing::PaletteIndex>(PocketPalette::selection);
        inline Drawing::PaletteIndex goldLight = static_cast<Drawing::PaletteIndex>(PocketPalette::selectionLight);
        inline Drawing::PaletteIndex goldShadow = static_cast<Drawing::PaletteIndex>(PocketPalette::shadow);
        inline Drawing::PaletteIndex text = static_cast<Drawing::PaletteIndex>(PocketPalette::secondary);
        inline void Refresh()
        {
            ink = static_cast<Drawing::PaletteIndex>(PocketPalette::panel);
            body = static_cast<Drawing::PaletteIndex>(PocketPalette::body);
            raised = static_cast<Drawing::PaletteIndex>(PocketPalette::raised);
            edge = static_cast<Drawing::PaletteIndex>(PocketPalette::edge);
            highlight = static_cast<Drawing::PaletteIndex>(PocketPalette::highlight);
            gold = static_cast<Drawing::PaletteIndex>(PocketPalette::selection);
            goldLight = static_cast<Drawing::PaletteIndex>(PocketPalette::selectionLight);
            goldShadow = static_cast<Drawing::PaletteIndex>(PocketPalette::shadow);
            text = static_cast<Drawing::PaletteIndex>(PocketPalette::secondary);
        }
    } // namespace Theme

    inline void Fill(Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, Drawing::PaletteIndex colour)
    {
        if (size.width > 0 && size.height > 0)
            Drawing::Rectangle::fill(rt, { pos, pos + ScreenCoordsXY{ size.width - 1, size.height - 1 } }, colour);
    }

    // Reports a focused card to the pointer in the frame's top-level coordinates. List and
    // grid rows draw into scrolled sub-targets; their pixel offset from the base target gives
    // the translation, and rows scrolled out of view are ignored.
    inline void ReportSelected(const Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size)
    {
        if (activeMenuFeel == nullptr)
            return;
        if (pos.x + size.width <= rt.x || pos.x >= rt.x + rt.width || pos.y + size.height <= rt.y || pos.y >= rt.y + rt.height)
            return;
        const auto& base = activeMenuFeel->base;
        if (base.bits == nullptr || base.stride <= 0)
        {
            activeMenuFeel->Selected({ pos.x, pos.y, size.width, size.height });
            return;
        }
        const auto offset = reinterpret_cast<const uint8_t*>(rt.bits) - base.bits;
        auto dy = offset / base.stride;
        auto dx = offset % base.stride;
        if (dx < 0)
        {
            dx += base.stride;
            --dy;
        }
        activeMenuFeel->Selected(
            { pos.x - rt.x + static_cast<int>(dx) + base.x, pos.y - rt.y + static_cast<int>(dy) + base.y, size.width,
              size.height });
    }

    // Five-pixel rounded corners and quiet single-line edges, at native resolution.
    inline void DrawCard(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, bool selected = false, bool button = false)
    {
        if (PocketPalette::Pixel())
        {
            // Windows: navy outline with notched corners and a pale inner frame line.
            const auto face = selected ? Theme::gold : button ? Theme::raised : Theme::ink;
            if (size.width >= 4 && size.height >= 4)
            {
                Fill(rt, pos + ScreenCoordsXY{ 2, 0 }, { size.width - 4, 1 }, Theme::edge);
                Fill(rt, pos + ScreenCoordsXY{ 1, 1 }, { size.width - 2, 1 }, Theme::edge);
                Fill(rt, pos + ScreenCoordsXY{ 0, 2 }, { 2, size.height - 4 }, Theme::edge);
                Fill(rt, pos + ScreenCoordsXY{ size.width - 2, 2 }, { 2, size.height - 4 }, Theme::edge);
                Fill(rt, pos + ScreenCoordsXY{ 1, size.height - 2 }, { size.width - 2, 1 }, Theme::edge);
                Fill(rt, pos + ScreenCoordsXY{ 2, size.height - 1 }, { size.width - 4, 1 }, Theme::edge);
                Fill(rt, pos + ScreenCoordsXY{ 2, 2 }, { size.width - 4, size.height - 4 }, face);
            }
            else
            {
                Fill(rt, pos, size, Theme::edge);
            }
            if (size.width > 12 && size.height > 12)
            {
                const auto line = static_cast<Drawing::PaletteIndex>(PocketPalette::separator);
                Fill(rt, pos + ScreenCoordsXY{ 4, 3 }, { size.width - 8, 1 }, line);
                Fill(rt, pos + ScreenCoordsXY{ 4, size.height - 4 }, { size.width - 8, 1 }, line);
                Fill(rt, pos + ScreenCoordsXY{ 3, 4 }, { 1, size.height - 8 }, line);
                Fill(rt, pos + ScreenCoordsXY{ size.width - 4, 4 }, { 1, size.height - 8 }, line);
            }
            if (selected)
                ReportSelected(rt, pos, size);
            return;
        }
        const auto edge = selected ? Theme::gold : Theme::edge;
        const auto face = selected ? Theme::gold : button ? Theme::raised : Theme::ink;
        if (size.width >= 10 && size.height >= 10)
        {
            // Non-overlapping perimeter strips and interior bands to avoid 6x software overdraw.
            Fill(rt, pos + ScreenCoordsXY{ 5, 0 }, { size.width - 10, 1 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 5, size.height - 1 }, { size.width - 10, 1 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 2, 2 }, { 3, 1 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ size.width - 5, 2 }, { 3, 1 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 2, size.height - 3 }, { 3, 1 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ size.width - 5, size.height - 3 }, { 3, 1 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 2, 3 }, { 1, 2 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ size.width - 3, 3 }, { 1, 2 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 2, size.height - 5 }, { 1, 2 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ size.width - 3, size.height - 5 }, { 1, 2 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 0, 5 }, { 1, size.height - 10 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ size.width - 1, 5 }, { 1, size.height - 10 }, edge);

            Fill(rt, pos + ScreenCoordsXY{ 5, 2 }, { size.width - 10, 1 }, face);
            Fill(rt, pos + ScreenCoordsXY{ 3, 3 }, { size.width - 6, 2 }, face);
            Fill(rt, pos + ScreenCoordsXY{ 1, 5 }, { size.width - 2, size.height - 10 }, face);
            Fill(rt, pos + ScreenCoordsXY{ 3, size.height - 5 }, { size.width - 6, 2 }, face);
            Fill(rt, pos + ScreenCoordsXY{ 5, size.height - 3 }, { size.width - 10, 2 }, face);
        }
        else
        {
            Fill(rt, pos + ScreenCoordsXY{ 5, 0 }, { size.width - 10, size.height }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 2, 2 }, { size.width - 4, size.height - 4 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 0, 5 }, { size.width, size.height - 10 }, edge);
            Fill(rt, pos + ScreenCoordsXY{ 5, 1 }, { size.width - 10, size.height - 2 }, face);
            Fill(rt, pos + ScreenCoordsXY{ 3, 3 }, { size.width - 6, size.height - 6 }, face);
            Fill(rt, pos + ScreenCoordsXY{ 1, 5 }, { size.width - 2, size.height - 10 }, face);
        }
        Fill(rt, pos + ScreenCoordsXY{ 5, 1 }, { size.width - 10, 1 }, selected ? Theme::goldLight : Theme::highlight);
        if (button)
        {
            // A two-pixel lip under every button so it reads as something to press.
            Fill(rt, pos + ScreenCoordsXY{ 5, size.height - 2 }, { size.width - 10, 1 }, Theme::goldShadow);
            Fill(rt, pos + ScreenCoordsXY{ 3, size.height - 3 }, { size.width - 6, 1 }, Theme::goldShadow);
        }
        if (selected && activeMenuFeel)
        {
            const int accentWidth = std::max(2, (size.width - 10) * activeMenuFeel->FocusPercent() / 100);
            Fill(
                rt, pos + ScreenCoordsXY{ (size.width - accentWidth) / 2, size.height - 2 }, { accentWidth, 1 },
                Theme::goldLight);
            ReportSelected(rt, pos, size);
        }
    }

    // ---- Component kit -------------------------------------------------------------------------
    // Every Pocket screen draws its buttons, stats, meters and tabs through these few shapes, so the
    // look is consistent and a skin can restyle the whole UI in one place.
    enum class ButtonKind : uint8_t
    {
        secondary, // Quiet cream card with a lip: most choices.
        primary,   // Filled with the section colour: the one thing this screen is for.
        danger     // Filled red: destructive confirmations.
    };
    // Small-radius rounded rectangle (2 px corner), crisp at the native 1x canvas.
    inline void FillRound(Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, Drawing::PaletteIndex colour)
    {
        if (PocketPalette::Pixel() && size.width >= 3 && size.height >= 3)
        {
            // Pixel theme: a single notched pixel per corner, like a GBA window.
            Fill(rt, pos + ScreenCoordsXY{ 1, 0 }, { size.width - 2, 1 }, colour);
            Fill(rt, pos + ScreenCoordsXY{ 0, 1 }, { size.width, size.height - 2 }, colour);
            Fill(rt, pos + ScreenCoordsXY{ 1, size.height - 1 }, { size.width - 2, 1 }, colour);
            return;
        }
        if (size.width < 4 || size.height < 4)
            return Fill(rt, pos, size, colour);
        Fill(rt, pos + ScreenCoordsXY{ 2, 0 }, { size.width - 4, 1 }, colour);
        Fill(rt, pos + ScreenCoordsXY{ 1, 1 }, { size.width - 2, 1 }, colour);
        Fill(rt, pos + ScreenCoordsXY{ 0, 2 }, { size.width, size.height - 4 }, colour);
        Fill(rt, pos + ScreenCoordsXY{ 1, size.height - 2 }, { size.width - 2, 1 }, colour);
        Fill(rt, pos + ScreenCoordsXY{ 2, size.height - 1 }, { size.width - 4, 1 }, colour);
    }
    inline void OutlineRound(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, Drawing::PaletteIndex colour, int thickness = 1)
    {
        for (int t = 0; t < thickness; ++t)
        {
            const auto p = pos + ScreenCoordsXY{ t, t };
            const ScreenSize s{ size.width - 2 * t, size.height - 2 * t };
            if (PocketPalette::Pixel())
            {
                Fill(rt, p + ScreenCoordsXY{ 1, 0 }, { s.width - 2, 1 }, colour);
                Fill(rt, p + ScreenCoordsXY{ 1, s.height - 1 }, { s.width - 2, 1 }, colour);
                Fill(rt, p + ScreenCoordsXY{ 0, 1 }, { 1, s.height - 2 }, colour);
                Fill(rt, p + ScreenCoordsXY{ s.width - 1, 1 }, { 1, s.height - 2 }, colour);
                continue;
            }
            Fill(rt, p + ScreenCoordsXY{ 2, 0 }, { s.width - 4, 1 }, colour);
            Fill(rt, p + ScreenCoordsXY{ 2, s.height - 1 }, { s.width - 4, 1 }, colour);
            Fill(rt, p + ScreenCoordsXY{ 0, 2 }, { 1, s.height - 4 }, colour);
            Fill(rt, p + ScreenCoordsXY{ s.width - 1, 2 }, { 1, s.height - 4 }, colour);
            Fill(rt, p + ScreenCoordsXY{ 1, 1 }, { 1, 1 }, colour);
            Fill(rt, p + ScreenCoordsXY{ s.width - 2, 1 }, { 1, 1 }, colour);
            Fill(rt, p + ScreenCoordsXY{ 1, s.height - 2 }, { 1, 1 }, colour);
            Fill(rt, p + ScreenCoordsXY{ s.width - 2, s.height - 2 }, { 1, 1 }, colour);
        }
    }
    inline Drawing::PaletteIndex ButtonFaceColour(ButtonKind kind)
    {
        return kind == ButtonKind::primary ? Theme::gold
            : kind == ButtonKind::danger   ? static_cast<Drawing::PaletteIndex>(PocketPalette::destructive)
                                           : Theme::raised;
    }
    // The focus ring sits just outside a card: the card keeps its own colour, so a focused primary
    // still reads as primary and a focused secondary never pretends to be one.
    inline void DrawFocusRing(Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, bool onAccent = false)
    {
        // Around a card already filled with the section colour the ring switches to dark ink, or
        // it would vanish into the card.
        OutlineRound(
            rt, pos - ScreenCoordsXY{ 2, 2 }, { size.width + 4, size.height + 4 },
            onAccent ? static_cast<Drawing::PaletteIndex>(PocketPalette::primary) : Theme::gold, 2);
        if (activeMenuFeel)
            ReportSelected(rt, pos, size);
    }
    inline void DrawButtonFace(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, ButtonKind kind, bool focused, bool enabled = true)
    {
        if (!enabled)
        {
            OutlineRound(rt, pos, size, Theme::edge);
            return;
        }
        if (PocketPalette::Pixel())
        {
            // Navy outline, a light top edge and a thick bottom shade: a chunky pressable block.
            const auto face = ButtonFaceColour(kind);
            const bool filled = kind != ButtonKind::secondary;
            FillRound(rt, pos, size, Theme::edge);
            Fill(rt, pos + ScreenCoordsXY{ 1, 1 }, { size.width - 2, size.height - 2 }, face);
            Fill(
                rt, pos + ScreenCoordsXY{ 1, size.height - 4 }, { size.width - 2, 3 },
                filled ? Theme::goldShadow : static_cast<Drawing::PaletteIndex>(PocketPalette::separator));
            Fill(
                rt, pos + ScreenCoordsXY{ 2, 2 }, { size.width - 4, 1 },
                filled ? Theme::goldLight : static_cast<Drawing::PaletteIndex>(PocketPalette::highlight));
            if (focused)
                DrawFocusRing(rt, pos, size, kind == ButtonKind::primary);
            return;
        }
        const auto face = ButtonFaceColour(kind);
        FillRound(rt, pos, size, kind == ButtonKind::secondary ? Theme::edge : Theme::goldShadow);
        FillRound(rt, pos + ScreenCoordsXY{ 1, 1 }, { size.width - 2, size.height - 3 }, face);
        if (kind == ButtonKind::secondary)
            Fill(rt, pos + ScreenCoordsXY{ 3, 1 }, { size.width - 6, 1 }, Theme::highlight);
        if (focused)
            DrawFocusRing(rt, pos, size, kind == ButtonKind::primary);
    }
    // A segmented switch: every option visible, the current one filled. The same look as a hero's
    // state switch, for any short set of modes (Single / Line / Fill).
    inline void DrawSegmented(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, const std::vector<std::string>& labels, size_t current)
    {
        if (labels.empty())
            return;
        FillRound(rt, pos, size, Theme::edge);
        FillRound(rt, pos + ScreenCoordsXY{ 1, 1 }, { size.width - 2, size.height - 2 }, Theme::ink);
        const int cell = (size.width - 4) / static_cast<int>(labels.size());
        for (size_t i = 0; i < labels.size(); ++i)
        {
            const auto at = pos + ScreenCoordsXY{ 2 + static_cast<int>(i) * cell, 2 };
            const bool on = i == current;
            if (on)
                FillRound(rt, at, { cell, size.height - 4 }, Theme::gold);
            const int w = TextWidth(labels[i], on);
            const auto text = std::string(on ? "{BLACK}" : "{MUTED}") + labels[i];
            const auto origin = at + ScreenCoordsXY{ (cell - w) / 2, CapCentre(size.height - 4) };
            if (on)
                DrawStrongText(rt, origin, text);
            else
                DrawModernText(rt, origin, text);
        }
    }
    // A thin rounded bar: loan against its limit, research progress, happiness. Shapes read faster
    // than numbers on a small screen.
    inline void DrawMeter(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, int width, int percent, Drawing::PaletteIndex ink, int height = 6)
    {
        FillRound(rt, pos, { width, height }, Theme::edge);
        const int filled = width * std::clamp(percent, 0, 100) / 100;
        if (filled > 0)
            FillRound(rt, pos, { std::max(filled, 4), height }, ink);
    }

    // Closest stable (non-cycling) palette entry to an RGB colour.
    inline Drawing::PaletteIndex NearestPaletteIndex(int red, int green, int blue)
    {
        int best = 1 << 30, index = 10;
        for (int i = 10; i < 230; ++i)
        {
            const auto colour = Drawing::gPalette[static_cast<uint8_t>(i)];
            const int r = colour.red - red, g = colour.green - green, b = colour.blue - blue;
            const int distance = r * r + g * g + b * b;
            if (distance < best)
            {
                best = distance;
                index = i;
            }
        }
        return static_cast<Drawing::PaletteIndex>(index);
    }

    // Each part of the game gets its own selection colour, so a glance at the highlight tells
    // you where you are. Colours stay dark enough for the light selected-label ink.
    enum class Accent : uint8_t
    {
        standard,
        rides,
        guests,
        staff,
        finances,
        research,
        marketing,
        build,
    };
    inline void ApplyAccent(Accent accent)
    {
        if (accent == Accent::standard)
            return;
        struct Rgb
        {
            int r, g, b;
        };
        static constexpr Rgb kAccents[] = {
            { 0, 0, 0 },      { 176, 48, 48 }, { 44, 92, 178 },  { 118, 70, 160 },
            { 164, 118, 24 }, { 40, 128, 72 }, { 170, 50, 120 }, { 200, 120, 30 },
        };
        const auto& c = kAccents[static_cast<size_t>(accent)];
        Theme::gold = NearestPaletteIndex(c.r, c.g, c.b);
        Theme::goldLight = NearestPaletteIndex(std::min(255, c.r + 60), std::min(255, c.g + 60), std::min(255, c.b + 60));
    }

    // RCT-style pointing glove beside the focused card. It glides between cards as focus
    // moves, with a faint outline tracing the same path, then idles with a slow bob.
    inline void DrawFocusPointer(Drawing::RenderTarget& rt, const MenuFeel& feel)
    {
        if (!feel.HasFocusRect())
            return;
        const auto now = MenuFeel::Clock();
        const auto rect = feel.FocusRect(now);
        if (feel.Gliding(now))
        {
            const auto ghost = static_cast<Drawing::PaletteIndex>(PocketPalette::positive);
            Fill(rt, { rect.x + 4, rect.y }, { rect.width - 8, 1 }, ghost);
            Fill(rt, { rect.x + 4, rect.y + rect.height - 1 }, { rect.width - 8, 1 }, ghost);
            Fill(rt, { rect.x, rect.y + 4 }, { 1, rect.height - 8 }, ghost);
            Fill(rt, { rect.x + rect.width - 1, rect.y + 4 }, { 1, rect.height - 8 }, ghost);
        }
        if (feel.PointerHidden())
            return;
        if (PocketPalette::Pixel())
        {
            // Pixel theme: the classic menu cursor, a solid triangle that nudges toward the choice.
            const auto ink = static_cast<Drawing::PaletteIndex>(PocketPalette::primary);
            const int x = std::max(0, rect.x - 9) + feel.PointerBob(now);
            const int y = rect.y + rect.height / 2;
            for (int i = 0; i < 5; ++i)
                Fill(rt, { x + i, y - 4 + i }, { 1, 9 - 2 * i }, ink);
            return;
        }
        static constexpr const char* kGlove[] = {
            "....ooo...........", "...o###o..........", "...o###oooooooooo.", "..oo#############o",
            ".o#o#############o", "o##o##oooooooooooo", "o####o###o........", "o#####oooo........",
            "o####o###o........", "o#####oooo........", ".o#####oo.........", "..ooooo...........",
        };
        const auto outline = NearestPaletteIndex(28, 28, 36);
        const auto fill = NearestPaletteIndex(255, 255, 255);
        const int x = std::max(0, rect.x - 13) + feel.PointerBob(now);
        const int y = rect.y + rect.height / 2 - 6;
        for (int row = 0; row < 12; ++row)
            for (int column = 0; kGlove[row][column] != '\0'; ++column)
            {
                const char cell = kGlove[row][column];
                if (cell != '.')
                    Fill(rt, { x + column, y + row }, { 1, 1 }, cell == 'o' ? outline : fill);
            }
    }

    inline void DrawSurface(Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size)
    {
        DrawCard(rt, pos, size);
    }

    enum class PanelIcon : uint8_t
    {
        play,
        terrain,
        save,
        exit,
        folder,
        back,
        raise,
        lower,
        path,
        remove,
        tree,
        none,
        largeScenery,
        walls,
        furniture
    };

    inline void DrawIcon(Drawing::RenderTarget& rt, ScreenCoordsXY pos, PanelIcon icon, bool selected = false)
    {
        if (icon == PanelIcon::none)
            return;
        NativeMenuArt art;
        switch (icon)
        {
            case PanelIcon::play:
                art = { SPR_TAB_RIDE_0, 16 };
                break;
            case PanelIcon::terrain:
                art = { SPR_G2_TAB_LAND };
                break;
            case PanelIcon::save:
                art = { SPR_FLOPPY };
                break;
            case PanelIcon::exit:
                art = { SPR_MENU_EXIT };
                break;
            case PanelIcon::folder:
                art = { SPR_G2_FOLDER };
                break;
            case PanelIcon::back:
                art = { SPR_PREVIOUS };
                break;
            case PanelIcon::raise:
                art = { SPR_G2_ARROW_UP };
                break;
            case PanelIcon::lower:
                art = { SPR_G2_ARROW_DOWN };
                break;
            case PanelIcon::path:
                art = { SPR_G2_BUTTON_FOOTPATH };
                break;
            case PanelIcon::remove:
                art = { SPR_DEMOLISH };
                break;
            case PanelIcon::tree:
                art = { SPR_G2_BUTTON_TREES };
                break;
            case PanelIcon::largeScenery:
                art = { SPR_G2_BUTTON_LARGE_SCENERY };
                break;
            case PanelIcon::walls:
                art = { SPR_G2_BUTTON_WALLS };
                break;
            case PanelIcon::furniture:
                art = { SPR_G2_BUTTON_PATH_ADDITIONS };
                break;
            default:
                break;
        }
        if (DrawNativeMenuArt(rt, pos + ScreenCoordsXY{ -4, -4 }, { 24, 24 }, art, selected))
            return;
        if (icon == PanelIcon::largeScenery || icon == PanelIcon::walls || icon == PanelIcon::furniture)
        {
            const auto sprite = icon == PanelIcon::largeScenery ? SPR_G2_BUTTON_LARGE_SCENERY
                : icon == PanelIcon::walls                      ? SPR_G2_BUTTON_WALLS
                                                                : SPR_G2_BUTTON_PATH_ADDITIONS;
            GfxDrawSprite(rt, ImageId(sprite), pos + ScreenCoordsXY{ -5, -6 });
            return;
        }
        if (icon == PanelIcon::tree)
        {
            GfxDrawSprite(rt, ImageId(SPR_G2_BUTTON_TREES), pos + ScreenCoordsXY{ -5, -6 });
            return;
        }
        if (icon == PanelIcon::folder)
        {
            GfxDrawSprite(rt, ImageId(SPR_G2_FOLDER), pos);
            return;
        }
        if (icon == PanelIcon::remove)
        {
            Concept::DrawIcon(rt, pos, Concept::Icon::remove, selected);
            return;
        }
        if (icon == PanelIcon::path)
        {
            GfxDrawSprite(rt, ImageId(SPR_G2_BUTTON_FOOTPATH), pos + ScreenCoordsXY{ -5, -6 });
            return;
        }
        // Native 16px pixel glyphs, independent of the user's RCT2 asset installation.
        static constexpr uint16_t glyphs[][16] = {
            { 0, 0, 0x0800, 0x0C00, 0x0E00, 0x0F00, 0x0F80, 0x0FC0, 0x0F80, 0x0F00, 0x0E00, 0x0C00, 0x0800, 0, 0, 0 },
            { 0, 0, 0, 0x0080, 0x01C0, 0x03E0, 0x07F0, 0x0FF8, 0x1FFC, 0x3FFE, 0x7FFF, 0x3FFE, 0x1FFC, 0, 0, 0 },
            { 0, 0, 0x3FFC, 0x2424, 0x2424, 0x2424, 0x27E4, 0x2004, 0x2004, 0x27E4, 0x2424, 0x2424, 0x3FFC, 0, 0, 0 },
            { 0, 0, 0x3F00, 0x2100, 0x2100, 0x2120, 0x2130, 0x27F8, 0x2130, 0x2120, 0x2100, 0x2100, 0x3F00, 0, 0, 0 },
            { 0, 0, 0, 0x3C00, 0x7FF8, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x7FF8, 0, 0, 0, 0 },
            { 0, 0, 0, 0x0200, 0x0600, 0x0C00, 0x1FF8, 0x3FF8, 0x1FF8, 0x0C00, 0x0600, 0x0200, 0, 0, 0, 0 },
            { 0, 0, 0, 0x0180, 0x03C0, 0x07E0, 0x0DB0, 0x1998, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0, 0, 0 },
            { 0, 0, 0, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x1998, 0x0DB0, 0x07E0, 0x03C0, 0x0180, 0, 0, 0 }
        };
        for (int y = 0; y < 16; ++y)
            for (int x = 0; x < 16; ++x)
                if (glyphs[static_cast<size_t>(icon)][y] & (0x8000 >> x))
                    Fill(
                        rt, pos + ScreenCoordsXY{ x, y }, { 1, 1 },
                        selected ? Theme::ink : (icon == PanelIcon::save ? Theme::highlight : Theme::goldLight));
    }

    inline void DrawPanelHeading(Drawing::RenderTarget& rt, const Window& window, const std::string& title)
    {
        DrawSurface(rt, window.windowPos, { window.width, window.height });
        DrawHeadingText(rt, window.windowPos + ScreenCoordsXY{ 12, 9 }, "{WHITE}" + title, window.width - 24);
        Fill(rt, window.windowPos + ScreenCoordsXY{ 10, 35 }, { window.width - 20, 1 }, Theme::edge);
    }

    enum class HintPriority
    {
        normal,
        secondary,
        guide
    };
    struct ControlHint
    {
        std::string key, label;
        HintPriority priority = HintPriority::normal;
    };
    inline std::vector<ControlHint> LastControlHints;
    struct ControlPlacement
    {
        ControlHint hint;
        int x, y, width;
    };
    inline std::string ControlKey(std::string key)
    {
        if (key == "L/R")
            return "L1/R1";
        if (key == "SEL")
            return "SELECT";
        return key;
    }
    inline int ControlKeyWidth(const std::string& key)
    {
        return key == "D-pad" || key.size() == 1 ? 16 : TextWidth(key, true) + 10; // Pills print the key in bold.
    }
    inline int ControlHintWidth(const ControlHint& hint)
    {
        return ControlKeyWidth(ControlKey(hint.key)) + 5 + TextWidth(hint.label) + 6;
    }
    // Face buttons are round badges in fixed colours (A teal, B red, X and Y dark), the way console
    // games print them, so the same glyph means the same button on every screen.
    // onAccent: the glyph sits on a filled button, often the same colour as the A disc, so it
    // inverts to a light disc with a dark letter instead of vanishing into the button.
    inline void DrawButtonGlyph(Drawing::RenderTarget& rt, ScreenCoordsXY centre, const std::string& key, bool onAccent = false)
    {
        const auto face = onAccent ? Theme::raised
                                   : static_cast<Drawing::PaletteIndex>(
                                         key == "A"       ? PocketPalette::positive
                                             : key == "B" ? PocketPalette::destructive
                                                          : PocketPalette::secondary);
        if (PocketPalette::Pixel())
        {
            FillRound(rt, centre - ScreenCoordsXY{ 6, 5 }, { 13, 11 }, face);
            DrawButtonLabel(rt, centre - ScreenCoordsXY{ 6, 6 }, { 13, 13 }, key, !onAccent);
            return;
        }
        // A true 13 px circle (the old 15 x 11 oval squeezed bold letters against its top and
        // bottom). The label is centred on the circle's centre pixel by optical weight.
        static constexpr int kSpan[]{ 2, 4, 5, 5, 6, 6, 6, 6, 6, 5, 5, 4, 2 };
        for (int row = 0; row < 13; ++row)
            Fill(rt, centre + ScreenCoordsXY{ -kSpan[row], row - 6 }, { 2 * kSpan[row] + 1, 1 }, face);
        DrawButtonLabel(rt, centre - ScreenCoordsXY{ 6, 6 }, { 13, 13 }, key, !onAccent);
    }
    inline void DrawCommand(Drawing::RenderTarget& rt, ScreenCoordsXY pos, int32_t, const char* key, const char* label)
    {
        const auto name = ControlKey(key);
        const int width = ControlKeyWidth(name);
        const int top = pos.y - 3;
        if (name == "D-pad")
        {
            Fill(rt, { pos.x + 5, top + 1 }, { 5, 15 }, Theme::text);
            Fill(rt, { pos.x, top + 6 }, { 15, 5 }, Theme::text);
            Fill(rt, { pos.x + 6, top + 7 }, { 3, 3 }, Theme::edge);
        }
        else if (name.size() == 1)
            DrawButtonGlyph(rt, { pos.x + width / 2, top + 8 }, name);
        else
        {
            // Shoulders, START and SELECT: a dark pill.
            const auto face = static_cast<Drawing::PaletteIndex>(PocketPalette::secondary);
            FillRound(rt, { pos.x, top + 1 }, { width, 15 }, face);
            DrawButtonLabel(rt, { pos.x, top }, { width, 17 }, name);
        }
        // Capitals centred on the glyph's centre row (top + 8).
        DrawModernText(rt, { pos.x + width + 5, top + 8 - 4 }, std::string("{WHITE}") + label);
    }
    inline void DrawFooter(Drawing::RenderTarget&, ScreenSize, const std::vector<ControlHint>&);
    inline void DrawFooterAt(Drawing::RenderTarget&, ScreenCoordsXY, ScreenSize, const std::vector<ControlHint>&);
    // Word wrapping preserves the color on every line and never splits UTF-8.
    inline void DrawWrappedLabel(Drawing::RenderTarget& rt, ScreenCoordsXY p, int width, std::string text, int lines = 2)
    {
        std::string ink = "{WHITE}";
        if (text.starts_with("{") && text.find('}') != std::string::npos)
        {
            const auto end = text.find('}') + 1;
            ink = text.substr(0, end);
            text.erase(0, end);
        }
        std::istringstream words(text);
        std::string word, line;
        int row = 0;
        while (words >> word)
        {
            const auto next = line.empty() ? word : line + " " + word;
            if (!line.empty() && TextWidth(next, true) > width && row + 1 < lines)
            {
                DrawStrongText(rt, p + ScreenCoordsXY{ 0, row++ * 16 }, ink + line, width);
                line = word;
            }
            else
                line = next;
        }
        DrawStrongText(rt, p + ScreenCoordsXY{ 0, row * 16 }, ink + line, width);
    }

    // Shared hierarchy for summaries, construction and management. These surfaces
    // deliberately distinguish information from controller-focusable actions.
    inline void DrawSectionLabel(Drawing::RenderTarget& rt, ScreenCoordsXY p, int width, const std::string& text)
    {
        DrawModernEllipsised(rt, p, width, "{MUTED}" + text);
    }

    inline void DrawMetric(
        Drawing::RenderTarget& rt, ScreenCoordsXY p, ScreenSize size, const std::string& label, const std::string& value,
        bool selected = false, const std::string& valueInk = "{WHITE}", bool hero = false)
    {
        FillRound(rt, p, size, Theme::raised);
        DrawModernEllipsised(rt, p + ScreenCoordsXY{ 8, size.height < 34 ? 5 : 8 }, size.width - 16, "{MUTED}" + label);
        if (hero && size.height >= 40)
            DrawHeadingText(rt, p + ScreenCoordsXY{ 8, 20 }, valueInk + value, size.width - 16);
        else
            DrawStrongText(
                rt,
                p
                    + ScreenCoordsXY{ 8,
                                      size.height >= 40      ? 25
                                          : size.height < 34 ? size.height - 12
                                                             : 22 },
                valueInk + value, size.width - 16);
        if (selected)
            DrawFocusRing(rt, p, size);
    }

    // Labels ending in "..." are a desktop convention for "opens another window"; on a console every
    // button opens something, so the dots are just noise.
    inline std::string ButtonText(std::string text)
    {
        while (text.ends_with("...") || text.ends_with("\xE2\x80\xA6"))
            text.erase(text.size() - 3); // "..." and U+2026 are both three bytes
        return text;
    }
    inline void DrawActionRow(
        Drawing::RenderTarget& rt, ScreenCoordsXY p, ScreenSize size, const std::string& rawTitle, const std::string& subtitle,
        bool focused, bool enabled = true, PanelIcon icon = PanelIcon::none, bool destructive = false,
        bool allowNativeArt = true, bool primary = false, bool centred = false)
    {
        const int press = ButtonPressOffset(focused);
        p.y += press;
        size.height -= press;
        const auto title = ButtonText(rawTitle);
        const auto kind = !enabled ? ButtonKind::secondary
            : destructive          ? ButtonKind::danger
            : primary              ? ButtonKind::primary
                                   : ButtonKind::secondary;
        DrawButtonFace(rt, p, size, kind, focused, enabled);
        const bool filled = enabled && kind != ButtonKind::secondary;
        const auto colour = !enabled ? "{DISABLED}" : filled ? "{BLACK}" : "{WHITE}";
        const bool roomForArt = allowNativeArt && TextWidth(title, true) + 58 <= size.width;
        const bool native = roomForArt
            && DrawNativeMenuArt(rt, p + ScreenCoordsXY{ 5, (size.height - 24) / 2 }, { 26, 24 }, ActionArt(rawTitle), filled);
        int inset = native || (roomForArt && icon != PanelIcon::none) ? 36 : 10;
        // Short labels in a row of equal buttons (toolbars) read as a set when centred. The A glyph
        // takes the right edge when focused, so the label never slides under it.
        if (centred && !native)
            inset = std::clamp(
                (size.width - TextWidth(title, true)) / 2, 6,
                std::max(6, size.width - (focused && enabled ? 22 : 8) - TextWidth(title, true)));
        if (!native && roomForArt)
            DrawIcon(rt, p + ScreenCoordsXY{ 9, 8 }, icon, filled);
        // The focused button carries the A glyph where a desktop menu would put a chevron: it says
        // which button acts and how, without reading the footer.
        const int glyphRoom = focused && enabled ? 22 : 8;
        DrawStrongText(
            rt, p + ScreenCoordsXY{ inset, subtitle.empty() || size.height < 34 ? CapCentre(size.height - 2) : 8 },
            std::string(colour) + title, size.width - inset - glyphRoom);
        if (!subtitle.empty() && size.height >= 34)
            DrawModernEllipsised(
                rt, p + ScreenCoordsXY{ inset, size.height - 13 }, size.width - inset - glyphRoom,
                std::string(
                    !enabled     ? "{DISABLED}"
                        : filled ? "{BLACK}"
                                 : "{MUTED}")
                    + subtitle);
        if (focused && enabled)
            DrawButtonGlyph(rt, p + ScreenCoordsXY{ size.width - 12, (size.height - 2) / 2 }, "A", filled);
    }

    inline void DrawListBackground(Drawing::RenderTarget& rt, ScreenCoordsXY p, ScreenSize size, bool focused)
    {
        if (focused)
        {
            FillRound(rt, p, { size.width, size.height - 1 }, Theme::raised);
            DrawFocusRing(rt, p, { size.width, size.height - 1 });
        }
        else
            Fill(rt, p + ScreenCoordsXY{ 4, size.height - 1 }, { size.width - 8, 1 }, Theme::edge);
    }
    // Rosters and long lists are rows, not a stack of buttons: plain text on the panel with a hairline
    // between entries. The focused row lifts into a card with the ring and the A glyph.
    inline void DrawListRow(
        Drawing::RenderTarget& rt, ScreenCoordsXY p, ScreenSize size, const std::string& rawTitle,
        const std::string& rawSubtitle, bool focused, bool enabled = true, bool allowNativeArt = true)
    {
        const auto title = ButtonText(rawTitle);
        auto subtitle = rawSubtitle;
        for (auto at = subtitle.find(" / "); at != std::string::npos; at = subtitle.find(" / ", at + 5))
            subtitle.replace(at, 3, "  |  ");
        DrawListBackground(rt, p, size, focused);
        const bool roomForArt = allowNativeArt && TextWidth(title, true) + 58 <= size.width;
        const bool native = roomForArt
            && DrawNativeMenuArt(rt, p + ScreenCoordsXY{ 5, (size.height - 24) / 2 }, { 26, 24 }, ActionArt(rawTitle), false);
        const int inset = native ? 36 : 8;
        const int glyphRoom = focused && enabled ? 22 : 8;
        const auto ink = enabled ? "{WHITE}" : "{DISABLED}";
        DrawStrongText(
            rt, p + ScreenCoordsXY{ inset, subtitle.empty() || size.height < 34 ? CapCentre(size.height - 1) : 6 },
            std::string(ink) + title, size.width - inset - glyphRoom);
        if (!subtitle.empty() && size.height >= 34)
            DrawModernEllipsised(
                rt, p + ScreenCoordsXY{ inset, size.height - 15 }, size.width - inset - glyphRoom,
                std::string(enabled ? "{MUTED}" : "{DISABLED}") + subtitle);
        if (focused && enabled)
            DrawButtonGlyph(rt, p + ScreenCoordsXY{ size.width - 12, size.height / 2 - 1 }, "A");
    }

    inline void DrawIconRow(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, int32_t width, const std::string& label, bool selected, PanelIcon icon)
    {
        DrawActionRow(rt, pos, { width, 28 }, label, "", selected, true, icon);
    }

    inline void DrawPanelRow(
        Drawing::RenderTarget& rt, const Window& window, int32_t y, const std::string& label, bool selected)
    {
        DrawIconRow(rt, { window.windowPos.x + 10, y - 8 }, window.width - 20, label, selected, PanelIcon::play);
    }

    // The pause screen is the game's home base, so it gets a proper layout: the park at a glance on
    // top, game actions on the left (Resume first and filled), and settings on the right as compact
    // choices that show their current value. Rows keep their order, so focus still moves down the
    // left column and then the right.
    struct PauseSummary
    {
        std::string park, date, cash, guests, simulation;
    };
    inline void DrawSettingRow(
        Drawing::RenderTarget& rt, ScreenCoordsXY p, int width, const std::string& label, const std::string& value,
        bool current, bool focused)
    {
        if (focused)
        {
            FillRound(rt, p, { width, 22 }, Theme::raised);
            DrawFocusRing(rt, p, { width, 22 });
        }
        // Radio mark: filled when this is the current choice.
        OutlineRound(rt, p + ScreenCoordsXY{ 6, 6 }, { 10, 10 }, Theme::text);
        if (current)
            FillRound(rt, p + ScreenCoordsXY{ 8, 8 }, { 6, 6 }, Theme::gold);
        DrawModernEllipsised(rt, p + ScreenCoordsXY{ 22, 6 }, width - 30, std::string(current ? "{WHITE}" : "{MUTED}") + label);
        if (!value.empty())
        {
            const int w = TextWidth(value, true);
            DrawStrongText(rt, p + ScreenCoordsXY{ width - 8 - w, 6 }, "{WHITE}" + value);
        }
    }
    // A settings row with a toggle switch on the right: the knob slides right and the track fills
    // when on. value, when given, replaces the switch with text (for multi-choice rows).
    inline void DrawToggleRow(
        Drawing::RenderTarget& rt, ScreenCoordsXY p, int width, const std::string& label, bool on, bool focused,
        const std::string& value = {})
    {
        constexpr int height = 26;
        if (focused)
        {
            FillRound(rt, p, { width, height }, Theme::raised);
            DrawFocusRing(rt, p, { width, height });
        }
        DrawModernEllipsised(rt, p + ScreenCoordsXY{ 10, CapCentre(height) }, width - 70, "{WHITE}" + label);
        if (!value.empty())
        {
            const int w = TextWidth(value, true);
            DrawStrongText(rt, p + ScreenCoordsXY{ width - 10 - w, CapCentre(height) }, "{WHITE}" + value);
            return;
        }
        const ScreenCoordsXY track = p + ScreenCoordsXY{ width - 42, (height - 14) / 2 };
        FillRound(rt, track, { 32, 14 }, Theme::edge);
        FillRound(rt, track + ScreenCoordsXY{ 1, 1 }, { 30, 12 }, on ? Theme::gold : Theme::ink);
        FillRound(rt, track + ScreenCoordsXY{ on ? 19 : 2, 2 }, { 11, 10 }, on ? Theme::raised : Theme::edge);
    }
    inline void DrawPauseScreen(
        Drawing::RenderTarget& rt, ScreenSize screen, const std::vector<std::string>& rows, size_t selected,
        const PauseSummary& summary, const std::string& status)
    {
        const int width = std::min(screen.width - 16, 496);
        const int left = (screen.width - width) / 2;
        const int bottom = screen.height - 34;
        // Park at a glance.
        DrawCard(rt, { left, 8 }, { width, 44 });
        DrawHeadingText(rt, { left + 12, 14 }, "{WHITE}" + summary.park, width / 2);
        const std::string glance = summary.date + "   " + summary.cash + "   " + summary.guests;
        DrawModernEllipsised(rt, { left + 12, 35 }, width - 24, "{MUTED}" + glance);
        const int pillWidth = TextWidth(summary.simulation, true) + 18;
        FillRound(rt, { left + width - 12 - pillWidth, 14 }, { pillWidth, 18 }, Theme::gold);
        DrawStrongText(rt, { left + width - 12 - pillWidth + 9, 17 }, "{BLACK}" + summary.simulation);
        // Game actions.
        const int columnTop = 60;
        const int actionsWidth = width * 56 / 100;
        DrawCard(rt, { left, columnTop }, { actionsWidth, bottom - columnTop });
        DrawSectionLabel(rt, { left + 12, columnTop + 8 }, actionsWidth - 24, "GAME");
        const size_t actions = std::min<size_t>(8, rows.size());
        const int pitch = std::min(32, (bottom - columnTop - 30) / static_cast<int>(std::max<size_t>(1, actions)));
        for (size_t i = 0; i < actions; ++i)
            DrawActionRow(
                rt, { left + 10, columnTop + 24 + static_cast<int>(i) * pitch }, { actionsWidth - 20, pitch - 3 }, rows[i], "",
                i == selected, true, PanelIcon::none, false, true, i == 0);
        // Settings.
        const int settingsLeft = left + actionsWidth + 8;
        const int settingsWidth = width - actionsWidth - 8;
        DrawCard(rt, { settingsLeft, columnTop }, { settingsWidth, bottom - columnTop });
        DrawSectionLabel(rt, { settingsLeft + 12, columnTop + 8 }, settingsWidth - 24, "SPEED");
        int y = columnTop + 24;
        for (size_t i = 8; i < rows.size() && i < 12; ++i, y += 24)
        {
            const bool current = rows[i].starts_with("Current: ");
            DrawSettingRow(
                rt, { settingsLeft + 8, y }, settingsWidth - 16, current ? rows[i].substr(9) : rows[i], "", current,
                i == selected);
        }
        y += 8;
        DrawSectionLabel(rt, { settingsLeft + 12, y }, settingsWidth - 24, "DISPLAY");
        y += 16;
        for (size_t i = 12; i < rows.size(); ++i, y += 24)
        {
            const auto colon = rows[i].find(": ");
            const auto label = colon == std::string::npos ? rows[i] : rows[i].substr(0, colon);
            const auto value = colon == std::string::npos ? std::string() : rows[i].substr(colon + 2);
            if (i == selected)
            {
                FillRound(rt, { settingsLeft + 8, y }, { settingsWidth - 16, 22 }, Theme::raised);
                DrawFocusRing(rt, { settingsLeft + 8, y }, { settingsWidth - 16, 22 });
            }
            DrawModernEllipsised(rt, { settingsLeft + 16, y + 6 }, settingsWidth / 2, "{MUTED}" + label);
            const int w = TextWidth(value, true);
            DrawStrongText(rt, { settingsLeft + settingsWidth - 16 - w, y + 6 }, "{WHITE}" + value);
        }
        if (!status.empty())
            DrawModernEllipsised(rt, { settingsLeft + 12, bottom - 18 }, settingsWidth - 24, "{MUTED}" + status);
    }

    inline void DrawPagePosition(
        Drawing::RenderTarget& rt, ScreenCoordsXY p, int width, size_t selected, size_t count, size_t capacity)
    {
        if (count <= capacity || !capacity)
            return;
        DrawModernEllipsised(
            rt, p, width,
            "{MUTED}Page " + std::to_string(selected / capacity + 1) + " / "
                + std::to_string((count + capacity - 1) / capacity));
    }

    inline void DrawPanelCommands(Drawing::RenderTarget& rt, const Window& window, const char* backLabel = "Back")
    {
        const auto pos = window.windowPos + ScreenCoordsXY{ 1, window.height - 29 };
        Fill(rt, pos, { window.width - 2, 28 }, Theme::raised);
        Fill(rt, pos, { window.width - 2, 1 }, Theme::edge);
        DrawFooterAt(
            rt, window.windowPos, { window.width, window.height },
            { { "D-pad", "Choose" }, { "A", "Select" }, { "B", backLabel } });
    }

    // One segmented strip with the shoulder buttons that move through it at either end, instead of
    // four separate button-shaped tabs that look like more things to press.
    inline void DrawTabs(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, int32_t width, const std::vector<std::string>& labels, size_t selected,
        std::string_view context = {})
    {
        if (labels.empty())
            return;
        constexpr int height = 26;
        FillRound(rt, pos, { width, height }, Theme::edge);
        FillRound(rt, pos + ScreenCoordsXY{ 1, 1 }, { width - 2, height - 2 }, Theme::raised);
        DrawModernText(rt, pos + ScreenCoordsXY{ 6, CapCentre(height) }, "{MUTED}L1");
        DrawModernText(rt, pos + ScreenCoordsXY{ width - 6 - TextWidth("R1"), CapCentre(height) }, "{MUTED}R1");
        const int inner = width - 50;
        const auto tabWidth = inner / static_cast<int32_t>(labels.size());
        for (size_t i = 0; i < labels.size(); ++i)
        {
            const auto tabPos = pos + ScreenCoordsXY{ 25 + static_cast<int32_t>(i) * tabWidth, 3 };
            const bool active = i == selected;
            if (active)
                FillRound(rt, tabPos, { tabWidth - 2, height - 6 }, Theme::gold);
            const bool showArt = TextWidth(labels[i], active) + 36 <= tabWidth;
            // Tab sprites draw at their own size, which can be taller than the strip and wider than
            // their slot. Clip each one to its slot so it never pokes through the outline or under
            // the label.
            Drawing::RenderTarget slot;
            const bool clipped = ClipRenderTarget(slot, rt, tabPos + ScreenCoordsXY{ 1, -1 }, 26, height - 4);
            const bool native = showArt && clipped
                && DrawNativeMenuArt(slot, { 1, 0 }, { 24, height - 4 }, TabArt(labels[i], context), active);
            const int inset = native ? 30 : 0; // 4 px of air after the 26 px art slot
            const int textWidth = TextWidth(labels[i], active);
            const int x = native ? inset : std::max(4, (tabWidth - 2 - textWidth) / 2);
            const auto ink = active ? std::string("{BLACK}") : std::string("{WHITE}");
            const int textY = CapCentre(height - 6);
            if (active)
                DrawStrongText(rt, tabPos + ScreenCoordsXY{ x, textY }, ink + labels[i], tabWidth - x - 4);
            else
                DrawModernEllipsised(rt, tabPos + ScreenCoordsXY{ x, textY }, tabWidth - x - 4, ink + labels[i]);
        }
    }

    struct MenuPresentation
    {
        std::string title;
        std::string section;
        std::vector<std::string> tabs;
        size_t selectedTab{};
        std::string detailTitle;
        std::string detailSubtitle;
        std::vector<std::pair<std::string, std::string>> stats;
        std::vector<std::string> help;
        std::vector<std::string> rows;
        std::vector<PanelIcon> icons;
        std::string status;
        size_t selected{};
        bool destructive{}; // The first row discards or replaces something: drawn red.
    };

    inline ScreenSize OverlaySize(size_t rows)
    {
        return { 244, 86 + static_cast<int32_t>(rows) * kPanelRowPitch };
    }

    inline void DrawOverlayCommands(Drawing::RenderTarget& rt, const Window& window)
    {
        const auto p = window.windowPos + ScreenCoordsXY{ 12, window.height - 23 };
        Fill(rt, window.windowPos + ScreenCoordsXY{ 10, window.height - 29 }, { window.width - 20, 1 }, Theme::edge);
        DrawCommand(rt, p, 17, "A", "Select");
        DrawCommand(rt, p + ScreenCoordsXY{ 92, 0 }, 17, "B", "Back");
    }

    inline void DrawOverlayShell(Drawing::RenderTarget& rt, const Window& window, const MenuPresentation& model)
    {
        DrawPanelHeading(rt, window, model.title);
        const auto p = window.windowPos;
        const int listHeight = std::max(32, window.height - 43 - 58);
        const int offset = ListOffset(model.selected, model.rows.size(), listHeight, kPanelRowPitch);
        const size_t first = offset / kPanelRowPitch;
        Drawing::RenderTarget listRt;
        if (ClipRenderTarget(listRt, rt, p + ScreenCoordsXY{ 0, 43 }, window.width, listHeight))
            for (size_t i = first; i < model.rows.size() && static_cast<int>(i) * kPanelRowPitch - offset < listHeight; ++i)
                DrawActionRow(
                    listRt, { 10, static_cast<int>(i) * kPanelRowPitch - offset }, { window.width - 20, 28 }, model.rows[i], "",
                    i == model.selected, true, i < model.icons.size() ? model.icons[i] : PanelIcon::none,
                    model.destructive && i == 0);
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ 13, window.height - 45 }, window.width - 26, model.status, { Drawing::Colour::white });
        DrawOverlayCommands(rt, window);
    }

    inline void DrawInformationOverlay(Drawing::RenderTarget& rt, const Window& w, const MenuPresentation& model)
    {
        DrawPanelHeading(rt, w, model.title);
        const auto p = w.windowPos;
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ 13, 44 }, w.width - 26, "{YELLOW}" + model.detailTitle, { Drawing::Colour::white });
        for (size_t i = 0; i < model.stats.size() && i < 6; ++i)
        {
            const int y = 69 + static_cast<int>(i) * 22;
            DrawModernEllipsised(rt, p + ScreenCoordsXY{ 13, y }, 154, model.stats[i].first, { Drawing::Colour::white });
            DrawModernEllipsised(
                rt, p + ScreenCoordsXY{ 177, y }, w.width - 190, model.stats[i].second, { Drawing::Colour::white });
        }
        for (size_t i = 0; i < model.rows.size(); ++i)
            DrawIconRow(
                rt, p + ScreenCoordsXY{ 10, w.height - 62 - static_cast<int>(model.rows.size() - i) * 32 }, w.width - 20,
                model.rows[i], i == model.selected, PanelIcon::play);
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ 13, w.height - 45 }, w.width - 26, model.status, { Drawing::Colour::white });
        DrawOverlayCommands(rt, w);
    }

    inline constexpr int kActionStripHeight = Layout::actionHeight;
    inline constexpr ScreenSize kTerrainOverlaySize{ 164, 278 };
    inline constexpr ScreenSize kContextSize{ 160, 42 };

    // Quick landscaping: four shapes named for what you want, the current one highlighted, and the
    // brush drawn as a little tile grid so its size reads at a glance.
    inline void DrawLandscapeOverlay(Drawing::RenderTarget& rt, ScreenCoordsXY pos, int shape, int brush)
    {
        using namespace Concept;
        static constexpr const char* names[]{ "Raise hill", "Dig", "Pond", "Flatten" };
        static constexpr const char* help[]{ "Gentle slopes up", "Gentle slopes down", "Dig and fill with water",
                                             "Level to the middle" };
        const Icon icons[]{ Icon::rise, Icon::lower, Icon::ground, Icon::ground };
        Box(rt, pos, { kRailWidth, 236 });
        Heading(rt, pos + ScreenCoordsXY{ 10, 12 }, "Landscape");
        for (int i = 0; i < 4; ++i)
            Row(rt, pos + ScreenCoordsXY{ 8, 34 + i * 26 }, names[i], icons[i], i == shape);
        DrawModernEllipsised(rt, pos + ScreenCoordsXY{ 12, 140 }, kRailWidth - 20, std::string("{MUTED}") + help[shape & 3]);
        Rule(rt, pos + ScreenCoordsXY{ 8, 158 }, 148);
        Section(rt, pos + ScreenCoordsXY{ 10, 166 }, "Brush");
        const std::string size = std::to_string(brush) + " x " + std::to_string(brush);
        DrawModernText(rt, pos + ScreenCoordsXY{ 12, 186 }, brush == 1 ? "Single tile" : size + " tiles");
        // Tile grid preview, 5 x 5 cells with the brush filled from the centre.
        const ScreenCoordsXY grid = pos + ScreenCoordsXY{ kRailWidth - 52, 168 };
        for (int y = 0; y < 5; ++y)
            for (int x = 0; x < 5; ++x)
            {
                const int r = brush / 2;
                const bool on = std::abs(x - 2) <= r && std::abs(y - 2) <= r;
                Fill(
                    rt, grid + ScreenCoordsXY{ x * 8, y * 8 }, { 7, 7 },
                    on ? Theme::gold : static_cast<Drawing::PaletteIndex>(PocketPalette::disabled));
            }
        DrawCommand(rt, pos + ScreenCoordsXY{ 12, 210 }, 32, "Y", "Shape   L2/R2  Size");
    }

    inline void DrawSurfaceOverlay(Drawing::RenderTarget& rt, ScreenCoordsXY pos, const std::string& name)
    {
        using namespace Concept;
        Box(rt, pos, { kRailWidth, 174 });
        Heading(rt, pos + ScreenCoordsXY{ 10, 12 }, "Land surface");
        Section(rt, pos + ScreenCoordsXY{ 10, 35 }, "Surface");
        Row(rt, pos + ScreenCoordsXY{ 8, 54 }, name, Icon::ground, true);
        Rule(rt, pos + ScreenCoordsXY{ 8, 99 }, 148);
        Section(rt, pos + ScreenCoordsXY{ 10, 109 }, "Brush");
        DrawModernText(rt, pos + ScreenCoordsXY{ 12, 132 }, "Single tile");
        DrawModernText(rt, pos + ScreenCoordsXY{ 12, 153 }, "{MUTED}1 x 1");
    }

    inline void DrawTreeOverlay(Drawing::RenderTarget& rt, ScreenCoordsXY pos, const std::string& name, bool available)
    {
        using namespace Concept;
        Box(rt, pos, { kRailWidth, 146 });
        Heading(rt, pos + ScreenCoordsXY{ 10, 12 }, "Trees");
        Section(rt, pos + ScreenCoordsXY{ 10, 35 }, "Tree");
        Row(rt, pos + ScreenCoordsXY{ 8, 54 }, name, Icon::tree, available);
        Rule(rt, pos + ScreenCoordsXY{ 8, 99 }, 148);
        DrawModernText(rt, pos + ScreenCoordsXY{ 12, 116 }, "{MUTED}Ground level");
    }

    inline void DrawShopOverlay(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, const std::string& name, bool available, Direction direction)
    {
        using namespace Concept;
        Box(rt, pos, { 184, 146 });
        Heading(rt, pos + ScreenCoordsXY{ 10, 12 }, "Place shop");
        DrawWrappedLabel(rt, pos + ScreenCoordsXY{ 12, 42 }, 160, (available ? "{WHITE}" : "{DISABLED}") + name, 2);
        Rule(rt, pos + ScreenCoordsXY{ 12, 83 }, 160);
        DrawModernText(
            rt, pos + ScreenCoordsXY{ 12, 96 },
            "{MUTED}Faces " + std::string(std::array{ "NW", "SW", "SE", "NE" }[direction & 3]));
        DrawCommand(rt, pos + ScreenCoordsXY{ 12, 121 }, 32, "SELECT", "Turn");
    }

    inline void DrawTrackColourOverlay(Drawing::RenderTarget& rt, ScreenCoordsXY pos, uint8_t scheme, bool target)
    {
        using namespace Concept;
        Box(rt, pos, { kRailWidth, 146 });
        Heading(rt, pos + ScreenCoordsXY{ 10, 12 }, "Track colours");
        Section(rt, pos + ScreenCoordsXY{ 10, 35 }, "Scheme");
        Row(rt, pos + ScreenCoordsXY{ 8, 54 }, "Scheme " + std::to_string(scheme + 1), Icon::path, target);
        Rule(rt, pos + ScreenCoordsXY{ 8, 99 }, 148);
        DrawModernText(rt, pos + ScreenCoordsXY{ 12, 116 }, "{MUTED}One track piece");
    }
    inline void DrawSceneryRemovalOverlay(Drawing::RenderTarget& rt, ScreenCoordsXY pos, const std::string& name, bool target)
    {
        using namespace Concept;
        Box(rt, pos, { kRailWidth, 146 });
        Heading(rt, pos + ScreenCoordsXY{ 10, 12 }, "Remove scenery");
        Section(rt, pos + ScreenCoordsXY{ 10, 35 }, "Target");
        Row(rt, pos + ScreenCoordsXY{ 8, 54 }, name, Icon::remove, target);
        Rule(rt, pos + ScreenCoordsXY{ 8, 99 }, 148);
        DrawModernText(rt, pos + ScreenCoordsXY{ 12, 116 }, "{MUTED}One object");
    }

    inline void DrawActionStrip(Drawing::RenderTarget& rt, ScreenSize screen)
    {
        const ScreenCoordsXY p{ 0, screen.height - 28 };
        Fill(rt, p, { screen.width, 28 }, static_cast<Drawing::PaletteIndex>(PocketPalette::panel));
        Concept::Rule(rt, p, screen.width);
    }

    // Full bindings remain available in Park -> Controls. Only declared secondary
    // shortcuts may leave the strip; primary actions never disappear to make room.
    inline std::vector<ControlHint> FooterHints(const std::vector<ControlHint>& hints, int available)
    {
        std::vector<ControlHint> result;
        for (auto h : hints)
        {
            if (h.label.empty() || h.priority == HintPriority::guide)
                continue;
            if (h.label == "Build selection" || h.label == "Build mode")
                h.label = "Build";
            if (h.label == "Select / review")
                h.label = "Select";
            if (h.label == "Add/Remove")
                h.label = "Paint mode";
            const auto duplicate = std::find_if(result.begin(), result.end(), [&](const auto& old) {
                return ControlKey(old.key) == ControlKey(h.key) && old.label == h.label;
            });
            if (duplicate == result.end())
                result.push_back(h);
        }
        const auto width = [&] {
            int total = 0;
            for (const auto& h : result)
                total += ControlHintWidth(h) + 6;
            return total ? total - 6 : 0;
        };
        for (auto it = result.end(); width() > available && it != result.begin();)
        {
            --it;
            if (it->priority == HintPriority::secondary)
                it = result.erase(it);
        }
        return result;
    }
    inline std::vector<ControlPlacement> MeasureFooter(ScreenSize screen, const std::vector<ControlHint>& hints)
    {
        const auto visible = FooterHints(hints, screen.width - 16);
        std::vector<ControlHint> left, actions, back;
        for (const auto& h : visible)
        {
            if (h.key == "D-pad" || ControlKey(h.key) == "L1/R1")
                left.push_back(h);
            else if (h.key == "B")
                back.push_back(h);
            else if (h.key == "A")
                actions.insert(actions.begin(), h);
            else
                actions.push_back(h);
        }
        std::vector<ControlPlacement> result;
        int total = 0;
        for (const auto& h : visible)
            total += ControlHintWidth(h) + 6;
        // Exceptional long labels wrap as complete prompts; text and glyphs never shrink.
        if (total > screen.width - 10)
        {
            std::vector<ControlHint> ordered = left;
            ordered.insert(ordered.end(), actions.begin(), actions.end());
            ordered.insert(ordered.end(), back.begin(), back.end());
            int x = 8, row = 0;
            for (const auto& h : ordered)
            {
                const int w = ControlHintWidth(h);
                if (x > 8 && x + w > screen.width - 8)
                {
                    x = 8;
                    ++row;
                }
                result.push_back({ h, x, row, w });
                x += w + 6;
            }
            const int height = 28 + row * 20;
            for (auto& p : result)
                p.y = screen.height - height + 7 + p.y * 20;
            return result;
        }
        const int y = screen.height - 28 + 7;
        int x = 8;
        for (const auto& h : left)
        {
            const int w = ControlHintWidth(h);
            result.push_back({ h, x, y, w });
            x += w + 6;
        }
        const int backWidth = back.empty() ? 0 : ControlHintWidth(back.front());
        int actionWidth = 0;
        for (const auto& h : actions)
            actionWidth += ControlHintWidth(h) + 6;
        const int maxAction = screen.width - 8 - backWidth - (back.empty() ? 0 : 6) - actionWidth + 6;
        x = std::max(x, std::min(screen.width * 39 / 100, maxAction));
        for (const auto& h : actions)
        {
            const int w = ControlHintWidth(h);
            result.push_back({ h, x, y, w });
            x += w + 6;
        }
        if (!back.empty())
            result.push_back({ back.front(), screen.width - 8 - backWidth, y, backWidth });
        return result;
    }
    inline int FooterHeight(ScreenSize screen, const std::vector<ControlHint>& hints)
    {
        const auto layout = MeasureFooter(screen, hints);
        int height = 28;
        for (const auto& p : layout)
            height = std::max(height, screen.height - p.y + 7);
        return height;
    }
    // When a sheet slides in, its content moves but the controls strip must not: menus point this at
    // the unshifted screen for the duration of a draw (see FooterSurfaceScope).
    inline Drawing::RenderTarget* FooterSurface = nullptr;
    struct FooterSurfaceScope
    {
        Drawing::RenderTarget* previous;
        explicit FooterSurfaceScope(Drawing::RenderTarget& target)
            : previous(FooterSurface)
        {
            FooterSurface = &target;
        }
        ~FooterSurfaceScope()
        {
            FooterSurface = previous;
        }
    };
    inline void DrawFooter(Drawing::RenderTarget& shifted, ScreenSize screen, const std::vector<ControlHint>& hints)
    {
        auto& rt = FooterSurface ? *FooterSurface : shifted;
        LastControlHints = hints;
        const int height = FooterHeight(screen, hints);
        Fill(rt, { 0, screen.height - height }, { screen.width, height }, Theme::ink);
        Concept::Rule(rt, { 0, screen.height - height }, screen.width);
        for (const auto& p : MeasureFooter(screen, hints))
            DrawCommand(rt, { p.x, p.y }, 0, p.hint.key.c_str(), p.hint.label.c_str());
    }
    inline void DrawFooterAt(
        Drawing::RenderTarget& rt, ScreenCoordsXY origin, ScreenSize size, const std::vector<ControlHint>& hints)
    {
        LastControlHints = hints;
        const int height = FooterHeight(size, hints);
        Fill(rt, origin + ScreenCoordsXY{ 0, size.height - height }, { size.width, height }, Theme::ink);
        Concept::Rule(rt, origin + ScreenCoordsXY{ 0, size.height - height }, size.width);
        for (const auto& p : MeasureFooter(size, hints))
            DrawCommand(rt, origin + ScreenCoordsXY{ p.x, p.y }, 0, p.hint.key.c_str(), p.hint.label.c_str());
    }
    inline void DrawModeBadge(Drawing::RenderTarget& rt, ScreenCoordsXY p, const std::string& label)
    {
        const int width = TextWidth(label, true) + 12;
        Fill(rt, p, { width, 18 }, Theme::raised);
        DrawStrongText(rt, p + ScreenCoordsXY{ 6, 5 }, "{WHITE}" + label, width - 12);
    }

    inline void DrawShopFooter(Drawing::RenderTarget& rt, ScreenSize screen)
    {
        DrawFooter(rt, screen, { { "D-pad", "Move" }, { "A", "Place" }, { "Y", "Shops" }, { "SEL", "Turn" }, { "B", "Back" } });
    }

    inline void DrawTreeFooter(Drawing::RenderTarget& rt, ScreenSize screen)
    {
        DrawFooter(rt, screen, { { "D-pad", "Move" }, { "A", "Place" }, { "Y", "Trees" }, { "B", "Back" } });
    }

    inline void DrawSceneryRemovalFooter(Drawing::RenderTarget& rt, ScreenSize screen)
    {
        DrawFooter(rt, screen, { { "D-pad", "Move" }, { "A", "Remove" }, { "B", "Back" } });
    }

    inline void DrawTrackColourFooter(Drawing::RenderTarget& rt, ScreenSize screen)
    {
        DrawFooter(rt, screen, { { "D-pad", "Move" }, { "A", "Paint" }, { "Y", "Scheme" }, { "B", "Back" } });
    }
    inline void DrawGameplayFooter(
        Drawing::RenderTarget& rt, ScreenSize screen, const std::string&, bool terrain, bool surface = false,
        const std::string& inspect = "Inspect", const std::string& secondary = "News")
    {
        DrawActionStrip(rt, screen);

        if (terrain)
        {
            DrawFooter(
                rt, screen,
                { { "D-pad", "Move" },
                  { "A", surface ? "Paint" : "Edit" },
                  { "Y", surface ? "Surface" : "Tool" },
                  { "B", "Back" } });
        }
        else
        {
            DrawFooter(
                rt, screen,
                { { "D-pad", "Move" },
                  { "A", inspect },
                  { "Y", secondary },
                  { "B", "Back" },
                  { "SEL", "Build" },
                  { "START", "Park" },
                  { "L1/R1", "Zoom", HintPriority::secondary },
                  { "X", "Rotate", HintPriority::guide } });
        }
    }

    inline void DrawPathOverlay(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, const std::string& surface, bool queue, bool remove,
        const std::string& placement = "Ground / auto slope", bool connected = false, bool adjusting = false)
    {
        DrawSurface(rt, pos, { 164, adjusting ? connected ? 174 : 194 : 146 });
        DrawIcon(rt, pos + ScreenCoordsXY{ 10, 12 }, PanelIcon::path);
        DrawStrongText(
            rt, pos + ScreenCoordsXY{ 36, 14 },
            adjusting       ? "{WHITE}Adjust path"
                : connected ? "{WHITE}Extend path"
                            : "{WHITE}Single path",
            116);
        Fill(rt, pos + ScreenCoordsXY{ 8, 35 }, { 148, 22 }, Theme::raised);
        DrawStrongText(
            rt, pos + ScreenCoordsXY{ 14, 42 },
            remove      ? "{WHITE}REMOVE"
                : queue ? "{WHITE}QUEUE"
                        : "{WHITE}FOOTPATH",
            136);
        DrawModernEllipsised(rt, pos + ScreenCoordsXY{ 12, 70 }, 140, "{WHITE}" + surface);
        const auto slash = placement.find(" / ");
        DrawModernEllipsised(
            rt, pos + ScreenCoordsXY{ 12, 91 }, 140,
            "{MUTED}" + (remove ? std::string("Existing tile height") : placement.substr(0, slash)));
        if (!remove && slash != std::string::npos)
            DrawModernEllipsised(rt, pos + ScreenCoordsXY{ 12, 109 }, 140, "{MUTED}" + placement.substr(slash + 3));
        if (adjusting)
        {
            DrawModernText(rt, pos + ScreenCoordsXY{ 12, 128 }, "{WHITE}Left/Right  Turn");
            DrawModernText(rt, pos + ScreenCoordsXY{ 12, 148 }, connected ? "{WHITE}Up/Down  Slope" : "{WHITE}Up/Down  Height");
        }
        else
            DrawCommand(rt, pos + ScreenCoordsXY{ 12, 128 }, 32, "L2/R2", connected ? "Slope" : "Height");
    }

    inline void DrawPathFooter(
        Drawing::RenderTarget& rt, ScreenSize screen, const std::string&, bool remove = false, bool connected = false,
        bool adjusting = false, bool ready = false)
    {
        DrawActionStrip(rt, screen);

        if (adjusting)
        {
            DrawFooter(
                rt, screen,
                { { "D-pad", "Adjust" }, { "A", connected ? "Flat" : "Flat/slope" }, { "X", "More" }, { "B", "Build" } });
        }
        else
        {
            DrawFooter(
                rt, screen,
                { { "D-pad", "Move" },
                  { "A",
                    connected    ? ready ? "Extend" : "Start"
                        : remove ? "Remove"
                                 : "Place" },
                  { "X",
                    connected    ? "View"
                        : remove ? "Place"
                                 : "Remove" },
                  { "Y", "Adjust" },
                  { "SEL", "Surface", HintPriority::guide },
                  { "L1/R1", "Zoom", HintPriority::secondary },
                  { "B", "Back" } });
        }
    }

    // Cost and status share the name-chip style: a card sized to its text with a colour tab.
    inline void DrawCostBadge(Drawing::RenderTarget& rt, ScreenCoordsXY pos, const std::string& amount, bool quote = false)
    {
        const std::string label = quote ? "Quote" : "Cost";
        const int labelWidth = TextWidth(label);
        const int width = std::min(kContextSize.width, labelWidth + TextWidth(amount, true) + 30);
        DrawCard(rt, pos, { width, 22 });
        Fill(rt, pos + ScreenCoordsXY{ 4, 4 }, { 4, 14 }, static_cast<Drawing::PaletteIndex>(PocketPalette::accent));
        DrawModernText(rt, pos + ScreenCoordsXY{ 13, 6 }, "{MUTED}" + label);
        DrawStrongText(rt, pos + ScreenCoordsXY{ 19 + labelWidth, 6 }, "{WHITE}" + amount, width - labelWidth - 24);
    }

    inline void DrawContextMessage(Drawing::RenderTarget& rt, ScreenCoordsXY pos, const std::string& text)
    {
        // One line when it fits, otherwise two; never a permanent status row.
        constexpr int kLine = 140;
        auto split = text.size();
        while (split > 0 && TextWidth(text.substr(0, split)) > kLine)
            --split;
        if (split < text.size())
        {
            const auto space = text.rfind(' ', split);
            if (space != std::string::npos && space > 0)
                split = space;
        }
        const auto first = text.substr(0, split);
        auto second = split < text.size() ? text.substr(split) : std::string();
        while (!second.empty() && second.front() == ' ')
            second.erase(0, 1);
        const int width = std::min(kContextSize.width, std::max(TextWidth(first), TextWidth(second)) + 24);
        const int height = second.empty() ? 22 : 38;
        DrawCard(rt, pos, { width, height });
        Fill(rt, pos + ScreenCoordsXY{ 4, 4 }, { 4, height - 8 }, static_cast<Drawing::PaletteIndex>(PocketPalette::selection));
        DrawModernEllipsised(rt, pos + ScreenCoordsXY{ 13, 6 }, width - 18, "{WHITE}" + first);
        if (!second.empty())
            DrawModernEllipsised(rt, pos + ScreenCoordsXY{ 13, 22 }, width - 18, "{MUTED}" + second);
    }

    // Bounded, content-sized feedback for a selection-wide native rejection or result.
    inline void DrawConstructionMessage(Drawing::RenderTarget& rt, ScreenSize screen, const std::string& text)
    {
        constexpr int width = 216;
        std::vector<std::string> lines;
        std::istringstream words(text);
        std::string line, word;
        while (words >> word)
        {
            const auto candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && TextWidth(candidate) > width - 20 && lines.size() < 3)
            {
                lines.push_back(line);
                line = word;
            }
            else
                line = candidate;
        }
        if (!line.empty())
            lines.push_back(line);
        const ScreenSize size{ width, 16 + static_cast<int>(lines.size()) * 17 };
        const auto pos = Layout::ContextOrigin(screen, { screen.width - Layout::margin - width, screen.height }, size);
        DrawSurface(rt, pos, size);
        for (size_t i = 0; i < lines.size(); ++i)
            DrawModernEllipsised(
                rt, pos + ScreenCoordsXY{ 10, 8 + static_cast<int>(i) * 17 }, width - 20,
                (i == 0 ? "{WHITE}" : "{MUTED}") + lines[i]);
    }

    inline void DrawPathOptions(
        Drawing::RenderTarget& rt, ScreenSize screen, const std::vector<std::string>& rows, size_t selected,
        const std::string& status)
    {
        const auto p = Layout::menuOrigin;
        constexpr int width = 240;
        DrawSurface(rt, p, { width, Layout::ContentHeight(screen) });
        DrawSectionLabel(rt, p + ScreenCoordsXY{ 12, 13 }, width - 24, "BUILD / PATHS");
        DrawHeadingText(rt, p + ScreenCoordsXY{ 12, 28 }, "{WHITE}Path settings", width - 24);
        DrawModernEllipsised(rt, p + ScreenCoordsXY{ 12, 54 }, width - 24, "{MUTED}" + status);
        const bool connected = rows.size() > 7;
        const int positions[] = { 76, 127, 155, 183, 211, 254, 282, 310 };
        DrawSectionLabel(rt, p + ScreenCoordsXY{ 12, 111 }, width - 24, connected ? "DIRECTION & SLOPE" : "HEIGHT & SLOPE");
        DrawSectionLabel(
            rt, p + ScreenCoordsXY{ 12, 238 }, width - 24, connected ? "SURFACE & RECOVERY" : "ORIENTATION & SURFACE");
        for (size_t i = 0; i < rows.size() && i < 8; ++i)
            DrawActionRow(
                rt, p + ScreenCoordsXY{ 8, positions[i] }, { width - 16, i == 0 ? 30 : 27 },
                i == 0 ? "Return to building" : rows[i], "", selected == i);
        const std::array<const char*, 8> connectedHelp{
            "Continue from the current path end.", "Turn the next section before building.",
            "Build uphill from the current end.",  "Build downhill from the current end.",
            "Make the next section level.",        "Pick another path end. Existing paths stay.",
            "Choose the path or queue surface.",   "Review removing the last new section."
        };
        const std::array<const char*, 7> groundHelp{ "Return to the park with this preview.",
                                                     "Each tile follows its own terrain. Works with Single, Line and Fill.",
                                                     "Use a fixed elevation above the current preview.",
                                                     "Lower the fixed elevation. Follow ground restores automatic height.",
                                                     "Change the slope for a single section. Line and Fill use level tiles.",
                                                     "Turn the uphill end of a single sloping section.",
                                                     "Choose the path or queue surface." };
        DrawSurface(rt, { screen.width - 236, screen.height - 132 }, { 228, 88 });
        DrawWrappedLabel(
            rt, { screen.width - 224, screen.height - 120 }, 204,
            connected ? connectedHelp[std::min(selected, connectedHelp.size() - 1)]
                      : groundHelp[std::min(selected, groundHelp.size() - 1)],
            4);
        DrawFooter(rt, screen, { { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Build" } });
    }

    inline void DrawMapFocus(Drawing::RenderTarget& rt, ScreenCoordsXY centre)
    {
        GfxDrawLine(rt, { centre + ScreenCoordsXY{ -7, -1 }, centre + ScreenCoordsXY{ 7, -1 } }, Drawing::PaletteIndex::pi10);
        GfxDrawLine(rt, { centre + ScreenCoordsXY{ -7, 1 }, centre + ScreenCoordsXY{ 7, 1 } }, Drawing::PaletteIndex::pi10);
        GfxDrawLine(rt, { centre + ScreenCoordsXY{ -1, -7 }, centre + ScreenCoordsXY{ -1, 7 } }, Drawing::PaletteIndex::pi10);
        GfxDrawLine(rt, { centre + ScreenCoordsXY{ 1, -7 }, centre + ScreenCoordsXY{ 1, 7 } }, Drawing::PaletteIndex::pi10);
        GfxDrawLine(rt, { centre + ScreenCoordsXY{ -6, 0 }, centre + ScreenCoordsXY{ 6, 0 } }, Drawing::PaletteIndex::pi21);
        GfxDrawLine(rt, { centre + ScreenCoordsXY{ 0, -6 }, centre + ScreenCoordsXY{ 0, 6 } }, Drawing::PaletteIndex::pi21);
    }

    // Viewfinder corners in the target's colour with a dark outline, breathing by a few pixels,
    // and a centre dot so the exact hit point stays visible.
    inline void DrawTargetBrackets(Drawing::RenderTarget& rt, ScreenCoordsXY centre, Drawing::PaletteIndex colour, int pulse)
    {
        const auto outline = NearestPaletteIndex(28, 28, 36);
        const int half = 11 + pulse;
        constexpr int arm = 6;
        for (int sx : { -1, 1 })
            for (int sy : { -1, 1 })
            {
                const ScreenCoordsXY corner = centre + ScreenCoordsXY{ sx * half, sy * half };
                const int hx = sx < 0 ? corner.x : corner.x - arm + 1;
                const int vy = sy < 0 ? corner.y : corner.y - arm + 1;
                const int cx = sx < 0 ? corner.x : corner.x - 1;
                const int cy = sy < 0 ? corner.y : corner.y - 1;
                Fill(rt, { hx - 1, cy - 1 }, { arm + 2, 4 }, outline);
                Fill(rt, { cx - 1, vy - 1 }, { 4, arm + 2 }, outline);
                Fill(rt, { hx, cy }, { arm, 2 }, colour);
                Fill(rt, { cx, vy }, { 2, arm }, colour);
            }
        Fill(rt, centre - ScreenCoordsXY{ 2, 2 }, { 4, 4 }, outline);
        Fill(rt, centre - ScreenCoordsXY{ 1, 1 }, { 2, 2 }, colour);
    }

    // Compact label beside the cursor naming what A would inspect, with a colour tab matching
    // the cursor brackets.
    inline void DrawInspectChip(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, ScreenSize size, const std::string& name, Drawing::PaletteIndex colour)
    {
        DrawCard(rt, pos, size);
        Fill(rt, pos + ScreenCoordsXY{ 4, 4 }, { 4, size.height - 8 }, colour);
        DrawStrongText(rt, pos + ScreenCoordsXY{ 13, 5 }, "{WHITE}" + name, size.width - 18);
    }

    // One layout for pause, tool selection, save/load and confirmation pages.
    // The renderer is also called directly by the preview tool: no separate mockup UI.
    inline void DrawMenuShell(Drawing::RenderTarget& rt, const Window& window, const MenuPresentation& model)
    {
        DrawPanelHeading(rt, window, model.title);
        const auto p = window.windowPos;
        if (model.tabs.empty())
            DrawModernEllipsised(rt, p + ScreenCoordsXY{ 14, 46 }, 204, "{YELLOW}" + model.section, { Drawing::Colour::white });
        else
            DrawTabs(rt, p + ScreenCoordsXY{ 10, 44 }, 221, model.tabs, model.selectedTab);
        const size_t first = model.selected >= 8 ? model.selected - 7 : 0;
        for (size_t i = first; i < model.rows.size() && i < first + 8; ++i)
            DrawIconRow(
                rt, p + ScreenCoordsXY{ 10, (model.tabs.empty() ? 64 : 82) + static_cast<int32_t>(i) * kPanelRowPitch }, 218,
                model.rows[i], i == model.selected, i < model.icons.size() ? model.icons[i] : PanelIcon::none);
        DrawCard(rt, p + ScreenCoordsXY{ 238, 44 }, { window.width - 248, 150 });
        GfxDrawSprite(rt, ImageId(SPR_G2_TAB_TREE), p + ScreenCoordsXY{ 250, 50 });
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ 289, 59 }, window.width - 305, model.detailTitle, { Drawing::Colour::white });
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ 250, 84 }, window.width - 272, model.detailSubtitle, { Drawing::Colour::white });
        Fill(rt, p + ScreenCoordsXY{ 250, 105 }, { window.width - 272, 1 }, Theme::edge);
        for (size_t i = 0; i < model.stats.size() && i < 3; ++i)
        {
            const auto y = 116 + static_cast<int32_t>(i) * 23;
            DrawModernEllipsised(rt, p + ScreenCoordsXY{ 250, y }, 100, model.stats[i].first, { Drawing::Colour::white });
            DrawModernEllipsised(
                rt, p + ScreenCoordsXY{ 357, y }, window.width - 371, "{YELLOW}" + model.stats[i].second,
                { Drawing::Colour::white });
        }
        DrawCard(rt, p + ScreenCoordsXY{ 238, 202 }, { window.width - 248, 87 });
        DrawModernText(rt, p + ScreenCoordsXY{ 250, 214 }, "{YELLOW}CONTROLS", { Drawing::Colour::white });
        for (size_t i = 0; i < model.help.size() && i < 3; ++i)
            DrawModernEllipsised(
                rt, p + ScreenCoordsXY{ 250, 237 + static_cast<int32_t>(i) * 16 }, window.width - 272, model.help[i],
                { Drawing::Colour::white });
        DrawModernEllipsised(
            rt, p + ScreenCoordsXY{ 14, window.height - 47 }, window.width - 28, model.status, { Drawing::Colour::white });
        DrawPanelCommands(rt, window);
    }
} // namespace OpenRCT2::Ui::Handheld
