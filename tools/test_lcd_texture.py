#!/usr/bin/env python3
"""Check the olive palette, fixed glass lighting, contrast and idle indicators."""
from pathlib import Path
import json
from PIL import Image, ImageChops, ImageStat

root = Path(__file__).resolve().parents[1]
smoke = root / "verification/smoke-v0.0.4"
render = root / "verification/render-v0.0.4"
current = Image.open(render / "square-root/result.png").convert("RGB")
blank = Image.open(smoke / "calculations/startup.png").convert("RGB")

def mean(image, box):
    return ImageStat.Stat(image.crop(box)).mean

center = (150, 80, 250, 140)
paper = mean(current, center)
assert paper[1] > paper[0] + 5 and paper[0] > paper[2] + 35, paper
assert max(paper) < 183, paper
colors = current.crop(center).getcolors(1000)
assert colors and 1 < len(colors) <= 4, colors
left = mean(blank, (24, 102, 88, 150))
right = mean(blank, (308, 102, 372, 150))
assert 1 < sum(left) - sum(right) < 15, (left, right)
edge = mean(blank, (2, 102, 6, 150))
near_edge = mean(blank, (12, 102, 16, 150))
assert 3 < sum(near_edge) - sum(edge) < 20, (edge, near_edge)

# Olive dark ink is legible and keeps the subtle variation in LCD pixels.
ink = {pixel for pixel in current.crop((329, 155, 380, 200)).get_flattened_data()
       if sum(pixel) < 260}
assert len(ink) > 1
assert all(g > r > b and sum(rgb) > 60 for rgb in ink for r, g, b in [rgb])
for folder, a, b in [
    (smoke / "calculations", "ans.png", "restored.png"),
    (smoke / "contrast", "calculator.png", "restored.png"),
]:
    assert ImageChops.difference(Image.open(folder / a), Image.open(folder / b)).getbbox() is None
for name in ("light", "dark"):
    image = Image.open(smoke / f"contrast/{name}.png").convert("RGB")
    region = image.crop((20, 90, 375, 145))
    values = list(map(sum, region.get_flattened_data()))
    assert max(values) - min(values) >= 80, (name, max(values) - min(values))

report = {"paper_rgb": paper, "left_rgb": left, "right_rgb": right,
          "inner_edge_rgb": edge, "near_edge_rgb": near_edge,
          "ink_shades": len(ink), "restored_frames_identical": True}
(root / "verification/texture-v0.0.4-summary.json").write_text(json.dumps(report, indent=2) + "\n")
print("Olive glass, subtle lighting, inner shadow, ink, and contrast checks passed.")
