// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <algorithm>
#include <openrct2/SpriteIds.h>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/drawing/Font.h>
#include <openrct2/drawing/G1Element.h>
#include <openrct2/drawing/RenderTarget.h>
#include <openrct2/world/Location.hpp>
#include <string>
#include <string_view>

// The Pixel theme's text: RCT2's own bitmap font, read straight from g1.dat. The glyph sprites mark
// the letter body with palette index 1 (2 is an outline, 3 a drop shadow), so we paint the body in
// any ink and optionally a one-pixel shadow, at 1x for body text or 2x for big titles. Every pixel
// lands on the game's grid, which is what makes it read as a handheld game.
namespace OpenRCT2::Ui::Handheld::PixelFont
{
    // Set by the game (Explorer.cpp) to g1.dat's sprite lookup. Standalone layout tests link without
    // the engine, so the font measures with a fixed advance there instead of pulling it in.
    inline const OpenRCT2::G1Element* (*glyphSource)(uint32_t) = nullptr;
    inline const OpenRCT2::G1Element* Glyph(uint32_t sprite)
    {
        return glyphSource ? glyphSource(sprite) : nullptr;
    }
    enum class Style : uint8_t
    {
        regular, // RCT2 "small" (the normal window font)
        bold     // RCT2 "medium" (window titles)
    };
    inline uint32_t Base(Style style)
    {
        return SPR_CHAR_START + (style == Style::bold ? 1 : 0) * kSpriteFontGlyphCount;
    }
    // Decodes one UTF-8 character and maps it onto the RCT2 sprite font, which follows Latin-1 for
    // the common symbols (pound, yen, degrees) and has its own slot for the euro.
    inline int GlyphIndex(std::string_view text, size_t& at)
    {
        const auto c = static_cast<unsigned char>(text[at]);
        char32_t code = c;
        if (c >= 0xC0 && at + 1 < text.size())
        {
            const int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : 1;
            code = c & (0x3F >> extra);
            for (int i = 0; i < extra && at + 1 < text.size(); ++i)
                code = (code << 6) | (static_cast<unsigned char>(text[++at]) & 0x3F);
        }
        ++at;
        if (code == 0x20AC)
            code = 0xB5; // euro
        else if (code == 0x2026)
            code = '.';
        else if (code == 0x2212)
            code = '-';
        if (code < 32 || code >= 32 + kSpriteFontGlyphCount)
            code = '?';
        return static_cast<int>(code - 32);
    }
    // RCT2's font has no '|', which the menus use as a separator ("Handyman | Walking"). The small
    // font's 'I' is a plain one-pixel bar, so '|' borrows it with a pixel of air each side.
    constexpr int kBarIndex = '|' - 32;
    constexpr int kCapIIndex = 'I' - 32;
    inline int AdvanceForGlyph(Style style, const OpenRCT2::G1Element* glyph)
    {
        return glyph ? glyph->width - 1 + (style == Style::bold ? 1 : 0) : 4;
    }
    inline int Advance(Style style, int index)
    {
        if (index == kBarIndex)
            return Advance(Style::regular, kCapIIndex) + 2;
        if (index == 0)
            return style == Style::bold ? 5 : 4; // space
        return AdvanceForGlyph(style, Glyph(Base(style) + index));
    }
    inline int Width(std::string_view text, Style style, int scale = 1)
    {
        int width = 0;
        for (size_t at = 0; at < text.size();)
            width += Advance(style, GlyphIndex(text, at));
        return width * scale;
    }
    inline int Height(int scale = 1)
    {
        return 10 * scale; // Both RCT2 sprite fonts are 10 px from cap line to descender.
    }
    inline void DrawGlyphElement(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, const OpenRCT2::G1Element* glyph, Drawing::PaletteIndex ink, int scale)
    {
        if (!glyph || !glyph->offset || scale <= 0 || rt.width <= 0 || rt.height <= 0)
            return;

        const int gx0 = pos.x + glyph->xOffset * scale;
        const int gy0 = pos.y + glyph->yOffset * scale;
        const int gx1 = gx0 + glyph->width * scale;
        const int gy1 = gy0 + glyph->height * scale;
        const int clipX0 = rt.x;
        const int clipY0 = rt.y;
        const int clipX1 = rt.x + rt.width;
        const int clipY1 = rt.y + rt.height;
        if (gx1 <= clipX0 || gy1 <= clipY0 || gx0 >= clipX1 || gy0 >= clipY1)
            return;

        const int stride = rt.LineStride();
        if (scale == 1)
        {
            const int xStart = std::max(0, clipX0 - gx0);
            const int xEnd = std::min<int>(glyph->width, clipX1 - gx0);
            const int yStart = std::max(0, clipY0 - gy0);
            const int yEnd = std::min<int>(glyph->height, clipY1 - gy0);
            const uint8_t* srcRow = glyph->offset + yStart * glyph->width;
            auto* dstRow = rt.bits + (gy0 + yStart - clipY0) * stride + (gx0 - clipX0);
            for (int y = yStart; y < yEnd; ++y, srcRow += glyph->width, dstRow += stride)
            {
                for (int x = xStart; x < xEnd; ++x)
                {
                    if (srcRow[x] == 1)
                        dstRow[x] = ink;
                }
            }
            return;
        }

        const int xStart = std::max(0, (clipX0 - gx0) / scale);
        const int xEnd = std::min<int>(glyph->width, (clipX1 - gx0 + scale - 1) / scale);
        const int yStart = std::max(0, (clipY0 - gy0) / scale);
        const int yEnd = std::min<int>(glyph->height, (clipY1 - gy0 + scale - 1) / scale);
        const uint8_t* srcRow = glyph->offset + yStart * glyph->width;
        for (int y = yStart; y < yEnd; ++y, srcRow += glyph->width)
        {
            const int py0 = std::max(clipY0, gy0 + y * scale);
            const int py1 = std::min(clipY1, gy0 + (y + 1) * scale);
            for (int x = xStart; x < xEnd; ++x)
            {
                if (srcRow[x] != 1)
                    continue;
                const int px0 = std::max(clipX0, gx0 + x * scale);
                const int px1 = std::min(clipX1, gx0 + (x + 1) * scale);
                auto* dstRow = rt.bits + (py0 - clipY0) * stride + (px0 - clipX0);
                for (int py = py0; py < py1; ++py, dstRow += stride)
                {
                    for (int px = 0; px < px1 - px0; ++px)
                        dstRow[px] = ink;
                }
            }
        }
    }
    inline void DrawGlyph(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, Style style, int index, Drawing::PaletteIndex ink, int scale)
    {
        DrawGlyphElement(rt, pos, Glyph(Base(style) + index), ink, scale);
    }
    inline void Draw(
        Drawing::RenderTarget& rt, ScreenCoordsXY pos, std::string_view text, Style style, Drawing::PaletteIndex ink,
        int scale = 1)
    {
        const uint32_t base = Base(style);
        for (size_t at = 0; at < text.size();)
        {
            const int index = GlyphIndex(text, at);
            if (index == kBarIndex)
            {
                const auto* barGlyph = Glyph(Base(Style::regular) + kCapIIndex);
                DrawGlyphElement(rt, pos + ScreenCoordsXY{ scale, 0 }, barGlyph, ink, scale);
                pos.x += (AdvanceForGlyph(Style::regular, barGlyph) + 2) * scale;
            }
            else if (index == 0)
            {
                pos.x += (style == Style::bold ? 5 : 4) * scale;
            }
            else
            {
                const auto* glyph = Glyph(base + index);
                DrawGlyphElement(rt, pos, glyph, ink, scale);
                pos.x += AdvanceForGlyph(style, glyph) * scale;
            }
        }
    }
} // namespace OpenRCT2::Ui::Handheld::PixelFont
