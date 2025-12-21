# Font Files Directory

This directory contains font files for text rendering in graphics mode.

## Ultimate Apple II Font

The Ultimate Apple II Font and related files are **bundled in this repository** for reliable builds.

### Bundled Files

The following files are included in the repository:

1. `PrintChar21.ttf` - The Ultimate Apple II Font
2. `charset/` - Mousetext character set map directory with index.html and PNG images
3. `FreeLicense.txt` - Font license from Kreative Korporation

All files are sourced from <https://www.kreativekorp.com/>

### Refreshing Bundled Files

To update the bundled files from upstream sources:

```bash
cmake -S . -B build -DREFRESH_BUNDLED_FONTS=ON
```

This will re-download:

- The font package from <https://www.kreativekorp.com/swdownload/fonts/retro/pr.zip>
- The mousetext charset map from <https://www.kreativekorp.com/charset/map/mousetext/>

### Manual Update (if automatic refresh fails)

If the automatic refresh fails (e.g., due to network restrictions), you can manually:

1. Visit: <https://www.kreativekorp.com/software/fonts/apple2/>
2. Download the "Print Char 21" font package
3. Extract `PrintChar21.ttf` and `FreeLicense.txt` to this directory
4. Download mousetext charset with wget:

   ```bash
   cd assets/fonts
   mkdir -p charset
   cd charset
   wget -p -np -nd --include-directories=/charset/map/mousetext/ -e robots=off https://www.kreativekorp.com/charset/map/mousetext/
   ```

   Note: The `-e robots=off` flag is required because the server blocks recursive downloads via robots.txt

### Font Specifications

- **Character Cell**: 7×8 pixels
- **Format**: TrueType (.ttf)
- **Usage**: Text rendering in 40×24 and 80×24 modes

### License

The Ultimate Apple II Font is created by Kreative Korporation. See `FreeLicense.txt` for the complete license terms. The font is free for use and redistribution under the Kreative Software Relay Fonts Free Use License.

## Build Integration

The CMakeLists.txt checks for bundled font files using the `FetchFont.cmake` module. Font loading is optional - if no font file is found, Raylib's default font will be used as a fallback.

### Configuration Options

- `REFRESH_BUNDLED_FONTS`: Set to `ON` to force refresh of bundled files from upstream
- `APPLE2_FONT_PACKAGE_URL`: Override the default font package download URL
- `APPLE2_CHARSET_URL`: Override the default charset map download URL

## Version Control

These font files are tracked in git to ensure:

- Reliable builds without network dependencies
- Consistent font rendering across all environments
- No CI/CD download failures
