#!/usr/bin/env python3
"""
generate_placeholder_assets.py
Generates placeholder PNG assets for DustCrate.
These are dark-warm-themed placeholders that a designer can replace.

Usage:
    python3 scripts/generate_placeholder_assets.py
"""

import os
from PIL import Image, ImageDraw, ImageFont

ASSETS_DIR = os.path.join(os.path.dirname(__file__), "..", "assets", "images")
os.makedirs(ASSETS_DIR, exist_ok=True)

# ── Palette ────────────────────────────────────────────────────────────────────
DARK_BG      = (26, 22, 18)       # #1a1612
DARK_BG2     = (42, 36, 32)       # #2a2420
AMBER        = (200, 146, 26)     # #c8921a
AMBER_DIM    = (107, 74, 10)      # #6b4a0a
TEXT_PRI     = (228, 220, 200)    # #e4dcc8
TEXT_SEC     = (106, 98, 88)      # #6a6258
PANEL_BORDER = (42, 44, 47)       # #2a2c2f


def _gradient_h(draw, x0, y0, x1, y1, c0, c1):
    """Draw a horizontal gradient rectangle."""
    w = x1 - x0
    if w <= 0:
        return
    for ix in range(w):
        t = ix / max(w - 1, 1)
        r = int(c0[0] + t * (c1[0] - c0[0]))
        g = int(c0[1] + t * (c1[1] - c0[1]))
        b = int(c0[2] + t * (c1[2] - c0[2]))
        draw.line([(x0 + ix, y0), (x0 + ix, y1 - 1)], fill=(r, g, b))


def _try_font(size, bold=False):
    """Return a best-effort font (falls back to default if none available)."""
    candidates = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
    ]
    for path in candidates:
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except Exception:
                pass
    return ImageFont.load_default()


# ── 1. dustcrate_logo.png ── 200×60 ──────────────────────────────────────────
def make_logo():
    img = Image.new("RGB", (200, 60), DARK_BG)
    draw = ImageDraw.Draw(img)

    # Subtle vinyl groove circles — outline uses 3-tuple (RGB image, no alpha channel)
    for r in range(22, 14, -2):
        draw.ellipse((8 - r + 22, 30 - r, 8 + r + 22, 30 + r),
                     outline=DARK_BG2, width=1)
    draw.ellipse((22, 26, 30, 34), fill=DARK_BG)
    draw.ellipse((25, 29, 27, 31), fill=AMBER)

    # Amber accent bar
    draw.rectangle([0, 57, 200, 59], fill=AMBER_DIM)

    # "DUSTCRATE" text
    font = _try_font(20, bold=True)
    draw.text((50, 12), "DUSTCRATE", fill=AMBER, font=font)

    # Tagline
    font_sm = _try_font(9)
    draw.text((50, 37), "dig deeper", fill=TEXT_SEC, font=font_sm)

    img.save(os.path.join(ASSETS_DIR, "dustcrate_logo.png"))
    print("  dustcrate_logo.png  (200×60)")


# ── 2. dustcrate_header_banner.png ── 800×80 ─────────────────────────────────
def make_header_banner():
    img = Image.new("RGB", (800, 80), DARK_BG)
    draw = ImageDraw.Draw(img)

    # Horizontal gradient: dark-warm left to slightly lighter centre to dark right
    _gradient_h(draw, 0, 0, 400, 80, DARK_BG, DARK_BG2)
    _gradient_h(draw, 400, 0, 800, 80, DARK_BG2, DARK_BG)

    # Grain texture (sparse dots)
    import random
    rng = random.Random(42)
    for _ in range(1200):
        gx = rng.randint(0, 799)
        gy = rng.randint(0, 79)
        v = rng.randint(20, 35)
        draw.point((gx, gy), fill=(v, v, v))

    # Amber bottom accent line
    draw.rectangle([0, 78, 800, 79], fill=AMBER_DIM)

    # Logo mark (small crate icon) — outline uses 3-tuple (RGB image)
    crate_x = 14
    draw.rectangle([crate_x, 22, crate_x + 36, 58], outline=AMBER_DIM, width=1)
    draw.line([(crate_x, 36), (crate_x + 36, 36)], fill=AMBER_DIM, width=1)
    draw.line([(crate_x + 18, 22), (crate_x + 18, 58)], fill=AMBER_DIM, width=1)

    # "DUSTCRATE" large text
    font_lg = _try_font(26, bold=True)
    draw.text((62, 16), "DUSTCRATE", fill=AMBER, font=font_lg)

    # Tagline
    font_sm = _try_font(10)
    draw.text((64, 50), "dig deeper  ·  MPC sample companion", fill=TEXT_SEC, font=font_sm)

    img.save(os.path.join(ASSETS_DIR, "dustcrate_header_banner.png"))
    print("  dustcrate_header_banner.png  (800×80)")


# ── 3–5. Section banners ── 800×32 ───────────────────────────────────────────
def make_section_banner(filename, label):
    img = Image.new("RGB", (800, 32), DARK_BG2)
    draw = ImageDraw.Draw(img)

    # Left amber accent bar
    draw.rectangle([0, 0, 2, 32], fill=AMBER)

    # Top + bottom hairlines
    draw.rectangle([0, 0, 800, 0], fill=PANEL_BORDER)
    draw.rectangle([0, 31, 800, 31], fill=PANEL_BORDER)

    # Small icon dot — outline uses 3-tuple (RGB image)
    draw.ellipse([10, 13, 18, 21], outline=AMBER, width=1)

    # Label text
    font = _try_font(11, bold=True)
    draw.text((26, 9), label, fill=TEXT_PRI, font=font)

    img.save(os.path.join(ASSETS_DIR, filename))
    print(f"  {filename}  (800×32)")


def main():
    print("Generating DustCrate placeholder assets …")
    make_logo()
    make_header_banner()
    make_section_banner("section_browser_banner.png",   "SAMPLE BROWSER")
    make_section_banner("section_keys_banner.png",      "CHROMATIC TRIGGER")
    make_section_banner("section_character_banner.png", "CHARACTER")
    print("Done. Files written to assets/images/")


if __name__ == "__main__":
    main()
