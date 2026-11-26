// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "Panel.h"

#include <algorithm>
#include <functional>
#include <vector>

namespace OpenRCT2::Ui::Handheld
{
    struct ChoiceCard
    {
        size_t action; // The existing menu action, not the visual position.
        std::string label, detail;
        PanelIcon icon = PanelIcon::none;
        uint32_t sprite = 0;
        size_t group = 0;                                      // Grouped menus: which row of the menu this card sits in.
        uint8_t frames = 1;                                    // Animated tab sprites play while focused.
        std::function<void(Drawing::RenderTarget&)> picture{}; // Big cards: a real object preview.
    };
    // A grouped menu (Build): each group is one row with its own card size, under a small label. The
    // first group is the big, everyday one; later groups get smaller.
    struct ChoiceGroup
    {
        std::string title;
        int height;
    };
    inline std::vector<std::vector<size_t>> GroupSlots(const std::vector<ChoiceCard>& cards, size_t groups)
    {
        std::vector<std::vector<size_t>> slots(groups);
        for (size_t i = 0; i < cards.size(); ++i)
            if (cards[i].group < groups)
                slots[cards[i].group].push_back(i);
        return slots;
    }
    inline size_t MoveChoiceGroups(const std::vector<ChoiceCard>& cards, size_t groups, size_t action, int x, int y)
    {
        const auto slots = GroupSlots(cards, groups);
        size_t g = 0, i = 0;
        for (size_t gi = 0; gi < slots.size(); ++gi)
            for (size_t k = 0; k < slots[gi].size(); ++k)
                if (cards[slots[gi][k]].action == action)
                {
                    g = gi;
                    i = k;
                }
        if (x)
        {
            const int next = static_cast<int>(i) + (x > 0 ? 1 : -1);
            if (next >= 0 && next < static_cast<int>(slots[g].size()))
                i = static_cast<size_t>(next);
        }
        else if (y)
        {
            int target = static_cast<int>(g) + (y > 0 ? 1 : -1);
            while (target >= 0 && target < static_cast<int>(slots.size()) && slots[target].empty())
                target += y > 0 ? 1 : -1;
            if (target >= 0 && target < static_cast<int>(slots.size()))
            {
                // Keep the same horizontal position: map the card's centre onto the other row.
                const double centre = (i + 0.5) / static_cast<double>(slots[g].size());
                i = std::min(slots[target].size() - 1, static_cast<size_t>(centre * slots[target].size()));
                g = static_cast<size_t>(target);
            }
        }
        return slots[g].empty() ? action : cards[slots[g][i]].action;
    }
    inline size_t MoveChoiceCards(const std::vector<ChoiceCard>& cards, size_t action, size_t columns, int x, int y)
    {
        if (cards.empty() || columns == 0)
            return action;
        const auto found = std::find_if(cards.begin(), cards.end(), [&](const auto& c) { return c.action == action; });
        size_t slot = found == cards.end() ? 0 : static_cast<size_t>(found - cards.begin());
        if (y)
        {
            const auto next = static_cast<int64_t>(slot) + (y > 0 ? 1 : -1) * static_cast<int64_t>(columns);
            if (next >= 0 && next < static_cast<int64_t>(cards.size()))
                slot = static_cast<size_t>(next);
            else if (y > 0 && (slot / columns + 1) * columns < cards.size())
                slot = cards.size() - 1;
        }
        else if (x > 0 && slot % columns + 1 < columns && slot + 1 < cards.size())
            ++slot;
        else if (x < 0 && slot % columns > 0)
            --slot;
        return cards[slot].action;
    }
    inline void DrawChoiceCard(
        Drawing::RenderTarget& rt, ScreenCoordsXY cell, ScreenSize size, const ChoiceCard& card, bool focused)
    {
        const int press = ButtonPressOffset(focused);
        cell.y += press;
        size.height -= press;
        // Labels are centred under the art on every card, splitting onto two centred lines when
        // they do not fit (a left-aligned label under a centred icon looks off-centre).
        auto label = [&](ScreenCoordsXY origin, int width, int bottom) {
            std::string first = card.label, second;
            if (TextWidth(first, true) > width)
                for (auto cut = card.label.rfind(' '); cut != std::string::npos && cut > 0;
                     cut = card.label.rfind(' ', cut - 1))
                    if (TextWidth(card.label.substr(0, cut), true) <= width)
                    {
                        first = card.label.substr(0, cut);
                        second = card.label.substr(cut + 1);
                        break;
                    }
            const int lines = second.empty() ? 1 : 2;
            int y = bottom - lines * 13;
            for (const auto& line : { first, second })
            {
                if (line.empty())
                    continue;
                const int w = std::min(TextWidth(line, true), width);
                DrawStrongText(rt, origin + ScreenCoordsXY{ (width - w) / 2, y }, "{WHITE}" + line, width);
                y += 13;
            }
            return lines;
        };
        DrawButtonFace(rt, cell, size, ButtonKind::secondary, focused);
        if (size.height >= 80)
        {
            // Big card: the object (or its icon) fills the top, the name sits centred underneath.
            const bool twoLines = TextWidth(card.label, true) > size.width - 8;
            const int artHeight = size.height - (twoLines ? 38 : 26);
            Drawing::RenderTarget art;
            if (card.picture)
            {
                if (ClipRenderTarget(art, rt, cell + ScreenCoordsXY{ 3, 3 }, size.width - 6, artHeight))
                    card.picture(art);
            }
            else if (
                !card.sprite
                || !DrawNativeMenuArt(
                    rt, cell + ScreenCoordsXY{ 3, 3 }, { size.width - 6, artHeight }, { card.sprite, card.frames }, focused))
                DrawIcon(rt, cell + ScreenCoordsXY{ size.width / 2 - 8, 3 + artHeight / 2 - 8 }, card.icon, focused);
            label(cell + ScreenCoordsXY{ 4, 0 }, size.width - 8, size.height - 6);
            return;
        }
        // Reserve two complete label lines, including glyph descenders.
        const bool twoLines = TextWidth(card.label, true) > size.width - 14;
        const int artSize = std::clamp(size.height - (twoLines ? 38 : 34), 20, 38);
        Drawing::RenderTarget inside;
        if (!ClipRenderTarget(inside, rt, cell + ScreenCoordsXY{ 3, 3 }, size.width - 6, size.height - 6))
            return;
        const auto icon = ScreenCoordsXY{ (size.width - 6 - artSize) / 2, 1 };
        if (card.sprite)
            DrawNativeMenuArt(inside, icon, { artSize, artSize }, { card.sprite, card.frames }, focused);
        else
            DrawIcon(inside, icon + ScreenCoordsXY{ artSize / 2 - 8, artSize / 2 - 8 }, card.icon, focused);
        label(cell + ScreenCoordsXY{ 7, 0 }, size.width - 14, size.height - 5);
    }
    // Small groups draw as compact chips: icon and label side by side.
    inline void DrawChoiceChip(
        Drawing::RenderTarget& rt, ScreenCoordsXY cell, ScreenSize size, const ChoiceCard& card, bool focused)
    {
        DrawButtonFace(rt, cell, size, ButtonKind::secondary, focused);
        const bool art = card.sprite
            && DrawNativeMenuArt(
                             rt, cell + ScreenCoordsXY{ 4, (size.height - 24) / 2 }, { 24, 24 }, { card.sprite, card.frames },
                             false);
        if (!art)
            DrawIcon(rt, cell + ScreenCoordsXY{ 8, (size.height - 16) / 2 }, card.icon, false);
        DrawStrongText(rt, cell + ScreenCoordsXY{ 32, CapCentre(size.height - 2) }, "{WHITE}" + card.label, size.width - 38);
    }
    inline void DrawChoiceGroups(
        Drawing::RenderTarget& rt, ScreenSize screen, const std::string& title, const std::vector<ChoiceCard>& cards,
        const std::vector<ChoiceGroup>& groups, size_t selected, const std::string& back)
    {
        const ScreenCoordsXY p{ 8, 8 };
        const int width = screen.width - 16;
        const int height = Layout::ContentHeight(screen);
        DrawSurface(rt, p, { width, height });
        DrawHeadingText(rt, p + ScreenCoordsXY{ 12, 11 }, "{WHITE}" + title, width - 24);
        Fill(rt, p + ScreenCoordsXY{ 12, 37 }, { width - 24, 1 }, Theme::edge);
        const auto slots = GroupSlots(cards, groups.size());
        // The rows are sized for a 4:3 screen. On a wider, shorter one (16:9 Android handhelds)
        // they would push the description under the footer, so the big first row gives up the
        // difference, down to the smallest size that still draws as a card.
        int needed = 44 + 13 + 4;
        for (size_t g = 0; g < groups.size(); ++g)
            if (!slots[g].empty())
                needed += 14 + groups[g].height + 8;
        const int squeeze = std::max(0, needed - height);
        int y = 44;
        for (size_t g = 0; g < groups.size(); ++g)
        {
            if (slots[g].empty())
                continue;
            int rowHeight = groups[g].height;
            if (g == 0 && rowHeight >= 40)
                rowHeight = std::max(48, rowHeight - squeeze);
            DrawSectionLabel(rt, p + ScreenCoordsXY{ 14, y }, width - 28, groups[g].title);
            y += 14;
            const int count = static_cast<int>(slots[g].size());
            const int cellWidth = (width - 18) / count;
            for (int k = 0; k < count; ++k)
            {
                const auto& card = cards[slots[g][k]];
                const auto cell = p + ScreenCoordsXY{ 10 + k * cellWidth, y };
                const ScreenSize size{ cellWidth - 6, rowHeight };
                if (rowHeight < 40)
                    DrawChoiceChip(rt, cell, size, card, selected == card.action);
                else
                    DrawChoiceCard(rt, cell, size, card, selected == card.action);
            }
            y += rowHeight + 8;
        }
        const auto found = std::find_if(cards.begin(), cards.end(), [&](const auto& c) { return c.action == selected; });
        if (found != cards.end())
            DrawWrappedLabel(rt, p + ScreenCoordsXY{ 14, y + 2 }, width - 28, "{MUTED}" + found->detail, 2);
        DrawFooter(rt, screen, { { "D-pad", "Choose" }, { "A", "Select" }, { "B", back } });
    }
    inline void DrawChoiceCards(
        Drawing::RenderTarget& rt, ScreenSize screen, const std::string& title, const std::vector<ChoiceCard>& cards,
        size_t selected, size_t columns, const std::string& back = "Back")
    {
        const ScreenCoordsXY p{ 8, 8 };
        const int width = screen.width - 16;
        const int height = Layout::ContentHeight(screen);
        DrawSurface(rt, p, { width, height });
        DrawHeadingText(rt, p + ScreenCoordsXY{ 12, 13 }, "{WHITE}" + title, width - 24);
        const int rows = static_cast<int>((cards.size() + columns - 1) / columns);
        const int pitch = std::min(96, (height - 88) / std::max(1, rows));
        const int cellWidth = (width - 18) / static_cast<int>(columns);
        for (size_t i = 0; i < cards.size(); ++i)
        {
            auto cell = p
                + ScreenCoordsXY{ 10 + static_cast<int>(i % columns) * cellWidth, 40 + static_cast<int>(i / columns) * pitch };
            const auto& card = cards[i];
            const bool focused = selected == card.action;
            DrawChoiceCard(rt, cell, { cellWidth - 6, pitch - 6 }, card, focused);
        }
        const auto found = std::find_if(cards.begin(), cards.end(), [&](const auto& c) { return c.action == selected; });
        if (found != cards.end())
            DrawWrappedLabel(rt, p + ScreenCoordsXY{ 14, 44 + rows * pitch }, width - 28, "{MUTED}" + found->detail, 2);
        DrawFooter(rt, screen, { { "D-pad", "Choose" }, { "A", "Select" }, { "B", back } });
    }
} // namespace OpenRCT2::Ui::Handheld
