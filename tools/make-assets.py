#!/usr/bin/env python3
"""
make-assets.py - gera os assets originais do jogo (CC0 / dominio publico).

Substitui as artes de terceiros (Blizzard, Kryptid etc.) por versoes proprias,
mantendo exatamente os nomes, tamanhos e layouts esperados pelo codigo:
  img/fundo.png           800x600      (fundo do jogo)
  img/stateMenu.jpg       800x600      (tela de menu)
  img/stateWin.jpg        800x600      (tela de vitoria)
  img/stateLose.jpg       800x600      (tela de derrota)
  img/ufo.png             32x32        (UFO do FollowerObject)
  img/PlanetaVermelho.png 44x44        (planetas vermelhos)
  img/Lua.png             16x16        (lua que orbita a Terra)
  img/NaveSheet.png       376x100      (4 quadros de 94x100 - nave)
  img/BoomSheet.png       760x100      (8 quadros de 95x100 - explosao)
  img/Tileset.png         450x450      (grid 6x5 de tiles 75x75)
  img/tileset/SpaceTile-XX.png         (tiles individuais, usados no EX02)
  sfx/boom.wav            efeito de explosao sintetizado

Uso: python tools/make-assets.py <pasta-de-saida>
Sem dependencias alem de Pillow. Sem IA generativa: tudo desenhado/sintetizado aqui.
"""

import math
import os
import random
import struct
import sys
import wave

from PIL import Image, ImageDraw, ImageFilter

try:
    LANCZOS = Image.Resampling.LANCZOS
except AttributeError:  # Pillow antigo
    LANCZOS = Image.LANCZOS

SS = 4  # supersampling para bordas suaves


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def clamp(v):
    return 0 if v < 0 else (255 if v > 255 else int(v))


# ---------------------------------------------------------------- fundo/telas

def starfield(w, h, seed, top, bottom, nebulae, star_count, planet=None):
    """Fundo espacial: gradiente vertical, nebulosas desfocadas e estrelas."""
    rnd = random.Random(seed)
    img = Image.new("RGBA", (w, h), (0, 0, 0, 255))
    d = ImageDraw.Draw(img)
    for y in range(h):
        d.line([(0, y), (w, y)], fill=lerp(top, bottom, y / h) + (255,))

    # nebulosas: elipses coloridas desfocadas
    neb = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    nd = ImageDraw.Draw(neb)
    for (color, alpha, count, rmin, rmax) in nebulae:
        for _ in range(count):
            cx = rnd.uniform(0, w)
            cy = rnd.uniform(0, h)
            rx = rnd.uniform(rmin, rmax)
            ry = rx * rnd.uniform(0.5, 1.0)
            nd.ellipse([cx - rx, cy - ry, cx + rx, cy + ry], fill=color + (alpha,))
    neb = neb.filter(ImageFilter.GaussianBlur(45))
    img = Image.alpha_composite(img, neb)

    # estrelas
    d = ImageDraw.Draw(img)
    for _ in range(star_count):
        x = rnd.randrange(w)
        y = rnd.randrange(h)
        r = rnd.random()
        if r < 0.85:
            a = rnd.randint(90, 180)
            d.point((x, y), fill=(255, 255, 255, a))
        elif r < 0.97:
            a = rnd.randint(180, 235)
            rr = rnd.choice([1, 1, 2])
            d.ellipse([x - rr, y - rr, x + rr, y + rr], fill=(255, 255, 255, a))
        else:
            # estrela com brilho
            glow = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            gd = ImageDraw.Draw(glow)
            gd.ellipse([x - 6, y - 6, x + 6, y + 6], fill=(180, 210, 255, 90))
            glow = glow.filter(ImageFilter.GaussianBlur(4))
            img = Image.alpha_composite(img, glow)
            d = ImageDraw.Draw(img)
            d.ellipse([x - 2, y - 2, x + 2, y + 2], fill=(255, 255, 255, 245))

    if planet:
        pimg, px, py, scale, color = planet
        p = sphere(int(color_size(pimg) * scale), *color)
        img.alpha_composite(p, (px, py))
    return img


def color_size(spec):
    return spec  # helper apenas para clareza


def sphere(size, base, shadow, light, seed=1, bands=0.0, craters=0, glow=None):
    """Esfera sombreada (planeta/lua) com crateras/bandas opcionais."""
    rnd = random.Random(seed)
    S = size * SS
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    px = img.load()
    cr = []
    for _ in range(craters):
        ang = rnd.uniform(0, 2 * math.pi)
        rr = rnd.uniform(0.0, 0.7)
        cr.append((0.5 + math.cos(ang) * rr * 0.5 * 0.8,
                   0.5 + math.sin(ang) * rr * 0.5 * 0.8,
                   rnd.uniform(0.05, 0.14), rnd.uniform(0.55, 0.8)))
    lx, ly = -0.55, -0.65
    ln = math.hypot(lx, ly)
    lx, ly = lx / ln, ly / ln
    band_phase = rnd.uniform(0, 6.28)
    for y in range(S):
        ny = (y + 0.5) / S * 2 - 1
        for x in range(S):
            nx = (x + 0.5) / S * 2 - 1
            r2 = nx * nx + ny * ny
            if r2 > 1.0:
                continue
            nz = math.sqrt(max(0.0, 1 - r2))
            t = max(0.0, nx * (-lx) + ny * (-ly) + nz * 0.65)
            t = t ** 0.85
            if bands:
                t *= 0.82 + 0.18 * math.sin(ny * bands + band_phase)
            col = [clamp(shadow[i] + (base[i] - shadow[i]) * t) for i in range(3)]
            spec = max(0.0, t - 0.72)
            for i in range(3):
                col[i] = clamp(col[i] + light[i] * spec)
            for (cx, cy, crr, cdark) in cr:
                dr = math.hypot(nx - (cx * 2 - 1), ny - (cy * 2 - 1))
                if dr < crr:
                    f = cdark + (1 - cdark) * (dr / crr)
                    for i in range(3):
                        col[i] = clamp(col[i] * f)
            a = 255 if r2 < 0.94 else clamp(255 * (1 - (r2 - 0.94) / 0.06))
            px[x, y] = (col[0], col[1], col[2], a)
    img = img.resize((size, size), LANCZOS)
    if glow:
        g = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        gd = ImageDraw.Draw(g)
        gd.ellipse([2, 2, size - 3, size - 3], outline=glow + (70,), width=2)
        g = g.filter(ImageFilter.GaussianBlur(2))
        img = Image.alpha_composite(img, g)
    return img


# ------------------------------------------------------------------- sprites

def draw_ship(frame):
    """Nave 94x100 vista de cima (nariz para cima). frame 2 = boost."""
    w, h = 94, 100
    img = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    def P(pts):
        return [(x * SS, y * SS) for (x, y) in pts]

    outline = (28, 32, 40, 255)
    body = (216, 222, 230, 255)
    wing = (168, 178, 192, 255)
    wing2 = (140, 150, 166, 255)

    # asas
    d.polygon(P([(36, 46), (8, 82), (16, 92), (34, 76)]), fill=wing, outline=outline)
    d.polygon(P([(58, 46), (86, 82), (78, 92), (60, 76)]), fill=wing2, outline=outline)
    # corpo
    d.polygon(P([(47, 3), (58, 26), (61, 62), (56, 93), (38, 93), (33, 62), (36, 26)]),
              fill=body, outline=outline, width=SS)
    # cockpit
    d.ellipse(P([(41, 22), (53, 44)]), fill=(70, 200, 225, 255), outline=outline)
    d.ellipse(P([(43, 25), (49, 36)]), fill=(160, 235, 245, 255))
    # bocais
    d.rectangle(P([(39, 91), (45, 99)]), fill=(60, 62, 70, 255))
    d.rectangle(P([(49, 91), (55, 99)]), fill=(60, 62, 70, 255))

    # chamas conforme o quadro (0,1,3 normais; 2 = boost grande)
    if frame == 2:
        for (x0, x1) in ((39, 45), (49, 55)):
            d.polygon(P([(x0, 99), (x1, 99), ((x0 + x1) / 2, 100)]), fill=(255, 170, 60, 255))
        flame_len = 12
    else:
        flame_len = 4 if frame in (0, 2) else (6 if frame == 1 else 3)
    for (x0, x1) in ((40, 44), (50, 54)):
        d.polygon(P([(x0, 99), (x1, 99), ((x0 + x1) / 2, 99 + flame_len)]),
                  fill=(120, 200, 255, 255))
        d.polygon(P([(x0 + 1, 99), (x1 - 1, 99), ((x0 + x1) / 2, 99 + flame_len * 0.6)]),
                  fill=(240, 250, 255, 255))
    return img.resize((w, h), LANCZOS)


def draw_ship_sheet():
    sheet = Image.new("RGBA", (376, 100), (0, 0, 0, 0))
    for i in range(4):
        sheet.alpha_composite(draw_ship(i), (i * 94, 0))
    return sheet


def draw_boom_frame(i, n=8):
    """Quadros de explosao 95x100: flash, nucleo, faiscas e fumaca."""
    w, h = 95, 100
    p = (i + 1) / n
    img = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 47.5 * SS, 50 * SS
    rnd = random.Random(100 + i)

    # flash inicial (primeiros quadros)
    if i < 2:
        fr = (18 if i == 0 else 12) * SS
        d.ellipse([cx - fr, cy - fr, cx + fr, cy + fr], fill=(255, 255, 245, 235))

    # nucleo: circulos concentricos (branco -> amarelo -> laranja -> vermelho)
    r0 = (8 + 34 * (p if p < 0.6 else 0.6 + (1 - p) * 0.5)) * SS
    steps = int(r0 / SS) + 1
    for s in range(steps, 0, -1):
        f = s / steps
        col = lerp((255, 250, 220), (255, 90, 20), f)
        a = clamp((1 - p) * 255 * (1 - f * 0.25))
        d.ellipse([cx - s * SS, cy - s * SS, cx + s * SS, cy + s * SS], fill=col + (a,))

    # faiscas irradiando (linhas fortes)
    for _ in range(34):
        ang = rnd.uniform(0, 2 * math.pi)
        rr = (8 + 42 * p) * rnd.uniform(0.45, 1.0)
        x1 = cx + math.cos(ang) * rr * 0.5
        y1 = cy + math.sin(ang) * rr * 0.5
        x2 = cx + math.cos(ang) * rr
        y2 = cy + math.sin(ang) * rr
        a = clamp(255 * (1 - p) * rnd.uniform(0.55, 1.0))
        d.line([x1, y1, x2, y2], fill=(255, 180, 80, a), width=int(SS * 1.5))

    # fumaca aparece e some
    if p > 0.3:
        smoke = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
        sd = ImageDraw.Draw(smoke)
        a = clamp(150 * (1 - p) + 40)
        for _ in range(7):
            ang = rnd.uniform(0, 2 * math.pi)
            rr = (10 + 30 * p) * rnd.uniform(0.3, 1.0)
            x = cx + math.cos(ang) * rr
            y = cy + math.sin(ang) * rr
            r = rnd.uniform(6, 13) * SS
            sd.ellipse([x - r, y - r, x + r, y + r], fill=(110, 108, 100, a))
        smoke = smoke.filter(ImageFilter.GaussianBlur(5))
        img = Image.alpha_composite(img, smoke)
    return img.resize((w, h), LANCZOS)


def draw_boom_sheet():
    sheet = Image.new("RGBA", (760, 100), (0, 0, 0, 0))
    for i in range(8):
        sheet.alpha_composite(draw_boom_frame(i), (i * 95, 0))
    return sheet


def draw_ufo():
    """Disco voador 32x32."""
    w = 32
    img = Image.new("RGBA", (w * SS, w * SS), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    def P(pts):
        return [(x * SS, y * SS) for (x, y) in pts]

    outline = (40, 46, 58, 255)
    # cúpula
    d.ellipse(P([(9, 6), (23, 20)]), fill=(150, 170, 195, 255), outline=outline)
    d.ellipse(P([(11, 7), (18, 13)]), fill=(215, 230, 245, 255))
    # corpo do disco
    d.ellipse(P([(1, 13), (31, 26)]), fill=(175, 185, 200, 255), outline=outline)
    d.ellipse(P([(3, 15), (29, 22)]), fill=(205, 214, 226, 255))
    # base
    d.ellipse(P([(8, 19), (24, 27)]), fill=(120, 130, 146, 255), outline=outline)
    # luzes
    for x in (7, 16, 25):
        d.ellipse(P([(x - 1, 17), (x + 2, 20)]), fill=(90, 220, 235, 255))
    return img.resize((w, w), LANCZOS)


# -------------------------------------------------------------------- tiles

def metal_plate(outer, detail, seed):
    """Placa metalica 75x75 (plataforma). outer: lados externos com borda."""
    S = 75 * SS
    rnd = random.Random(seed)
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    edge = (20, 23, 30, 255)
    lightline = (150, 158, 172, 255)
    darkline = (34, 38, 46, 255)

    for y in range(S):
        d.line([(0, y), (S, y)], fill=lerp((66, 72, 84), (92, 99, 112), y / S) + (255,))
    for _ in range(150):
        x = rnd.randrange(S)
        y = rnd.randrange(S)
        r, g, b, a = img.getpixel((x, y))
        v = rnd.randint(-12, 12)
        img.putpixel((x, y), (clamp(r + v), clamp(g + v), clamp(b + v), 255))

    t = 3 * SS
    if outer in ("tl", "t", "tr"):
        d.rectangle([0, 0, S - 1, t], fill=edge)
        d.line([(0, t + SS), (S, t + SS)], fill=(120, 128, 142, 160), width=SS)
    if outer in ("bl", "b", "br"):
        d.rectangle([0, S - 1 - t, S - 1, S - 1], fill=edge)
        d.line([(0, S - t - 2 * SS), (S, S - t - 2 * SS)], fill=(120, 128, 142, 120), width=SS)
    if outer in ("tl", "l", "bl"):
        d.rectangle([0, 0, t, S - 1], fill=edge)
    if outer in ("tr", "r", "br"):
        d.rectangle([S - 1 - t, 0, S - 1, S - 1], fill=edge)
    if outer == "all":
        d.rectangle([0, 0, S - 1, S - 1], outline=edge, width=t)

    m = 6 * SS
    d.rectangle([m, m, S - 1 - m, S - 1 - m], fill=(82, 88, 101, 255))
    d.line([(m, m), (S - m, m)], fill=lightline, width=SS)
    d.line([(m, m), (m, S - m)], fill=lightline, width=SS)
    d.line([(S - m, m), (S - m, S - m)], fill=darkline, width=SS)
    d.line([(m, S - m), (S - m, S - m)], fill=darkline, width=SS)

    for x, y in ((4, 4), (S - 5, 4), (4, S - 5), (S - 5, S - 5),
                 (S // 2, 4), (S // 2, S - 5), (4, S // 2), (S - 5, S // 2)):
        d.ellipse([x - 2 * SS, y - 2 * SS, x + 2 * SS, y + 2 * SS],
                  fill=(156, 162, 174, 255), outline=edge)

    if detail == "vent":
        for i in range(3):
            y0 = (24 + i * 10) * SS
            d.rectangle([22 * SS, y0, 53 * SS, y0 + 4 * SS], fill=(52, 57, 68, 255))
            d.line([(22 * SS, y0), (53 * SS, y0)], fill=lightline, width=SS)
    elif detail == "grate":
        for i in range(5):
            x0 = (14 + i * 10) * SS
            d.rectangle([x0, 16 * SS, x0 + 5 * SS, 59 * SS], fill=(50, 55, 66, 255))
            d.line([(x0, 16 * SS), (x0, 59 * SS)], fill=(110, 118, 132, 200), width=SS)
    elif detail == "hazard":
        sy = 52 * SS
        d.rectangle([8 * SS, sy, 67 * SS, sy + 10 * SS], fill=(200, 170, 40, 255))
        for i in range(-2, 14):
            x0 = (8 + i * 6) * SS
            d.line([(x0, sy + 10 * SS), (x0 + 6 * SS, sy)], fill=(30, 30, 30, 255), width=2 * SS)
    elif detail == "lights":
        for i in range(4):
            x0 = (16 + i * 12) * SS
            d.ellipse([x0, 32 * SS, x0 + 5 * SS, 37 * SS], fill=(70, 220, 235, 255))
            d.ellipse([x0 + SS, 33 * SS, x0 + 4 * SS, 36 * SS], fill=(210, 250, 255, 255))
    elif detail == "door":
        d.rectangle([18 * SS, 14 * SS, 57 * SS, 61 * SS], fill=(58, 63, 75, 255),
                    outline=darkline, width=2 * SS)
        d.line([(37 * SS, 14 * SS), (37 * SS, 61 * SS)], fill=darkline, width=2 * SS)
        d.ellipse([31 * SS, 36 * SS, 34 * SS, 39 * SS], fill=(150, 156, 168, 255))
        d.ellipse([41 * SS, 36 * SS, 44 * SS, 39 * SS], fill=(150, 156, 168, 255))
    elif detail == "panel":
        d.rectangle([22 * SS, 22 * SS, 53 * SS, 53 * SS], fill=(88, 95, 108, 255),
                    outline=darkline, width=2 * SS)
        d.line([(22 * SS, 22 * SS), (53 * SS, 22 * SS)], fill=lightline, width=SS)
    elif detail in ("winl", "winm", "winr"):
        y0, y1 = 28 * SS, 46 * SS
        d.rectangle([6 * SS, y0, 69 * SS, y1], fill=(40, 46, 60, 255))
        for i in range(3):
            x0 = (10 + i * 20) * SS
            d.rectangle([x0, y0 + 3 * SS, x0 + 12 * SS, y1 - 3 * SS], fill=(80, 170, 220, 255))
            d.line([(x0, y0 + 3 * SS), (x0 + 12 * SS, y0 + 3 * SS)],
                   fill=(180, 230, 250, 255), width=SS)
    return img.resize((75, 75), LANCZOS)


PLATE_ROLES = [
    ("tl", None), ("t", None), ("t", "vent"), ("all", "hazard"),
    ("t", "lights"), ("tr", None), ("l", None), ("none", "door"),
    ("none", "grate"), ("none", "panel"), ("all", "hazard"), ("r", "vent"),
    ("l", "lights"), ("none", None), ("none", None), ("none", "panel"),
    ("none", None), ("r", "lights"), ("l", "grate"), ("none", "winl"),
    ("none", "winm"), ("none", "winr"), ("none", "grate"), ("none", "panel"),
    ("bl", None), ("b", None), ("b", "lights"), ("all", "hazard"),
    ("all", "panel"), ("br", None),
]


def make_tileset():
    tiles = [metal_plate(outer, detail, seed=2000 + i)
             for i, (outer, detail) in enumerate(PLATE_ROLES)]
    sheet = Image.new("RGBA", (450, 450), (0, 0, 0, 0))
    for i, t in enumerate(tiles):
        sheet.alpha_composite(t, ((i % 6) * 75, (i // 6) * 75))
    return sheet, tiles


# --------------------------------------------------------------------- audio

def make_boom_wav(path, seconds=0.7, rate=22050):
    rnd = random.Random(42)
    n = int(seconds * rate)
    samples = []
    lp = 0.0
    for i in range(n):
        t = i / rate
        env = math.exp(-t * 7.0)
        noise = rnd.uniform(-1.0, 1.0)
        lp += 0.18 * (noise - lp)          # abafa (passa-baixa simples)
        rumble = math.sin(2 * math.pi * (60 - 30 * t) * t) * math.exp(-t * 5.0) * 0.6
        click = noise * math.exp(-t * 90.0) * 0.9
        s = (lp * 1.6 + rumble) * env + click
        samples.append(s)
    peak = max(1e-9, max(abs(s) for s in samples))
    frames = b"".join(struct.pack("<h", int(s / peak * 0.9 * 32767)) for s in samples)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(frames)


# ---------------------------------------------------------------------- main

def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "."
    img_dir = os.path.join(out, "img")
    sfx_dir = os.path.join(out, "sfx")
    tiles_dir = os.path.join(img_dir, "tileset")
    os.makedirs(img_dir, exist_ok=True)
    os.makedirs(sfx_dir, exist_ok=True)
    os.makedirs(tiles_dir, exist_ok=True)

    fundo = starfield(
        800, 600, seed=7, top=(4, 6, 16), bottom=(10, 14, 34),
        nebulae=[((70, 100, 200), 60, 5, 90, 200), ((120, 90, 190), 45, 3, 70, 160)],
        star_count=420,
        planet=(48, 520, 40, 1.0, ((150, 160, 175), (40, 44, 60), (90, 95, 110))))
    fundo.convert("RGB").save(os.path.join(img_dir, "fundo.png"))

    menu = starfield(
        800, 600, seed=11, top=(8, 4, 20), bottom=(16, 10, 38),
        nebulae=[((150, 70, 210), 70, 5, 100, 220), ((80, 80, 230), 45, 3, 80, 170)],
        star_count=380,
        planet=(40, 585, 40, 1.4, ((140, 110, 190), (35, 30, 55), (120, 100, 170))))
    menu.convert("RGB").save(os.path.join(img_dir, "stateMenu.jpg"), quality=85)

    win = starfield(
        800, 600, seed=23, top=(10, 8, 6), bottom=(38, 22, 10),
        nebulae=[((220, 150, 60), 70, 5, 100, 230), ((230, 110, 90), 40, 3, 70, 150)],
        star_count=350,
        planet=(90, 560, 40, 1.5, ((210, 150, 90), (60, 40, 30), (150, 120, 80))))
    win.convert("RGB").save(os.path.join(img_dir, "stateWin.jpg"), quality=85)

    lose = starfield(
        800, 600, seed=31, top=(10, 4, 4), bottom=(30, 8, 8),
        nebulae=[((190, 50, 50), 65, 5, 100, 220), ((130, 40, 90), 40, 3, 70, 160)],
        star_count=320,
        planet=(60, 560, 40, 1.2, ((160, 80, 80), (40, 20, 25), (110, 60, 60))))
    lose.convert("RGB").save(os.path.join(img_dir, "stateLose.jpg"), quality=85)

    ufo = draw_ufo()
    ufo.save(os.path.join(img_dir, "ufo.png"))

    red = sphere(44, (215, 90, 40), (60, 20, 12), (255, 190, 120), seed=5, craters=4)
    red.save(os.path.join(img_dir, "PlanetaVermelho.png"))

    moon = sphere(16, (170, 170, 175), (50, 50, 58), (70, 70, 75), seed=9, craters=5)
    moon.save(os.path.join(img_dir, "Lua.png"))

    draw_ship_sheet().save(os.path.join(img_dir, "NaveSheet.png"))
    draw_boom_sheet().save(os.path.join(img_dir, "BoomSheet.png"))

    sheet, tiles = make_tileset()
    sheet.save(os.path.join(img_dir, "Tileset.png"))
    for i, t in enumerate(tiles):
        t.save(os.path.join(tiles_dir, "SpaceTile-%02d.png" % (i + 1)))

    make_boom_wav(os.path.join(sfx_dir, "boom.wav"))

    print("assets gerados em:", os.path.abspath(out))


if __name__ == "__main__":
    main()
