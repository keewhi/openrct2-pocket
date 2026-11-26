// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "ConceptStyle.h"
#include "MapSubjects.h"

#include <openrct2-ui/windows/NativeMap.h>
#include <openrct2/Context.h>
#include <openrct2/GameState.h>
#include <openrct2/interface/Viewport.h>
#include <openrct2/world/Map.h>
#include <openrct2/world/tile_element/SurfaceElement.h>
#include <vector>

namespace OpenRCT2::Ui::Handheld::ParkMap
{
    // Transient view state. All map colours and rotation geometry come from desktop Map.
    struct State
    {
        CoordsXY focus{};
        TileCoordsXY size{};
        uint8_t rotation = 255;
        bool rides{};
        bool details{};
        std::optional<MapSubjects::Target> subject;
        int mode{}, filter{};
        std::optional<GuestManagement::Filter> guestFilter;
        int zoom = 1;
        int extent{}, next{};
        std::vector<Drawing::PaletteIndex> pixels;
        bool ready{};
        void Clamp()
        {
            const auto bounds = getGameState().mapSize;
            focus.x = std::clamp(focus.x / 32, 1, std::max(1, bounds.x - 2)) * 32;
            focus.y = std::clamp(focus.y / 32, 1, std::max(1, bounds.y - 2)) * 32;
        }
        void Begin()
        {
            guestFilter.reset();
            const auto bounds = getGameState().mapSize;
            focus = CoordsXY{ bounds.x / 2 * 32, bounds.y / 2 * 32 };
            if (auto* main = WindowGetMain(); main && main->viewport)
            {
                Viewport* viewport = nullptr;
                auto point = ScreenGetMapXY(main->windowPos + ScreenCoordsXY{ main->width / 2, main->height / 2 }, &viewport);
                if (point && viewport == main->viewport)
                    focus = *point;
            }
            rotation = 255;
            zoom = 1;
            Clamp();
            Update();
        }
        void Move(int x, int y)
        {
            if (x || y)
                subject.reset();
            focus += CoordsXY{ y * 32, x * 32 }.rotate(-GetCurrentRotation());
            Clamp();
        }
        void Mode(int direction = 1)
        {
            mode = (mode + direction + 4) % 4;
            rides = mode == 1;
            filter = 0;
            guestFilter.reset();
            rotation = 255;
        }
        void Update()
        {
            Clamp();
            const auto bounds = getGameState().mapSize;
            const auto current = GetCurrentRotation();
            if (size != bounds || rotation != current)
            {
                size = bounds;
                rotation = current;
                extent = std::max(size.x, size.y) * 2 + 4;
                pixels.assign(extent * extent, Drawing::PaletteIndex::pi13);
                next = 0;
                ready = false;
            }
            const int count = std::max(0, (size.x - 2) * (size.y - 2));
            for (int n = 0; n < std::min(count, 2048); ++n)
            {
                const CoordsXY c{ (next % (size.x - 2) + 1) * 32, (next / (size.x - 2) + 1) * 32 };
                const auto point = Windows::NativeMiniMapCoords(c);
                const auto colours = Windows::NativeMiniMapTileColour(c, rides);
                if (point.x > 0 && point.y >= 0 && point.x < extent && point.y < extent)
                {
                    pixels[point.y * extent + point.x - 1] = colours[0];
                    pixels[point.y * extent + point.x] = colours[1];
                }
                if (++next == count)
                {
                    next = 0;
                    ready = true;
                }
            }
        }
    };
    inline void Draw(Drawing::RenderTarget& rt, const State& state)
    {
        // Laid out like the other screens: heading, then the view tabs (L1/R1), then the map on the
        // left with a card on the right for this tile, the legend, the subject and details. Those
        // used to be boxes drawn over the map itself.
        DrawSurface(rt, { 8, 8 }, { 496, 348 });
        DrawHeadingText(rt, { 20, 19 }, "{WHITE}Park map", 300);
        DrawTabs(rt, { 18, 46 }, 476, { "Terrain", "Rides", "Guests", "Staff" }, state.mode);
        const ScreenCoordsXY origin{ 20, 80 };
        constexpr int width = 318, height = 266;
        FillRound(rt, origin - ScreenCoordsXY{ 2, 2 }, { width + 4, height + 4 }, Theme::edge);
        const auto focus = Windows::NativeMiniMapCoords(state.focus);
        // Fit the native raster at overview; zoom crops around the selected native tile.
        const int denominator = std::max(1, state.extent);
        const int scale = std::min(width, height) * state.zoom;
        const auto project = [&](TileCoordsXY p) {
            return ScreenCoordsXY{
                origin.x + width / 2 + (p.x - (state.zoom == 1 ? denominator / 2 : focus.x)) * scale / denominator,
                origin.y + height / 2 + (p.y - (state.zoom == 1 ? denominator / 2 : focus.y)) * scale / denominator
            };
        };
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                const int px = origin.x + x, py = origin.y + y;
                const int mx = (x - width / 2) * denominator / std::max(1, scale)
                    + (state.zoom == 1 ? denominator / 2 : focus.x);
                const int my = (y - height / 2) * denominator / std::max(1, scale)
                    + (state.zoom == 1 ? denominator / 2 : focus.y);
                if (px >= rt.x && px < rt.x + rt.width && py >= rt.y && py < rt.y + rt.height && mx >= 0 && my >= 0
                    && mx < state.extent && my < state.extent && !state.pixels.empty())
                    rt.bits[(py - rt.y) * rt.LineStride() + px - rt.x] = state.pixels[my * state.extent + mx];
            }
        if (const auto bounds = Windows::NativeMiniMapViewportBounds())
        {
            auto a = project(TileCoordsXY{ (*bounds)[0].x, (*bounds)[0].y });
            auto b = project(TileCoordsXY{ (*bounds)[1].x, (*bounds)[1].y });
            if (a.x < origin.x + width && b.x >= origin.x && a.y < origin.y + height && b.y >= origin.y)
            {
                a.x = std::clamp(a.x, origin.x, origin.x + width - 1);
                a.y = std::clamp(a.y, origin.y, origin.y + height - 1);
                b.x = std::clamp(b.x, origin.x, origin.x + width - 1);
                b.y = std::clamp(b.y, origin.y, origin.y + height - 1);
                const auto colour = static_cast<Drawing::PaletteIndex>(PocketPalette::primary);
                GfxDrawLine(rt, { a, { b.x, a.y } }, colour);
                GfxDrawLine(rt, { { b.x, a.y }, b }, colour);
                GfxDrawLine(rt, { b, { a.x, b.y } }, colour);
                GfxDrawLine(rt, { { a.x, b.y }, a }, colour);
            }
        }
        std::vector<uint16_t> guestMembers;
        if (state.mode == 2 && state.guestFilter)
        {
            for (const auto& member : MapSubjects::Roster(2, state.filter, state.guestFilter))
                guestMembers.push_back(std::get<GuestManagement::Target>(member).id.ToUnderlying());
            std::sort(guestMembers.begin(), guestMembers.end());
        }
        if (state.mode >= 2)
            for (const auto& peep : Windows::NativeMiniMapPeepMarkers(
                     state.mode == 3, state.mode == 2 && state.filter != 0, state.mode == 3 ? state.filter - 1 : -1))
            {
                if (state.mode == 2 && state.guestFilter
                    && !std::binary_search(guestMembers.begin(), guestMembers.end(), peep.id.ToUnderlying()))
                    continue;
                const auto point = project(peep.position);
                if (point.x >= origin.x && point.x < origin.x + width - 1 && point.y >= origin.y
                    && point.y < origin.y + height - 1)
                    for (int y = 0; y < 2; ++y)
                        for (int x = 0; x < 2; ++x)
                            if (point.x + x >= rt.x && point.x + x < rt.x + rt.width && point.y + y >= rt.y
                                && point.y + y < rt.y + rt.height)
                                rt.bits[(point.y + y - rt.y) * rt.LineStride() + point.x + x - rt.x] = peep.colour;
            }
        const auto marker = project(focus);
        const auto gold = static_cast<Drawing::PaletteIndex>(PocketPalette::selection);
        GfxDrawLine(rt, { marker - ScreenCoordsXY{ 5, 0 }, marker + ScreenCoordsXY{ 5, 0 } }, gold);
        GfxDrawLine(rt, { marker - ScreenCoordsXY{ 0, 5 }, marker + ScreenCoordsXY{ 0, 5 } }, gold);
        const auto* surface = MapGetSurfaceElementAt(state.focus);
        const auto ownership = !surface                                       ? "No surface"
            : surface->hasOwnership(OwnershipFlag::landOwned)                 ? "Land owned"
            : surface->hasOwnership(OwnershipFlag::constructionRightsOwned)   ? "Rights owned"
            : surface->hasOwnership(OwnershipFlag::landForSale)               ? "Land for sale"
            : surface->hasOwnership(OwnershipFlag::constructionRightsForSale) ? "Rights for sale"
                                                                              : "Land unavailable";
        const ScreenCoordsXY card{ origin.x + width + 10, origin.y - 2 };
        const int cardW = 494 - card.x, cardH = height + 4;
        FillRound(rt, card, { cardW, cardH }, Theme::raised);
        int y = card.y + 8;
        DrawSectionLabel(rt, card + ScreenCoordsXY{ 10, y - card.y }, cardW - 20, "THIS TILE");
        y += 14;
        DrawStrongText(rt, { card.x + 10, y }, "{WHITE}" + std::string(ownership), cardW - 20);
        y += 20;
        DrawSectionLabel(rt, { card.x + 10, y }, cardW - 20, "KEY");
        y += 15;
        const std::array<std::pair<int, std::string>, 3> legend = state.rides
            ? std::array<std::pair<int, std::string>, 3>{ { { 61, "Rides" }, { 14, "Paths" }, { 195, "Water" } } }
            : std::array<std::pair<int, std::string>, 3>{ { { state.mode == 3       ? 138
                                                                  : state.mode == 2 ? 172
                                                                                    : 98,
                                                              state.mode == 3       ? "Staff"
                                                                  : state.mode == 2 ? "Guests"
                                                                                    : "Land" },
                                                            { 14, "Paths" },
                                                            { 195, "Water" } } };
        for (const auto& [colour, label] : legend)
        {
            FillRound(rt, { card.x + 10, y + 1 }, { 9, 9 }, static_cast<Drawing::PaletteIndex>(colour));
            DrawModernText(rt, { card.x + 26, y }, "{WHITE}" + label, {}, cardW - 36);
            y += 15;
        }
        y += 6;
        if (state.subject)
        {
            DrawSectionLabel(rt, { card.x + 10, y }, cardW - 20, "SHOWING");
            DrawWrappedLabel(rt, { card.x + 10, y + 14 }, cardW - 20, "{WHITE}" + MapSubjects::Name(*state.subject), 2);
            y += 48;
        }
        if (state.details)
        {
            DrawSectionLabel(rt, { card.x + 10, y }, cardW - 20, "DETAILS");
            y += 14;
            for (const auto& line : { "Tile " + std::to_string(state.focus.x / 32) + ", " + std::to_string(state.focus.y / 32),
                                      "Map " + std::to_string(state.size.x) + " x " + std::to_string(state.size.y),
                                      "Zoom " + std::to_string(state.zoom) + "x",
                                      MapSubjects::FilterName(state.mode, state.filter, state.guestFilter) })
            {
                DrawModernEllipsised(rt, { card.x + 10, y }, cardW - 20, "{MUTED}" + line);
                y += 14;
            }
        }
        DrawFooter(
            rt, { ContextGetWidth(), ContextGetHeight() },
            { { "D-pad", "Move" },
              { "A", "Locate" },
              { "Y", "Zoom" },
              { "SEL", "Subjects" },
              { "R2", "Details" },
              { "B", "Back" },
              { "X", "Rotate", HintPriority::guide },
              { "L1/R1", "View", HintPriority::guide } });
    }
} // namespace OpenRCT2::Ui::Handheld::ParkMap
