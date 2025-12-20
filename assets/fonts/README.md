# Font Files Directory

This directory contains font files for text rendering in graphics mode.

## Ultimate Apple II Font

The Ultimate Apple II Font is **automatically downloaded** during the CMake build process.

### Automatic Download

The build system will attempt to download:
1. `PrintChar21.ttf` - The Ultimate Apple II Font
2. `apple2-charset.html` - The Apple II character set map

Both files are fetched from https://www.kreativekorp.com/ during the CMake configuration phase.

### Manual Download (if automatic fails)

If the automatic download fails (e.g., due to network restrictions), you can manually:

1. Visit: https://www.kreativekorp.com/software/fonts/apple2/
2. Download the font file `PrintChar21.ttf`
3. Place it in this directory
4. Optionally download the charset map from: https://www.kreativekorp.com/charset/map/apple2/
5. Save it as `apple2-charset.html` in this directory

### Font Specifications

- **Character Cell**: 7×8 pixels
- **Format**: TrueType (.ttf)
- **Usage**: Text rendering in 40×24 and 80×24 modes

### License

The Ultimate Apple II Font is created by kreativekorp.com. Please review the license terms on their website before distribution.

## Build Integration

The CMakeLists.txt automatically downloads font files using the `FetchFont.cmake` module. Font loading is optional - if no font file is found, Raylib's default font will be used as a fallback.

## CI/CD Caching

In GitHub Actions CI, the fonts are cached to avoid repeated downloads. The cache key is based on the `FetchFont.cmake` script content.

