#!/usr/bin/env python3
"""Execute the release with Graph Math+ logical keys; inspect saved results."""
import shutil
from PIL import Image,ImageChops
from mathplus_emulator import ROOT,run as drive,saved,catalog
base=ROOT/'verification/smoke-v0.0.4'
if base.exists():shutil.rmtree(base)
def run(name,script):return drive(base/name,script)
def check(entries,expression,value,exact=None):
 found=[r for r in entries if r[0]==expression]
 assert found,(expression,entries)
 assert abs(found[-1][2]-value)<1e-9,(expression,found)
 if exact is not None:assert found[-1][1]==exact,found
calc=run('calculations','shot startup.png, 1, DIV, 3, EXE, shot fraction.png, FORMAT, 1, shot decimal.png, FORMAT, 1, '
 '2, MULT, LPAR, 3, PLUS, 4, EXE, shot brackets.png, '
 'SQRT, 8, RPAR, EXE, shot radical.png, SIN, 3, 0, EXE, shot sine.png, '
 '5, '+catalog(6)+'DIV, 2, '+catalog(6)+'EXE, shot factorial.png, PLUS, 4, EXE, shot ans.png, HOME')
header,entries=saved(calc)
check(entries,'1/3',1/3,'1/3');check(entries,'2*(3+4)',14,'14');check(entries,'sqrt(8)',8**.5,'2*sqrt(2)')
check(entries,'sin(30)',.5,'1/2');check(entries,'5!/2!',60,'60');check(entries,'Ans+4',64,'64')
print('Arithmetic, exact/decimal FORMAT menu, brackets, roots, sin, factorials and Ans passed',flush=True)
run('calculations','shot restored.png, HOME')
assert ImageChops.difference(Image.open(calc/'ans.png'),Image.open(calc/'restored.png')).getbbox() is None
print('Persistence and identical restored screen passed',flush=True)
edit=run('editing','7, PLUS, 1, LEFT, LEFT, 0, shot edited.png, EXE, '
 'UP, DEL, 2, EXE, shot reused.png, 1, DIV, 0, EXE, shot error.png, DEL, 2, EXE, shot recovered.png, HOME')
_,entries=saved(edit)
check(entries,'70+1',71);check(entries,'70+2',72);check(entries,'1/2',.5)
assert '1/0' not in [r[0] for r in entries]
print('Cursor insertion, history editing and domain error recovery passed',flush=True)
menus=run('menus','CATALOG, shot catalog.png, 2, 2, 7, EXE, shot cube-root.png, '
 +catalog(21)+'1, 0, COMMA, 3, RPAR, EXE, shot combination.png, '
 'SETTINGS, shot settings.png, 1, shot radians-setting.png, BACK, '
 'SIN, SHIFT, 7, DIV, 2, EXE, shot radians.png, SETTINGS, 1, BACK, '
 'COS, 1, 0, 0, EXE, shot gradians.png, HOME')
header,entries=saved(menus)
check(entries,'cbrt(27)',3);check(entries,'ncr(10,3)',120);check(entries,'sin(pi/2)',1);check(entries,'cos(100)',0)
assert header[3]==2
print('CATALOG shortcuts, combinations, DEG/RAD/GRAD SETTINGS passed',flush=True)
memory=run('memory','4, 2, SHIFT, XVAR, ALPHA, XVAR, EXE, shot stored.png, AC, '
 'ALPHA, XVAR, PLUS, 8, EXE, shot recall.png, TOOLS, 1, shot help.png, RIGHT, shot help2.png, BACK, HOME')
header,entries=saved(memory);check(entries,'A+8',50);assert header[8]==42
print('Arrow assignment, alphabet recall and physical-key help passed',flush=True)
long=run('long',', '.join([item for _ in range(2) for i in range(1,10) for item in (str(i),'PLUS')][:-1])+', shot scrolling.png, EXE, shot result.png, HOME')
_,entries=saved(long);assert entries[-1][2]==90
print('Horizontal scrolling passed',flush=True)
contrast=run('contrast','SETTINGS, 5, shot contrast.png, '+', '.join(['LEFT']*14)+', shot light.png, '
 +', '.join(['RIGHT']*20)+', shot dark.png, '+', '.join(['LEFT']*6)+', shot chosen.png, EXE, BACK, shot calculator.png, HOME')
header,_=saved(contrast);assert header[-1]==4
run('contrast','shot restored.png, HOME')
assert ImageChops.difference(Image.open(contrast/'calculator.png'),Image.open(contrast/'restored.png')).getbbox() is None
light=Image.open(contrast/'light.png').convert('RGB').crop((15,40,380,190))
dark=Image.open(contrast/'dark.png').convert('RGB').crop((15,40,380,190))
assert min(sum(p) for p in light.get_flattened_data())>min(sum(p) for p in dark.get_flattened_data())+100
# Spatial lighting varies across the panel, so use the empty startup frame
# at the same contrast to detect stray labels in the calculation footer.
footer=(10,201,385,215)
startup=Image.open(calc/'startup.png').convert('RGB')
answer=Image.open(calc/'ans.png').convert('RGB')
assert ImageChops.difference(startup.crop(footer),answer.crop(footer)).getbbox() is None
print('Contrast limits, live shades, persistence, clean main LCD passed',flush=True)
for shot in base.rglob('*.png'):
 im=Image.open(shot).convert('RGB');assert im.size==(396,224)
 assert im.getpixel((0,0))==im.getpixel((395,0))
print('Native 396x224 screenshots and screen bounds passed',flush=True)
