# Ultimate Apple II Font Integration Plan

## Font Source
- **Name**: The Ultimate Apple II Font
- **URL**: https://www.kreativekorp.com/software/fonts/apple2/
- **Character Set Map**: https://www.kreativekorp.com/charset/map/apple2/

## Font Specifications
- **Character Cell Size**: 7×8 pixels
- **Text Mode 40-column**: 280 pixels wide (40 chars × 7 pixels)
- **Text Mode 80-column**: 560 pixels effective (80 chars × 7 pixels, scaled horizontally to fit 280 pixels)
- **Screen Height**: 192 pixels (24 rows × 8 pixels)

## Implementation Strategy

### 1. Font File Storage
The font file is **automatically downloaded** during the CMake build process:
- Target location: `assets/fonts/PrintChar21.ttf` (TrueType format)
- Charset map: `assets/fonts/apple2-charset.html`
- The `cmake/FetchFont.cmake` module handles the download
- Files are cached in CI to avoid repeated downloads

### 2. Font Loading in GraphicsRenderer
- Load font file when Raylib is available
- Use `LoadFontEx()` to load at specific size (7×8 or larger)
- Store as member variable in `GraphicsRenderer`

### 3. Character Rendering
- Implement character-by-character rendering in text mode
- Apply scaling for 80-column mode (horizontal compression)
- Handle special characters (inverse, flash, control codes)

### 4. Text Mode Support
- 40-column mode: Direct 7×8 pixel rendering
- 80-column mode: Scale horizontally by 0.5x or use smaller font

## Current Implementation Status
- [x] Add font auto-fetching to CMake build (via FetchFont.cmake module)
- [x] Download charset map together with font file
- [x] Add CI caching for downloaded fonts
- [ ] Implement font loading in GraphicsRenderer::loadApple2Font()
- [ ] Update drawChar() to use loaded font
- [ ] Update drawText() to use loaded font
- [ ] Add text mode scaling logic
- [ ] Test rendering in 40-column mode
- [ ] Test rendering in 80-column mode

## License Considerations
The Ultimate Apple II Font is free to use. Need to verify license and attribution requirements from kreativekorp.com website.

## Alternative Approach
If direct font integration is complex, consider:
1. Using a pre-rendered bitmap atlas (256 characters in a grid)
2. Store as PNG or embedded C array
3. More control over pixel-perfect rendering
4. Faster rendering without font rasterization overhead

## Testing
- Test character rendering in both text modes
- Verify 7×8 pixel alignment
- Test special characters (inverse, flash)
- Compare output with actual Apple II display
