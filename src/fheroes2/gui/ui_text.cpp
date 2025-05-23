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

#include "ui_text.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <map>
#include <memory>

#include "agg_image.h"
#include "icn.h"
#include "ui_language.h"

namespace
{
    const uint8_t hyphenChar{ '-' };

    const uint8_t invalidChar{ '?' };

    const std::string truncationSymbol( "..." );

    // Returns true if character is a line separator ('\n').
    bool isLineSeparator( const uint8_t character )
    {
        return ( character == '\n' );
    }

    // Returns true if character is a Space character (' ').
    bool isSpaceChar( const uint8_t character )
    {
        return ( character == ' ' );
    }

    const fheroes2::Sprite errorImage;

    const fheroes2::Sprite & getChar( const uint8_t character, const fheroes2::FontType & fontType )
    {
        if ( character < 0x21 ) {
            return errorImage;
        }

        switch ( fontType.size ) {
        case fheroes2::FontSize::SMALL:
            switch ( fontType.color ) {
            case fheroes2::FontColor::WHITE:
                return fheroes2::AGG::GetICN( ICN::SMALFONT, character - 0x20 );
            case fheroes2::FontColor::GRAY:
                return fheroes2::AGG::GetICN( ICN::GRAY_SMALL_FONT, character - 0x20 );
            case fheroes2::FontColor::YELLOW:
                return fheroes2::AGG::GetICN( ICN::YELLOW_SMALLFONT, character - 0x20 );
            default:
                // Did you add a new font color? Add the corresponding logic for it!
                assert( 0 );
                break;
            }
            break;
        case fheroes2::FontSize::NORMAL:
            switch ( fontType.color ) {
            case fheroes2::FontColor::WHITE:
                return fheroes2::AGG::GetICN( ICN::FONT, character - 0x20 );
            case fheroes2::FontColor::GRAY:
                return fheroes2::AGG::GetICN( ICN::GRAY_FONT, character - 0x20 );
            case fheroes2::FontColor::YELLOW:
                return fheroes2::AGG::GetICN( ICN::YELLOW_FONT, character - 0x20 );
            case fheroes2::FontColor::GOLDEN_GRADIENT:
                return fheroes2::AGG::GetICN( ICN::GOLDEN_GRADIENT_FONT, character - 0x20 );
            case fheroes2::FontColor::SILVER_GRADIENT:
                return fheroes2::AGG::GetICN( ICN::SILVER_GRADIENT_FONT, character - 0x20 );
            default:
                // Did you add a new font color? Add the corresponding logic for it!
                assert( 0 );
                break;
            }
            break;
        case fheroes2::FontSize::LARGE:
            switch ( fontType.color ) {
            case fheroes2::FontColor::WHITE:
                return fheroes2::AGG::GetICN( ICN::WHITE_LARGE_FONT, character - 0x20 );
            case fheroes2::FontColor::GOLDEN_GRADIENT:
                return fheroes2::AGG::GetICN( ICN::GOLDEN_GRADIENT_LARGE_FONT, character - 0x20 );
            case fheroes2::FontColor::SILVER_GRADIENT:
                return fheroes2::AGG::GetICN( ICN::SILVER_GRADIENT_LARGE_FONT, character - 0x20 );
            default:
                // Did you add a new font color? Add the corresponding logic for it!
                assert( 0 );
                break;
            }
            break;
        case fheroes2::FontSize::BUTTON_RELEASED:
            switch ( fontType.color ) {
            case fheroes2::FontColor::WHITE:
                return fheroes2::AGG::GetICN( ICN::BUTTON_GOOD_FONT_RELEASED, character - 0x20 );
            case fheroes2::FontColor::GRAY:
                return fheroes2::AGG::GetICN( ICN::BUTTON_EVIL_FONT_RELEASED, character - 0x20 );
            default:
                // Did you add a new font color? Add the corresponding logic for it!
                assert( 0 );
                break;
            }
            break;
        case fheroes2::FontSize::BUTTON_PRESSED:
            switch ( fontType.color ) {
            case fheroes2::FontColor::WHITE:
                return fheroes2::AGG::GetICN( ICN::BUTTON_GOOD_FONT_PRESSED, character - 0x20 );
            case fheroes2::FontColor::GRAY:
                return fheroes2::AGG::GetICN( ICN::BUTTON_EVIL_FONT_PRESSED, character - 0x20 );
            default:
                // Did you add a new font color? Add the corresponding logic for it!
                assert( 0 );
                break;
            }
            break;
        default:
            // Did you add a new font size? Add the corresponding logic for it!
            assert( 0 );
            break;
        }

        assert( 0 ); // Did you add a new font size? Please add implementation.

        return errorImage;
    }

    uint32_t getCharacterLimit( const fheroes2::FontSize fontSize )
    {
        switch ( fontSize ) {
        case fheroes2::FontSize::SMALL:
            return fheroes2::AGG::GetICNCount( ICN::SMALFONT ) + 0x20 - 1;
        case fheroes2::FontSize::NORMAL:
        case fheroes2::FontSize::LARGE:
            return fheroes2::AGG::GetICNCount( ICN::FONT ) + 0x20 - 1;
        case fheroes2::FontSize::BUTTON_RELEASED:
        case fheroes2::FontSize::BUTTON_PRESSED:
            return fheroes2::AGG::GetICNCount( ICN::BUTTON_GOOD_FONT_RELEASED ) + 0x20 - 1;
        default:
            // Did you add a new font size? Please add implementation.
            assert( 0 );
        }

        return 0;
    }

    int32_t getLineWidth( const uint8_t * data, const int32_t size, const fheroes2::FontCharHandler & charHandler, const bool keepTrailingSpaces )
    {
        assert( data != nullptr && size > 0 );

        int32_t width = 0;
        const uint8_t * dataEnd = data + size;

        if ( keepTrailingSpaces ) {
            for ( ; data != dataEnd; ++data ) {
                width += charHandler.getWidth( *data );
            }

            return width;
        }

        int32_t spaceWidth = 0;
        const int32_t spaceCharWidth = charHandler.getSpaceCharWidth();

        for ( ; data != dataEnd; ++data ) {
            if ( isSpaceChar( *data ) ) {
                spaceWidth += spaceCharWidth;
            }
            else if ( !isLineSeparator( *data ) ) {
                width += spaceWidth + charHandler.getWidth( *data );

                spaceWidth = 0;
            }
        }

        return width;
    }

    fheroes2::Rect getTextLineArea( const uint8_t * data, const int32_t size, const fheroes2::FontCharHandler & charHandler )
    {
        assert( data != nullptr && size > 0 );

        const uint8_t * dataEnd = data + size;

        const fheroes2::Sprite & firstCharSprite = charHandler.getSprite( *data );
        fheroes2::Rect area( firstCharSprite.x(), firstCharSprite.y(), firstCharSprite.width(), firstCharSprite.height() );
        ++data;

        for ( ; data != dataEnd; ++data ) {
            const fheroes2::Sprite & sprite = charHandler.getSprite( *data );

            if ( const int32_t spriteY = sprite.y(); spriteY < area.y ) {
                // This character sprite is drawn higher than all previous - update `height` and `y`.
                area.height += area.y - spriteY;
                area.y = spriteY;
                area.height = std::max( area.height, sprite.height() );
            }
            else {
                area.height = std::max( area.height, spriteY - area.y + sprite.height() );
            }

            area.width += sprite.x() + sprite.width();
        }

        return area;
    }

    int32_t getMaxCharacterCount( const uint8_t * data, const int32_t size, const fheroes2::FontCharHandler & charHandler, const int32_t maxWidth )
    {
        assert( data != nullptr && size > 0 && maxWidth > 0 );

        int32_t width = 0;

        for ( int32_t characterCount = 0; characterCount < size; ++characterCount, ++data ) {
            width += charHandler.getWidth( *data );

            if ( width > maxWidth ) {
                return characterCount;
            }
        }

        return size;
    }

    int32_t renderSingleLine( const uint8_t * data, const int32_t size, const int32_t x, const int32_t y, fheroes2::Image & output, const fheroes2::Rect & imageRoi,
                              const fheroes2::FontCharHandler & charHandler )
    {
        assert( data != nullptr && size > 0 && !output.empty() );

        int32_t offsetX = x;

        const int32_t spaceCharWidth = charHandler.getSpaceCharWidth();
        const uint8_t * dataEnd = data + size;

        for ( ; data != dataEnd; ++data ) {
            if ( isSpaceChar( *data ) ) {
                offsetX += spaceCharWidth;
                continue;
            }

            // TODO: remove this hack or expand it to cover more cases.
            if ( isLineSeparator( *data ) ) {
                // This should never happen as a line cannot contain line separator in the middle.
                // But due to some limitations in UI we have to deal with it.
                // The only way is to just ignore it here.
                continue;
            }

            const fheroes2::Sprite & charSprite = charHandler.getSprite( *data );
            assert( !charSprite.empty() );

            const fheroes2::Rect charRoi{ offsetX + charSprite.x(), y + charSprite.y(), charSprite.width(), charSprite.height() };

            const fheroes2::Rect overlappedRoi = imageRoi ^ charRoi;

            fheroes2::Blit( charSprite, overlappedRoi.x - charRoi.x, overlappedRoi.y - charRoi.y, output, overlappedRoi.x, overlappedRoi.y, overlappedRoi.width,
                            overlappedRoi.height );
            offsetX += charSprite.width() + charSprite.x();
        }

        return offsetX;
    }

    int32_t getMaxWordWidth( const uint8_t * data, const int32_t size, const fheroes2::FontType fontType )
    {
        assert( data != nullptr && size > 0 );

        const fheroes2::FontCharHandler charHandler( fontType );

        int32_t maxWidth = 1;
        int32_t width = 0;

        const uint8_t * dataEnd = data + size;
        while ( data != dataEnd ) {
            if ( isSpaceChar( *data ) || isLineSeparator( *data ) ) {
                // If it is the end of line ("\n") or a space (" "), then the word has ended.
                if ( width == 0 && isSpaceChar( *data ) ) {
                    // No words exist on this till now. Let's put maximum width as space width.
                    width = charHandler.getWidth( *data );
                }

                if ( maxWidth < width ) {
                    maxWidth = width;
                }

                width = 0;
            }
            else {
                width += charHandler.getWidth( *data );
            }

            ++data;
        }

        return std::max( maxWidth, width );
    }

    std::unique_ptr<fheroes2::LanguageSwitcher> getLanguageSwitcher( const fheroes2::TextBase & text )
    {
        const auto & language = text.getLanguage();
        if ( !language ) {
            return {};
        }

        return std::make_unique<fheroes2::LanguageSwitcher>( language.value() );
    }

    int32_t getTruncationSymbolWidth( const fheroes2::FontType fontType )
    {
        // Symbol width depends on font size and not on its color.
        static std::map<fheroes2::FontSize, int32_t> truncationSymbolWidth;

        auto [iter, isEmplaced] = truncationSymbolWidth.try_emplace( fontType.size, 0 );

        if ( isEmplaced ) {
            // Set the correct width value for the just emplaced element.
            iter->second = getLineWidth( reinterpret_cast<const uint8_t *>( truncationSymbol.data() ), static_cast<int32_t>( truncationSymbol.size() ),
                                         fheroes2::FontCharHandler( fontType ), true );
        }

        return iter->second;
    }
}

namespace fheroes2
{
    int32_t getFontHeight( const FontSize fontSize )
    {
        switch ( fontSize ) {
        case FontSize::SMALL:
            return 8 + 2 + 1;
        case FontSize::NORMAL:
            return 13 + 3 + 1;
        case FontSize::LARGE:
            return 26 + 6 + 1;
        case FontSize::BUTTON_RELEASED:
        case FontSize::BUTTON_PRESSED:
            return 15;
        default:
            assert( 0 ); // Did you add a new font size? Please add implementation.
            break;
        }

        return 0;
    }

    TextBase::~TextBase() = default;

    Text::~Text() = default;

    // TODO: Properly handle strings with many text lines ('\n'). Now their widths are counted as if they're one line.
    int32_t Text::width() const
    {
        const auto languageSwitcher = getLanguageSwitcher( *this );
        const fheroes2::FontCharHandler charHandler( _fontType );

        return getLineWidth( reinterpret_cast<const uint8_t *>( _text.data() ), static_cast<int32_t>( _text.size() ), charHandler, _keepLineTrailingSpaces );
    }

    // TODO: Properly handle strings with many text lines ('\n'). Now their heights are counted as if they're one line.
    int32_t Text::height() const
    {
        const auto languageSwitcher = getLanguageSwitcher( *this );
        return getFontHeight( _fontType.size );
    }

    int32_t Text::width( const int32_t maxWidth ) const
    {
        if ( _text.empty() ) {
            return 0;
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );
        const int32_t fontHeight = height();

        std::vector<TextLineInfo> lineInfos;
        getTextLineInfos( lineInfos, maxWidth, fontHeight, false );

        if ( lineInfos.size() == 1 ) {
            // This is a single-line message.
            return lineInfos.front().lineWidth;
        }

        if ( !_isUniformedVerticalAlignment ) {
            // This is a multi-lined message and we try to fit as many words on every line as possible.
            return std::max_element( lineInfos.begin(), lineInfos.end(), []( const TextLineInfo & a, const TextLineInfo & b ) { return a.lineWidth < b.lineWidth; } )
                ->lineWidth;
        }

        // This is a multi-line message. Optimize it to fit the text evenly to the same number of lines.
        int32_t startWidth = getMaxWordWidth( reinterpret_cast<const uint8_t *>( _text.data() ), static_cast<int32_t>( _text.size() ), _fontType );
        int32_t endWidth = maxWidth;

        while ( startWidth + 1 < endWidth ) {
            const int32_t currentWidth = ( endWidth + startWidth ) / 2;
            std::vector<TextLineInfo> tempLineInfos;
            getTextLineInfos( tempLineInfos, currentWidth, fontHeight, false );

            if ( tempLineInfos.size() > lineInfos.size() ) {
                startWidth = currentWidth;
                continue;
            }

            endWidth = currentWidth;
        }

        return endWidth;
    }

    int32_t Text::height( const int32_t maxWidth ) const
    {
        if ( _text.empty() ) {
            return 0;
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );
        const int32_t fontHeight = height();

        std::vector<TextLineInfo> lineInfos;
        getTextLineInfos( lineInfos, maxWidth, fontHeight, false );

        return lineInfos.back().offsetY + fontHeight;
    }

    int32_t Text::rows( const int32_t maxWidth ) const
    {
        if ( _text.empty() ) {
            return 0;
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );
        std::vector<TextLineInfo> lineInfos;
        getTextLineInfos( lineInfos, maxWidth, height(), false );

        return static_cast<int32_t>( lineInfos.size() );
    }

    Rect Text::area() const
    {
        if ( _text.empty() ) {
            return {};
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );
        const fheroes2::FontCharHandler charHandler( _fontType );

        return getTextLineArea( reinterpret_cast<const uint8_t *>( _text.data() ), static_cast<int32_t>( _text.size() ), charHandler );
    }

    void Text::drawInRoi( const int32_t x, const int32_t y, Image & output, const Rect & imageRoi ) const
    {
        if ( output.empty() || _text.empty() ) {
            // No use to render something on an empty image or if something is empty.
            return;
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );
        const fheroes2::FontCharHandler charHandler( _fontType );

        renderSingleLine( reinterpret_cast<const uint8_t *>( _text.data() ), static_cast<int32_t>( _text.size() ), x, y, output, imageRoi, charHandler );
    }

    void Text::drawInRoi( const int32_t x, const int32_t y, const int32_t maxWidth, Image & output, const Rect & imageRoi ) const
    {
        if ( output.empty() || _text.empty() ) {
            // No use to render something on an empty image or if something is empty.
            return;
        }

        if ( maxWidth <= 0 ) {
            drawInRoi( x, y, output, imageRoi );
            return;
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );

        std::vector<TextLineInfo> lineInfos;
        getTextLineInfos( lineInfos, maxWidth, height(), false );

        const uint8_t * data = reinterpret_cast<const uint8_t *>( _text.data() );
        const fheroes2::FontCharHandler charHandler( _fontType );

        for ( const TextLineInfo & info : lineInfos ) {
            if ( info.characterCount > 0 ) {
                // Center the text line when rendering multi-line texts.
                // TODO: Implement text alignment setting to allow multi-line left aligned text for editor's warning messages.
                const int32_t offsetX = info.offsetX + ( maxWidth - info.lineWidth ) / 2;

                renderSingleLine( data, info.characterCount, x + offsetX, y + info.offsetY, output, imageRoi, charHandler );
            }

            data += info.characterCount;
        }
    }

    void Text::fitToOneRow( const int32_t maxWidth )
    {
        assert( maxWidth > 0 ); // Why is the limit less than 1?
        if ( maxWidth <= 0 ) {
            return;
        }

        if ( _text.empty() ) {
            // Nothing needs to be done.
            return;
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );
        const fheroes2::FontCharHandler charHandler( _fontType );

        const int32_t originalTextWidth
            = getLineWidth( reinterpret_cast<const uint8_t *>( _text.data() ), static_cast<int32_t>( _text.size() ), charHandler, _keepLineTrailingSpaces );
        if ( originalTextWidth <= maxWidth ) {
            // Nothing to do. The text is not longer than the provided maximum width.
            return;
        }

        const int32_t maxCharacterCount = getMaxCharacterCount( reinterpret_cast<const uint8_t *>( _text.data() ), static_cast<int32_t>( _text.size() ), charHandler,
                                                                maxWidth - getTruncationSymbolWidth( _fontType ) );

        _text.resize( maxCharacterCount );
        _text += truncationSymbol;
    }

    void TextInput::fitToOneRow( const int32_t maxWidth )
    {
        assert( maxWidth > 0 );
        if ( maxWidth <= 0 || _text.empty() ) {
            return;
        }

        const auto languageSwitcher = getLanguageSwitcher( *this );
        const fheroes2::FontCharHandler charHandler( _fontType );
        const int32_t fullLineWidth = getLineWidth( reinterpret_cast<const uint8_t *>( _text.data() ), static_cast<int32_t>( _text.size() ), charHandler, true );
        if ( fullLineWidth < maxWidth ) {
            return;
        }

        constexpr size_t cursorToBorderDistance = 4;

        // If the cursor is to the left of the TextBox.
        _textOffsetX = std::max( std::min( static_cast<int>( _textOffsetX ), static_cast<int>( _cursorPosition - cursorToBorderDistance ) ), 0 );

        // If some characters were deleted and we have space for new characters.
        int32_t currentWidth
            = getLineWidth( reinterpret_cast<const uint8_t *>( _text.data() + _textOffsetX ), static_cast<int32_t>( _text.size() - _textOffsetX ), charHandler, true );
        const uint8_t * textData = reinterpret_cast<const uint8_t *>( _text.data() );

        while ( _textOffsetX > 0 ) {
            const uint8_t prevChar = textData[_textOffsetX - 1];
            currentWidth += charHandler.getWidth( prevChar );

            if ( currentWidth > maxWidth ) {
                break;
            }

            --_textOffsetX;
        }

        // If the cursor is to the right of the Textbox.
        int32_t maxCharacterCount = getMaxCharacterCount( reinterpret_cast<const uint8_t *>( _text.data() + _textOffsetX ),
                                                          static_cast<int32_t>( _text.size() - _textOffsetX ), charHandler, maxWidth );
        while ( ( _textOffsetX + maxCharacterCount <= _cursorPosition + cursorToBorderDistance ) && ( _textOffsetX + maxCharacterCount < _text.size() ) ) {
            ++_textOffsetX;
            maxCharacterCount = getMaxCharacterCount( reinterpret_cast<const uint8_t *>( _text.data() + _textOffsetX ),
                                                      static_cast<int32_t>( _text.size() - _textOffsetX ), charHandler, maxWidth );
        }

        const size_t originalTextSize = _text.size();
        _text = _text.substr( _textOffsetX, maxCharacterCount );

        const int32_t truncationSymbolWidth = getTruncationSymbolWidth( _fontType );

        // Insert truncation symbol at the beginning if required.
        if ( _textOffsetX != 0 ) {
            int totalWidth = 0;
            int charCount = 0;

            for ( auto iter = _text.begin(); iter != _text.end(); ++iter ) {
                if ( totalWidth >= truncationSymbolWidth ) {
                    break;
                }

                totalWidth += charHandler.getWidth( *iter );
                ++charCount;
            }

            _text.erase( 0, charCount );
            _text.insert( 0, truncationSymbol );
        }

        // Insert truncation symbol at the end if required.
        if ( _text.size() + _textOffsetX < originalTextSize ) {
            int totalWidth = 0;
            int charCount = 0;

            for ( auto iter = _text.rbegin(); iter != _text.rend(); ++iter ) {
                if ( totalWidth >= truncationSymbolWidth ) {
                    break;
                }

                totalWidth += charHandler.getWidth( *iter );
                ++charCount;
            }

            _text.erase( _text.size() - charCount, charCount );
            _text.insert( _text.size(), truncationSymbol );
        }
    }

    void Text::getTextLineInfos( std::vector<TextLineInfo> & textLineInfos, const int32_t maxWidth, const int32_t rowHeight, const bool keepTextTrailingSpaces ) const
    {
        assert( !_text.empty() );

        const uint8_t * data = reinterpret_cast<const uint8_t *>( _text.data() );

        const int32_t firstLineOffsetX = textLineInfos.empty() ? 0 : textLineInfos.back().lineWidth;
        int32_t lineWidth = firstLineOffsetX;
        int32_t offsetY = textLineInfos.empty() ? 0 : textLineInfos.back().offsetY;

        const fheroes2::FontCharHandler charHandler( _fontType );

        if ( maxWidth < 1 ) {
            // The text will be displayed in a single line.

            const int32_t size = static_cast<int32_t>( _text.size() );
            lineWidth += getLineWidth( data, size, charHandler, _keepLineTrailingSpaces || keepTextTrailingSpaces );

            textLineInfos.emplace_back( firstLineOffsetX, offsetY, lineWidth, size );
            return;
        }

        int32_t offsetX = firstLineOffsetX;
        int32_t lineCharCount = 0;
        int32_t lastWordCharCount = 0;
        int32_t spaceCharCount = 0;

        const uint8_t * dataEnd = data + _text.size();

        while ( data != dataEnd ) {
            if ( isLineSeparator( *data ) ) {
                if ( !_keepLineTrailingSpaces ) {
                    lineWidth -= spaceCharCount * charHandler.getSpaceCharWidth();
                }

                textLineInfos.emplace_back( offsetX, offsetY, lineWidth, lineCharCount + 1 );

                spaceCharCount = 0;
                offsetX = 0;
                offsetY += rowHeight;
                lineCharCount = 0;
                lastWordCharCount = 0;
                lineWidth = 0;

                ++data;
            }
            else {
                // This is another character in the line. Get its width.

                const int32_t charWidth = charHandler.getWidth( *data );

                if ( lineWidth + charWidth > maxWidth ) {
                    // Current character has exceeded the maximum line width.

                    if ( !_keepLineTrailingSpaces && isSpaceChar( *data ) ) {
                        // Current character could be a space character then current line is over.
                        // For the characters count we take this space into the account.
                        ++lineCharCount;

                        // Skip this space character.
                        ++data;
                    }
                    else if ( lineCharCount == lastWordCharCount ) {
                        // This is the only word in the line.
                        // Search for '-' symbol to avoid truncating the word in the middle.
                        const uint8_t * hyphenPos = data - lineCharCount;
                        for ( ; hyphenPos != data; ++hyphenPos ) {
                            if ( *hyphenPos == hyphenChar ) {
                                break;
                            }
                        }

                        if ( hyphenPos != data ) {
                            // The '-' symbol has been found. In this case we consider everything after it as a separate word.
                            const int32_t postHyphenCharCount = static_cast<int32_t>( data - hyphenPos ) - 1;

                            lineCharCount -= postHyphenCharCount;
                            lineWidth -= getLineWidth( data - postHyphenCharCount, postHyphenCharCount, charHandler, true );

                            data = hyphenPos;
                            ++data;
                        }
                        else if ( firstLineOffsetX > 0 && ( textLineInfos.empty() || textLineInfos.back().offsetY == offsetY ) ) {
                            // This word was not the first in the line so we can move it to the next line.
                            // It can happen in the case of the multi-font text.
                            data -= lastWordCharCount;

                            lineCharCount = 0;
                            lineWidth = firstLineOffsetX;
                        }
                    }
                    else if ( lastWordCharCount > 0 ) {
                        // Exclude last word from this line.
                        data -= lastWordCharCount;

                        lineCharCount -= lastWordCharCount;
                        lineWidth -= getLineWidth( data, lastWordCharCount, charHandler, true );
                    }

                    if ( !_keepLineTrailingSpaces ) {
                        lineWidth -= spaceCharCount * charHandler.getSpaceCharWidth();
                    }

                    textLineInfos.emplace_back( offsetX, offsetY, lineWidth, lineCharCount );

                    spaceCharCount = 0;
                    offsetX = 0;
                    offsetY += rowHeight;
                    lineCharCount = 0;
                    lastWordCharCount = 0;
                    lineWidth = 0;
                }
                else {
                    if ( isSpaceChar( *data ) ) {
                        lastWordCharCount = 0;
                        ++spaceCharCount;
                    }
                    else {
                        ++lastWordCharCount;
                        spaceCharCount = 0;
                    }

                    ++data;
                    ++lineCharCount;
                    lineWidth += charWidth;
                }
            }
        }

        if ( !_keepLineTrailingSpaces && !keepTextTrailingSpaces ) {
            lineWidth -= spaceCharCount * charHandler.getSpaceCharWidth();
        }

        textLineInfos.emplace_back( offsetX, offsetY, lineWidth, lineCharCount );
    }

    MultiFontText::~MultiFontText() = default;

    void MultiFontText::add( Text text )
    {
        if ( !text._text.empty() ) {
            _texts.emplace_back( std::move( text ) );
        }
    }

    int32_t MultiFontText::width() const
    {
        int32_t totalWidth = 0;
        for ( const Text & text : _texts ) {
            totalWidth += text.width();
        }

        return totalWidth;
    }

    int32_t MultiFontText::height() const
    {
        int32_t maxHeight = 0;

        for ( const Text & text : _texts ) {
            maxHeight = std::max( maxHeight, text.height() );
        }

        return maxHeight;
    }

    int32_t MultiFontText::width( const int32_t maxWidth ) const
    {
        const int32_t maxFontHeight = height();

        std::vector<TextLineInfo> lineInfos;
        _getMultiFontTextLineInfos( lineInfos, maxWidth, maxFontHeight );

        int32_t maxRowWidth = lineInfos.front().lineWidth;
        for ( const TextLineInfo & lineInfo : lineInfos ) {
            maxRowWidth = std::max( maxRowWidth, lineInfo.lineWidth );
        }

        return maxRowWidth;
    }

    int32_t MultiFontText::height( const int32_t maxWidth ) const
    {
        const int32_t maxFontHeight = height();

        std::vector<TextLineInfo> lineInfos;
        _getMultiFontTextLineInfos( lineInfos, maxWidth, maxFontHeight );

        return lineInfos.back().offsetY + maxFontHeight;
    }

    int32_t MultiFontText::rows( const int32_t maxWidth ) const
    {
        if ( _texts.empty() ) {
            return 0;
        }

        const int32_t maxFontHeight = height();

        std::vector<TextLineInfo> lineInfos;
        _getMultiFontTextLineInfos( lineInfos, maxWidth, maxFontHeight );

        if ( lineInfos.empty() ) {
            return 0;
        }

        return static_cast<int32_t>( lineInfos.size() );
    }

    Rect MultiFontText::area() const
    {
        Rect area;
        bool isFirstText = true;

        for ( const Text & text : _texts ) {
            if ( isFirstText ) {
                isFirstText = false;
                area = text.area();
                continue;
            }

            const fheroes2::Rect & textArea = text.area();

            if ( textArea.y < area.y ) {
                // This character sprite is drawn higher than all previous - update `height` and `y`.
                area.height += area.y - textArea.y;
                area.y = textArea.y;
                area.height = std::max( area.height, textArea.height );
            }
            else {
                area.height = std::max( area.height, textArea.y - area.y + textArea.height );
            }

            area.width += textArea.x + textArea.width;
        }

        return area;
    }

    void MultiFontText::drawInRoi( const int32_t x, const int32_t y, Image & output, const Rect & imageRoi ) const
    {
        if ( output.empty() || _texts.empty() ) {
            // No use to render something on an empty image or if something is empty.
            return;
        }

        const int32_t maxFontHeight = height();

        int32_t offsetX = x;
        for ( const Text & text : _texts ) {
            const auto languageSwitcher = getLanguageSwitcher( text );
            const int32_t fontHeight = getFontHeight( text._fontType.size );
            const fheroes2::FontCharHandler charHandler( text._fontType );

            offsetX = renderSingleLine( reinterpret_cast<const uint8_t *>( text._text.data() ), static_cast<int32_t>( text._text.size() ), offsetX,
                                        y + ( maxFontHeight - fontHeight ) / 2, output, imageRoi, charHandler );
        }
    }

    void MultiFontText::drawInRoi( const int32_t x, const int32_t y, const int32_t maxWidth, Image & output, const Rect & imageRoi ) const
    {
        if ( output.empty() || _texts.empty() ) {
            // No use to render something on an empty image or if something is empty.
            return;
        }

        const int32_t maxFontHeight = height();

        std::vector<TextLineInfo> lineInfos;
        _getMultiFontTextLineInfos( lineInfos, maxWidth, maxFontHeight );

        if ( lineInfos.empty() ) {
            return;
        }

        // One line can contain text with a different font. Calculate the width of each line.
        std::vector<int32_t> lineWidths;
        lineWidths.reserve( lineInfos.size() );

        int32_t offsetY = 0;

        for ( const TextLineInfo & info : lineInfos ) {
            if ( lineWidths.empty() || offsetY != info.offsetY ) {
                lineWidths.push_back( info.lineWidth );
                offsetY = info.offsetY;
            }
            else {
                lineWidths.back() = info.lineWidth;
            }
        }

        auto widthIter = lineWidths.cbegin();
        auto infoIter = lineInfos.cbegin();

        for ( const Text & singleText : _texts ) {
            const auto languageSwitcher = getLanguageSwitcher( singleText );
            const uint8_t * data = reinterpret_cast<const uint8_t *>( singleText._text.data() );

            const uint8_t * dataEnd = data + singleText._text.size();

            const fheroes2::FontCharHandler charHandler( singleText._fontType );

            while ( data < dataEnd ) {
                if ( infoIter->characterCount > 0 ) {
                    const int32_t offsetX = x + ( maxWidth > 0 ? ( maxWidth - *widthIter ) / 2 : 0 );

                    renderSingleLine( data, infoIter->characterCount, offsetX + infoIter->offsetX, y + infoIter->offsetY, output, imageRoi, charHandler );
                }

                data += infoIter->characterCount;

                ++widthIter;
                ++infoIter;
            }

            --widthIter;
        }
    }

    std::string MultiFontText::text() const
    {
        std::string output;

        for ( const Text & singleText : _texts ) {
            output += singleText.text();
        }

        return output;
    }

    void MultiFontText::_getMultiFontTextLineInfos( std::vector<TextLineInfo> & textLineInfos, const int32_t maxWidth, const int32_t rowHeight ) const
    {
        const size_t textsCount = _texts.size();
        for ( size_t i = 0; i < textsCount; ++i ) {
            const auto languageSwitcher = getLanguageSwitcher( _texts[i] );

            // To properly render a multi-font text we must not ignore spaces at the end of a text entry which is not the last one.
            const bool isNotLastTextEntry = ( i != textsCount - 1 );

            _texts[i].getTextLineInfos( textLineInfos, maxWidth, rowHeight, isNotLastTextEntry );
        }
    }

    FontCharHandler::FontCharHandler( const FontType fontType )
        : _fontType( fontType )
        , _charLimit( getCharacterLimit( fontType.size ) )
        , _spaceCharWidth( _getSpaceCharWidth() )
    {
        // Do nothing.
    }

    bool FontCharHandler::isAvailable( const uint8_t character ) const
    {
        return ( isSpaceChar( character ) || _isValid( character ) || isLineSeparator( character ) );
    }

    const Sprite & FontCharHandler::getSprite( const uint8_t character ) const
    {
        // Display '?' in place of the invalid character.
        return getChar( _isValid( character ) ? character : invalidChar, _fontType );
    }

    int32_t FontCharHandler::getWidth( const uint8_t character ) const
    {
        if ( isSpaceChar( character ) ) {
            return _spaceCharWidth;
        }

        const Sprite & image = getSprite( character );

        assert( ( _fontType.size != FontSize::BUTTON_RELEASED && _fontType.size != FontSize::BUTTON_PRESSED && image.x() >= 0 ) || image.x() < 0 );

        return image.x() + image.width();
    }

    bool FontCharHandler::_isValid( const uint8_t character ) const
    {
        return character >= 0x21 && character <= _charLimit;
    }

    int32_t FontCharHandler::_getSpaceCharWidth() const
    {
        switch ( _fontType.size ) {
        case FontSize::SMALL:
            return 4;
        case FontSize::NORMAL:
            return 6;
        case FontSize::LARGE:
        case FontSize::BUTTON_RELEASED:
        case FontSize::BUTTON_PRESSED:
            return 8;
        default:
            // Did you add a new font size? Please add implementation.
            assert( 0 );
            break;
        }

        return 0;
    }

    bool isFontAvailable( const std::string_view text, const FontType fontType )
    {
        if ( text.empty() ) {
            return true;
        }

        const FontCharHandler charHandler( fontType );

        return std::all_of( text.begin(), text.end(), [&charHandler]( const int8_t character ) { return charHandler.isAvailable( character ); } );
    }
}
