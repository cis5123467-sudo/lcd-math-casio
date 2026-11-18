#!/usr/bin/env python3
"""Check fixed LCD grain, contrast readability and optional old/new glyph masks."""
from pathlib import Path
import argparse,json
from PIL import Image,ImageChops,ImageStat
root=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument('--baseline-dir',type=Path,help='Optional older render-suite folder for pixel-mask comparison')
args=ap.parse_args()
render=root/'verification/render-v0.0.3'
smoke=root/'verification/smoke-v0.0.3'
current=Image.open(render/'square-root/result.png').convert('RGB')
patch=current.crop((150,80,250,140))
colors=patch.getcolors(1000)
assert colors and 1<len(colors)<=3, 'Expected subtle, limited LCD paper shades'
assert max(sum(rgb) for _,rgb in colors)-min(sum(rgb) for _,rgb in colors)<=45, 'Excessive background grain'
# Pixels at the same physical LCD position must have the same shade after relaunch.
for folder,a,b in [(smoke/'calculations','ans.png','restored.png'),(smoke/'contrast','calculator.png','restored.png')]:
 assert ImageChops.difference(Image.open(folder/a),Image.open(folder/b)).getbbox() is None, 'Texture moved across relaunch'
for name in ['light','dark']:
 image=Image.open(smoke/f'contrast/{name}.png').convert('RGB')
 region=image.crop((20,90,375,145))
 assert max(sum(p) for p in region.get_flattened_data())-min(sum(p) for p in region.get_flattened_data())>=100, 'Contrast preview became unreadable'
# Filled numeral silhouettes retain all active dots but contain subtly varying ink.
char_colors={p for p in current.crop((329,155,380,200)).get_flattened_data() if sum(p)<300}
assert 1<len(char_colors)<=6, 'Expected subtle character/rule shading'
report={'background_shades':len(colors),'background_mean_rgb':ImageStat.Stat(patch).mean,
        'character_shades':len(char_colors),'restored_frames_identical':True,'baseline_masks_compared':0}
if args.baseline_dir:
 baseline=Image.open(args.baseline_dir/'square-root/result.png').convert('RGB')
 old_mean=ImageStat.Stat(baseline.crop((150,80,250,140))).mean
 delta=[old_mean[i]-report['background_mean_rgb'][i] for i in range(3)]
 assert all(5<=d<=18 for d in delta),('Expected a slightly darker LCD',delta)
 for path in render.glob('*/*.png'):
  previous=args.baseline_dir/path.parent.name/path.name
  if not previous.exists():continue
  old=Image.open(previous).convert('RGB').crop((10,30,386,210))
  new=Image.open(path).convert('RGB').crop((10,30,386,210))
  old_mask=[sum(p)<300 for p in old.get_flattened_data()]
  new_mask=[sum(p)<300 for p in new.get_flattened_data()]
  assert old_mask==new_mask, f'Changed glyph geometry: {path}'
  report['baseline_masks_compared']+=1
 report['background_rgb_darkening']=delta
(root/'verification/texture-v0.0.3-summary.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
print('Fixed LCD grain, subtly shaded glyphs, contrast readability and restored frames passed.')
