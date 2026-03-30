# Bad Mofo Font

Download `BadMofo.ttf` and place it in this directory before building.

**Bad Mofo** is a bold, retro-futuristic graffiti typeface by Christopher Hansen (Livin Hell).

## License
**Free for commercial use** — no purchase required!

## Download
- https://www.1001fonts.com/bad-mofo-font.html
- https://www.dafont.com/bad-mofo.font
- https://www.fontspace.com/bad-mofo-font-f5276

## Instructions
1. Download `BadMofo.ttf` from one of the links above
2. Place it in this directory as `assets/fonts/BadMofo.ttf`
3. Run CMake to compile the font into the plugin binary:
   ```bash
   mkdir build && cd build
   cmake .. && cmake --build .
   ```

The font will be embedded via `juce_add_binary_data` and loaded at runtime for the "DUSTCRATE" header.
