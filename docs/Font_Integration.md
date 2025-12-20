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
- [x] Implement font loading in `GraphicsRenderer::loadApple2Font()`
- [x] Update `drawChar()` to use loaded font instead of Raylib default
- [x] Update `drawText()` to use loaded font for text mode rendering
- [x] Add text mode scaling logic for 80-column mode (0.5x horizontal scaling)
- [ ] Test rendering in 40-column mode (7×8 pixels per char) - requires graphics environment
- [ ] Test rendering in 80-column mode (with horizontal compression) - requires graphics environment

## Implementation Details

### Font Loading (`GraphicsRenderer::loadApple2Font()`)

The font loading implementation:

1. **Font File Search**: Checks multiple paths for `PrintChar21.ttf`:
   - `assets/fonts/PrintChar21.ttf` (relative to working directory)
   - `../assets/fonts/PrintChar21.ttf` (one level up)
   - `/usr/share/fonts/apple2/PrintChar21.ttf` (system-wide location)

2. **Font Loading**: Uses Raylib's `LoadFontEx()` function:
   - Font size: 8 pixels (matching Apple II 7×8 character cell)
   - Character set: Default ASCII (nullptr, 0 parameters)
   - Texture filter: `TEXTURE_FILTER_BILINEAR` for smooth scaling

3. **Error Handling**: 
   - Validates font texture ID after loading
   - Graceful fallback to Raylib's default font on failure
   - Comprehensive error messages guide users to manual installation

4. **Memory Management**:
   - Font stored as `Font*` pointer (allows optional loading)
   - Properly unloaded in `shutdown()` method
   - Protected by `fontLoaded_` flag to prevent invalid access

### Character Rendering (`drawChar()`)

Single character rendering implementation:

1. **Font Selection**: Uses Apple II font if loaded, otherwise falls back to default
2. **Scaling**: Applies `config_.scaleFactor` for window scaling (e.g., 2x → 16 pixels)
3. **Text Mode Handling**:
   - 40-column mode: Normal 7-pixel character width
   - 80-column mode: 0.5x horizontal scaling (3.5-pixel effective width)
4. **Rendering**: Uses `DrawTextEx()` for consistent font rendering

### Text Rendering (`drawText()`)

Multi-character text rendering implementation:

1. **40-Column Mode** (default):
   - Direct rendering with `DrawTextEx()`
   - Character spacing: 1 pixel × scaleFactor
   - Character width: 7 pixels × scaleFactor

2. **80-Column Mode** (horizontal compression):
   - Character-by-character rendering loop
   - X-position adjusted by `7 × scaleFactor × 0.5` per character
   - Effective character width: 3.5 pixels (7 × 0.5)
   - Results in 80 characters fitting in 280 pixels

3. **Color Handling**: RGB color values extracted from 24-bit integer
4. **Font Fallback**: Uses Raylib default font if Apple II font unavailable

### Text Mode Detection

- Uses `config_.textMode` from `GraphicsConfig`
- `TextMode::Text40`: Standard 40×24 text mode (7-pixel chars)
- `TextMode::Text80`: 80×24 text mode with horizontal compression (3.5-pixel effective width)

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
