/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2021 - 2025                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "image.h"
#include "math_base.h"

namespace fheroes2
{
    enum class SupportedLanguage : uint8_t;

    enum class FontSize : uint8_t
    {
        SMALL,
        NORMAL,
        LARGE,
        // These are special fonts designed only for buttons. Use WHITE font color for Good Interface and GRAY for Evil Interface.
        BUTTON_RELEASED,
        BUTTON_PRESSED,
    };

    enum class FontColor : uint8_t
    {
        WHITE,
        GRAY,
        YELLOW,
        GOLDEN_GRADIENT,
        SILVER_GRADIENT,
    };

    struct FontType
    {
        FontType() = default;

        FontType( const FontSize size_, const FontColor color_ )
            : size( size_ )
            , color( color_ )
        {
            // Do nothing.
        }

        FontSize size = FontSize::NORMAL;
        FontColor color = FontColor::WHITE;

        static FontType normalWhite()
        {
            return { FontSize::NORMAL, FontColor::WHITE };
        }

        static FontType normalYellow()
        {
            return { FontSize::NORMAL, FontColor::YELLOW };
        }

        static FontType smallWhite()
        {
            return { FontSize::SMALL, FontColor::WHITE };
        }

        static FontType smallYellow()
        {
            return { FontSize::SMALL, FontColor::YELLOW };
        }

        static FontType largeWhite()
        {
            return { FontSize::LARGE, FontColor::WHITE };
        }

        static FontType buttonReleasedWhite()
        {
            return { FontSize::BUTTON_RELEASED, FontColor::WHITE };
        }
    };

    struct TextLineInfo
    {
        TextLineInfo() = default;

        TextLineInfo( const int32_t offsetX_, const int32_t offsetY_, const int32_t lineWidth_, const int32_t count )
            : offsetX( offsetX_ )
            , offsetY( offsetY_ )
            , lineWidth( lineWidth_ )
            , characterCount( count )
        {
            // Do nothing.
        }

        int32_t offsetX{ 0 };
        int32_t offsetY{ 0 };
        int32_t lineWidth{ 0 };
        int32_t characterCount{ 0 };
    };

    int32_t getFontHeight( const FontSize fontSize );

    class TextBase
    {
    public:
        TextBase() = default;
        virtual ~TextBase();

        // Returns width of a text as a single-line text only.
        virtual int32_t width() const = 0;

        // Returns height of a text as a single-line text only.
        virtual int32_t height() const = 0;

        // Returns width of a text as a multi-line text limited by maximum width of a line.
        virtual int32_t width( const int32_t maxWidth ) const = 0;

        // Returns height of a text as a multi-line text limited by width of a line.
        virtual int32_t height( const int32_t maxWidth ) const = 0;

        // Returns number of multi-line text rows limited by width of a line. It can be 0 if the text is empty.
        virtual int32_t rows( const int32_t maxWidth ) const = 0;

        // Returns the text line ROI relative to the text line begin. It analyzes offset and size of all characters in the text.
        virtual Rect area() const = 0;

        // Draw text as a single line text.
        void draw( const int32_t x, const int32_t y, Image & output ) const
        {
            drawInRoi( x, y, output, { 0, 0, output.width(), output.height() } );
        }

        // Draw text as a multi-line limited by width of a line. The text will be centered according to the provided maximum width.
        void draw( const int32_t x, const int32_t y, const int32_t maxWidth, Image & output ) const
        {
            drawInRoi( x, y, maxWidth, output, { 0, 0, output.width(), output.height() } );
        }

        // Draw text as a single line text within a given image ROI.
        virtual void drawInRoi( const int32_t x, const int32_t y, Image & output, const Rect & imageRoi ) const = 0;

        // Draw text as a multi-line within a given image ROI and limited by width of a line. The text will be centered according to the provided maximum width.
        virtual void drawInRoi( const int32_t x, const int32_t y, const int32_t maxWidth, Image & output, const Rect & imageRoi ) const = 0;

        // Returns true if nothing to draw.
        virtual bool empty() const = 0;

        // Returns full text. Multi-text class cannot return by reference hence returning by value.
        virtual std::string text() const = 0;

        void setUniformVerticalAlignment( const bool isUniform )
        {
            _isUniformedVerticalAlignment = isUniform;
        }

        const std::optional<SupportedLanguage> & getLanguage() const
        {
            return _language;
        }

    protected:
        std::optional<SupportedLanguage> _language;

        bool _isUniformedVerticalAlignment{ true };
    };

    class Text : public TextBase
    {
    public:
        friend class MultiFontText;

        Text() = default;

        Text( std::string text, const FontType fontType )
            : _text( std::move( text ) )
            , _fontType( fontType )
        {
            // Do nothing.
        }

        Text( std::string text, const FontType fontType, const std::optional<SupportedLanguage> language )
            : _text( std::move( text ) )
            , _fontType( fontType )
        {
            _language = language;
        }

        Text( const Text & text ) = default;
        Text( Text && text ) = default;
        Text & operator=( const Text & text ) = default;
        Text & operator=( Text && text ) = default;

        ~Text() override;

        int32_t width() const override;
        int32_t height() const override;

        int32_t width( const int32_t maxWidth ) const override;
        int32_t height( const int32_t maxWidth ) const override;

        int32_t rows( const int32_t maxWidth ) const override;

        Rect area() const override;

        void drawInRoi( const int32_t x, const int32_t y, Image & output, const Rect & imageRoi ) const override;
        void drawInRoi( const int32_t x, const int32_t y, const int32_t maxWidth, Image & output, const Rect & imageRoi ) const override;

        bool empty() const override
        {
            return _text.empty();
        }

        void set( std::string text, const FontType fontType )
        {
            _text = std::move( text );
            _fontType = fontType;
            _language = std::nullopt;
        }

        void set( std::string text, const FontType fontType, const std::optional<SupportedLanguage> language )
        {
            _text = std::move( text );
            _fontType = fontType;
            _language = language;
        }

        // This method modifies the underlying text and ends it with '...' if it is longer than the provided width.
        virtual void fitToOneRow( const int32_t maxWidth );

        std::string text() const override
        {
            return _text;
        }

        FontType getFontType() const
        {
            return _fontType;
        }

        // Sets to keep trailing spaces at each text line end including the end of the text.
        void keepLineTrailingSpaces()
        {
            _keepLineTrailingSpaces = true;
        }

        // Returns text lines parameters (in pixels) in 'offsets': x - horizontal line shift, y - vertical line shift.
        // And in 'characterCount' - the number of characters on the line, in 'lineWidth' the width including the `offsetX` value.
        // The 'keepTextTrailingSpaces' is used to take into account all the spaces at the text end in example when you want to join multiple texts in multi-font texts.
        void getTextLineInfos( std::vector<TextLineInfo> & textLineInfos, const int32_t maxWidth, const int32_t rowHeight, const bool keepTextTrailingSpaces ) const;

    protected:
        std::string _text;

        FontType _fontType;

        bool _keepLineTrailingSpaces{ false };
    };

    class TextInput final : public Text
    {
    public:
        using Text::Text;

        void setCursorPosition( const size_t position )
        {
            _cursorPosition = position;
        }

        void fitToOneRow( const int32_t maxWidth ) override;

        size_t getOffsetX() const
        {
            return _textOffsetX;
        }

    private:
        size_t _cursorPosition{ 0 };
        size_t _textOffsetX{ 0 };
    };

    class MultiFontText final : public TextBase
    {
    public:
        MultiFontText() = default;
        ~MultiFontText() override;

        void add( Text text );

        int32_t width() const override;
        int32_t height() const override;

        int32_t width( const int32_t maxWidth ) const override;
        int32_t height( const int32_t maxWidth ) const override;

        int32_t rows( const int32_t maxWidth ) const override;

        Rect area() const override;

        void drawInRoi( const int32_t x, const int32_t y, Image & output, const Rect & imageRoi ) const override;
        void drawInRoi( const int32_t x, const int32_t y, const int32_t maxWidth, Image & output, const Rect & imageRoi ) const override;

        bool empty() const override
        {
            return _texts.empty();
        }

        std::string text() const override;

    private:
        void _getMultiFontTextLineInfos( std::vector<TextLineInfo> & textLineInfos, const int32_t maxWidth, const int32_t rowHeight ) const;

        std::vector<Text> _texts;
    };

    class FontCharHandler
    {
    public:
        explicit FontCharHandler( const FontType fontType );

        // Returns true if character is available to render, including space (' ') and new line ('\n').
        bool isAvailable( const uint8_t character ) const;

        const Sprite & getSprite( const uint8_t character ) const;

        int32_t getWidth( const uint8_t character ) const;

        int32_t getSpaceCharWidth() const
        {
            return _spaceCharWidth;
        }

    private:
        // Returns true if character is valid for the current code page, excluding space (' ') and new line ('\n').
        bool _isValid( const uint8_t character ) const;

        int32_t _getSpaceCharWidth() const;

        const FontType _fontType;
        const uint32_t _charLimit;
        const int32_t _spaceCharWidth;
    };

    // This function is usually useful for text generation on buttons as button font is a separate set of sprites.
    bool isFontAvailable( const std::string_view text, const FontType fontType );
}
