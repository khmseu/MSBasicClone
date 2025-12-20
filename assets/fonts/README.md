# Font Files Directory

This directory contains font files for text rendering in graphics mode.

## Ultimate Apple II Font

The Ultimate Apple II Font should be placed here for authentic Apple II text rendering.

### Download Instructions

1. Visit: https://www.kreativekorp.com/software/fonts/apple2/
2. Download the font file (e.g., `PrintChar21.ttf`)
3. Place it in this directory

### Font Specifications

- **Character Cell**: 7×8 pixels
- **Format**: TrueType (.ttf) or bitmap atlas
- **Usage**: Text rendering in 40×24 and 80×24 modes

### License

The Ultimate Apple II Font is created by kreativekorp.com. Please review the license terms on their website before distribution.

## Build Integration

The CMakeLists.txt will automatically include font files from this directory if they are present. Font loading is optional - if no font file is found, Raylib's default font will be used as a fallback.
