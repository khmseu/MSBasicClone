# Font Auto-Fetching

This document describes the automatic font downloading feature in MSBasic.

## Overview

MSBasic automatically downloads the Ultimate Apple II Font and its charset map during the CMake configuration phase. This ensures that authentic Apple II text rendering is available without manual setup.

## How It Works

### Build-Time Download

When you run `cmake` to configure the build, the `cmake/FetchFont.cmake` module:

1. **Checks for existing files**: If font files already exist in `assets/fonts/`, they are not re-downloaded
2. **Downloads font**: Attempts to download `PrintChar21.ttf` from kreativekorp.com
3. **Downloads charset map**: Attempts to download the Apple II charset map
4. **Graceful fallback**: If downloads fail, the build continues with warnings, and Raylib's default font is used

### Files Downloaded

- `assets/fonts/PrintChar21.ttf` - The Ultimate Apple II Font (TrueType format)
- `assets/fonts/apple2-charset.html` - Apple II character set reference map

## CI/CD Integration

### GitHub Actions Caching

The GitHub Actions workflow (`.github/workflows/build.yml`) includes caching for downloaded fonts:

```yaml
- name: Cache Apple II fonts
  uses: actions/cache@v4
  with:
      path: assets/fonts
      key: apple2-fonts-${{ hashFiles('cmake/FetchFont.cmake') }}
      restore-keys: |
          apple2-fonts-
```

This cache:
- Avoids repeated downloads in CI runs
- Is invalidated only when `FetchFont.cmake` changes
- Speeds up CI build times

## Manual Installation

If automatic download fails (e.g., due to network restrictions or blocked domains), you can manually install the fonts:

1. Download from: https://www.kreativekorp.com/software/fonts/apple2/
2. Save `PrintChar21.ttf` to `assets/fonts/PrintChar21.ttf`
3. Optionally download charset map from: https://www.kreativekorp.com/charset/map/apple2/
4. Save as `assets/fonts/apple2-charset.html`
5. Re-run `cmake` - it will detect the existing files

## Font Specifications

- **Font Name**: The Ultimate Apple II Font (PrintChar21)
- **Character Cell**: 7×8 pixels
- **Format**: TrueType (.ttf)
- **Usage**: Text rendering in 40×24 and 80×24 modes
- **License**: Created by kreativekorp.com - see their website for license terms

## Troubleshooting

### Download Fails

If you see warnings like:
```
CMake Warning: Failed to download font: "Could not resolve hostname"
```

**Solutions**:
1. Check your network connection
2. Try manually downloading the font (see Manual Installation above)
3. If kreativekorp.com is blocked, contact your network administrator
4. The build will continue without the font - graphics mode will use Raylib's default font

### Font Not Loading

The font loading is currently a TODO in the codebase. The infrastructure is in place, but actual font rasterization is not yet implemented in `GraphicsRenderer::loadApple2Font()`.

## Implementation Details

### CMake Module

The `cmake/FetchFont.cmake` module provides the `fetch_apple2_font()` function:

- Uses CMake's `file(DOWNLOAD ...)` command
- Sets 30-second timeout for downloads
- Verifies TLS/SSL certificates
- Cleans up partial downloads on failure
- Sets `APPLE2_FONT_AVAILABLE` cache variable

### Integration Points

1. **CMakeLists.txt**: Calls `fetch_apple2_font()` during configuration
2. **GraphicsRenderer**: Checks for font files at multiple paths
3. **.gitignore**: Excludes downloaded fonts from version control

## Future Enhancements

- [ ] Implement actual font loading in GraphicsRenderer
- [ ] Support alternative font sources/mirrors
- [ ] Add option to disable auto-fetching
- [ ] Implement offline/embedded font fallback
- [ ] Add font file integrity checks (checksum/hash)
