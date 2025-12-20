# Ultimate Apple II Font Integration Plan

## Font Source

- **Name**: The Ultimate Apple II Font
- **URL**: <https://www.kreativekorp.com/software/fonts/apple2/>
- **Character Set Map**: <https://www.kreativekorp.com/charset/map/apple2/>

## Font Specifications

- **Character Cell Size**: 7×8 pixels
- **Text Mode 40-column**: 280 pixels wide (40 chars × 7 pixels)
- **Text Mode 80-column**: 560 pixels effective (80 chars × 7 pixels, scaled horizontally to fit 280 pixels)
- **Screen Height**: 192 pixels (24 rows × 8 pixels)

## Implementation Strategy

### 1. Font File Storage

The font file is **automatically downloaded** during the CMake build process:

- **Target location**: `assets/fonts/PrintChar21.ttf` (TrueType format, 7×8 pixels per character)
- **Charset map**: `assets/fonts/apple2-charset.html` (developer reference)
- **License**: `assets/fonts/FreeLicense.txt` (auto-downloaded from kreativekorp.com)
- **Module**: The `cmake/FetchFont.cmake` module handles downloading
- **Smart caching**: Files are cached in CI to avoid repeated downloads on every build
- **Idempotent**: If files exist locally, they are never re-downloaded
- **Graceful fallback**: If downloads fail (network issues, blocked URLs, etc.), the build continues without error using Raylib's default font

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

- [x] Add font auto-fetching to CMake build (via `cmake/FetchFont.cmake` module)
- [x] Download charset map together with font file during configuration
- [x] Download license file (`FreeLicense.txt`) automatically
- [x] Add CI caching for downloaded fonts in GitHub Actions (`.github/workflows/build.yml`)
- [x] Idempotent downloads (skip if files already present)
- [x] Graceful error handling with helpful fallback messages
- [x] Configurable URLs via CMake variables (`APPLE2_FONT_URL`, `APPLE2_CHARSET_URL`)
- [ ] Implement font loading in `GraphicsRenderer::loadApple2Font()` (currently stubbed)
- [ ] Update `drawChar()` to use loaded font instead of Raylib default
- [ ] Update `drawText()` to use loaded font for text mode rendering
- [ ] Add text mode scaling logic for 80-column mode (0.5x horizontal scaling)
- [ ] Test rendering in 40-column mode (7×8 pixels per char)
- [ ] Test rendering in 80-column mode (with horizontal compression)

## License Considerations

The Ultimate Apple II Font uses the **Free License** from kreativekorp.com:

- **License File**: Automatically downloaded to `assets/fonts/FreeLicense.txt`
- **URL**: <https://www.kreativekorp.com/software/fonts/FreeLicense.txt>
- **Usage**: Free to use with appropriate attribution
- **Attribution**: Please see the license file for specific requirements

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
