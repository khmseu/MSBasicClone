# Font Auto-Fetching

This document describes the automatic font downloading feature in MSBasic.

## Overview

MSBasic automatically downloads the Ultimate Apple II Font and its charset map during the CMake configuration phase. This ensures that authentic Apple II text rendering is available without manual setup.

## How It Works

### Build-Time Download

When you run `cmake` to configure the build, the `cmake/FetchFont.cmake` module automatically:

1. **Checks for existing files**: If font files already exist in `assets/fonts/`, they are not re-downloaded
2. **Creates fonts directory**: Creates `assets/fonts/` if needed
3. **Downloads font**: Attempts to download `PrintChar21.ttf` from kreativekorp.com
4. **Downloads charset map**: Attempts to download the Apple II charset map from the same source
5. **Graceful fallback**: If downloads fail, the build continues with warnings and Raylib's default font is used as fallback
6. **Sets availability flag**: Sets `APPLE2_FONT_AVAILABLE` cache variable for other build components

### Files Downloaded

- `assets/fonts/PrintChar21.ttf` - The Ultimate Apple II Font (TrueType format, 7×8 pixels per character)
- `assets/fonts/apple2-charset.html` - Apple II character set reference map (developer documentation)

### URLs Used

- Font: `https://www.kreativekorp.com/swdownload/fonts/apple2/PrintChar21.ttf`
- Charset Map: `https://www.kreativekorp.com/charset/map/apple2/`

Both URLs can be overridden via CMake variables:

```bash
cmake -DAPPLE2_FONT_URL="https://alternative-source/font.ttf" \
      -DAPPLE2_CHARSET_URL="https://alternative-source/map.html" ..
```

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

- **Avoids repeated downloads** in CI runs (important for frequent PRs and commits)
- **Is invalidated only when** `FetchFont.cmake` changes (stable key)
- **Restores from previous runs** via fallback key prefix `apple2-fonts-`
- **Speeds up CI build times** significantly (font file is ~50KB, charset map is ~20KB)
- **Works across workflow runs** without manual intervention

## Manual Installation

If automatic download fails (e.g., due to network restrictions or blocked domains), you can manually install the fonts:

1. Download from: <https://www.kreativekorp.com/software/fonts/apple2/>
2. Save `PrintChar21.ttf` to `assets/fonts/PrintChar21.ttf`
3. Optionally download charset map from: <https://www.kreativekorp.com/charset/map/apple2/>
4. Save as `assets/fonts/apple2-charset.html`
5. Re-run `cmake` - it will detect the existing files

## Font Specifications

- **Font Name**: The Ultimate Apple II Font (PrintChar21)
- **Character Cell**: 7×8 pixels per character
- **Format**: TrueType (.ttf)
- **Text Mode 40-column**: 280 pixels wide (40 chars × 7 pixels)
- **Text Mode 80-column**: 560 pixels effective (80 chars × 7 pixels, scaled 0.5x to fit 280 pixels)
- **Screen Height**: 192 pixels (24 rows × 8 pixels)
- **Usage**: Text rendering in 40×24 and 80×24 modes
- **License**: Free License (see `FreeLicense.txt` from kreativekorp.com)

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
4. Use an alternative font source by setting CMake variables:

   ```bash
   cmake -DAPPLE2_FONT_URL="https://alternative-source.com/PrintChar21.ttf" ..
   ```

5. The build will continue without the font - graphics mode will use Raylib's default font

### Font Not Loading

The font loading is currently a TODO in the codebase. The infrastructure is in place, but actual font rasterization is not yet implemented in `GraphicsRenderer::loadApple2Font()`.

## Implementation Details

### CMake Module

The `cmake/FetchFont.cmake` module provides the `fetch_apple2_font()` function with the following features:

- **Uses CMake's `file(DOWNLOAD ...)` command** for native, cross-platform download support (no external tools needed)
- **30-second timeout per download** to prevent hanging on slow/blocked connections
- **TLS/SSL certificate verification** enabled for security
- **Smart cleanup** - removes partial downloads on failure to save space
- **Idempotent** - skips re-downloading if files already exist (safe for repeated builds)
- **Sets `APPLE2_FONT_AVAILABLE` cache variable** - other build components can check this
- **Supports configurable URLs** via `APPLE2_FONT_URL` and `APPLE2_CHARSET_URL` CMake variables
- **Helpful error messages** - guides users to manual installation and alternative sources

### Invocation

The module is automatically included and called during CMake configuration:

```cmake
# In CMakeLists.txt
include(cmake/FetchFont.cmake)
```

The `fetch_apple2_font()` function is called automatically and doesn't require manual invocation.

### Configurable URLs

You can override the default font source URLs when running CMake:

```bash
cmake -DAPPLE2_FONT_URL="https://mirror.example.com/PrintChar21.ttf" \
      -DAPPLE2_CHARSET_URL="https://mirror.example.com/apple2-charset.html" \
      -S . -B build
```

This is useful when:

- The default source is blocked or unavailable
- You have a local mirror or cache
- Testing with alternative font files

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
