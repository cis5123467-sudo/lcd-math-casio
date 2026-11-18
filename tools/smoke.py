#!/usr/bin/env python3
"""Run the released SuperH binary in emuCasioCalc; assert saved calculation results."""
from pathlib import Path
from struct import unpack_from
import subprocess, shutil
from PIL import Image,ImageChops
root=Path(__file__).resolve().parents[1]
addin=root/'release/lcd-math-v0.0.2.g3a'
emulator=Path('/tmp/numplay-toolchain/emu-build/emucasiocalc')
base=root/'verification/smoke-v0.0.2'
if base.exists(): shutil.rmtree(base)

def run(name,script):
    folder=base/name; folder.mkdir(parents=True,exist_ok=True)
    completed=subprocess.run([str(emulator),'--headless','--model','cg50','--clock','2026-11-18 12:00','--keys',
        'scale 1, wait 0.7s, '+script.replace('shot ','wait 0.3s, shot '),str(addin)],
        cwd=folder,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=90)
    (folder/'run.log').write_text(completed.stdout)
    assert completed.returncode==0 and 'add-in exited after' in completed.stdout,completed.stdout
    assert 'stopped with an error' not in completed.stdout,completed.stdout
    return folder

def saved(folder):
    data=(folder/'storage/fls0/lcdmath.dat').read_bytes()
    assert len(data)==3796,len(data)
    header=unpack_from('>IIiiiii10dHH',data)
    assert header[:2]==(0x4c434431,1)
    entries=[]
    for index in range(header[2]):
        offset=112+184*index
        expression=data[offset:offset+112].split(b'\0')[0].decode().replace(chr(28),'/')
        exact=data[offset+112:offset+176].split(b'\0')[0].decode().replace(chr(28),'/')
        value=unpack_from('>d',data,offset+176)[0]
        entries.append((expression,exact,value))
    h=2166136261
    for b in data[:3792]: h=((h^b)*16777619)&0xffffffff
    assert h==unpack_from('>I',data,3792)[0]
    return header,entries

def check(entries,expression,value,exact=None):
    found=[r for r in entries if r[0]==expression]
    assert found,(expression,entries)
    assert abs(found[-1][2]-value)<1e-9,(expression,found)
    if exact is not None: assert found[-1][1]==exact,found

calc=run('calculations','shot startup.png, 1, DIV, 3, EXE, shot fraction.png, FD, shot decimal.png, FD, '
    '2, MULT, LPAR, 3, PLUS, 4, EXE, shot brackets.png, '
    'SHIFT, SQUARE, 8, RPAR, EXE, shot radical.png, '
    'SIN, 3, 0, EXE, shot sine.png, '
    '5, SHIFT, DP, DIV, 2, SHIFT, DP, EXE, shot factorial.png, '
    'PLUS, 4, EXE, shot ans.png, MENU')
header,entries=saved(calc)
check(entries,'1/3',1/3,'1/3');check(entries,'2*(3+4)',14,'14');check(entries,'sqrt(8)',8**.5,'2*sqrt(2)')
check(entries,'sin(30)',.5,'1/2');check(entries,'5!/2!',60,'60');check(entries,'Ans+4',64,'64')
print('SuperH arithmetic, exact/decimal, brackets, roots, trig, factorials and Ans passed',flush=True)
run('calculations','shot restored.png, MENU')
assert ImageChops.difference(Image.open(calc/'ans.png'),Image.open(calc/'restored.png')).getbbox() is None
print('SuperH persistence and identical restored screen passed',flush=True)

edit=run('editing','7, PLUS, 1, LEFT, LEFT, 0, shot edited.png, EXE, '
    'UP, DEL, 2, EXE, shot reused.png, 1, DIV, 0, EXE, shot error.png, '
    'DEL, 2, EXE, shot recovered.png, MENU')
_,entries=saved(edit)
check(entries,'70+1',71);check(entries,'70+2',72);check(entries,'1/2',.5)
assert '1/0' not in [r[0] for r in entries]
print('SuperH cursor insertion, history editing and domain error screen passed',flush=True)

menus=run('menus','F1, shot catalog.png, DOWN, EXE, 2, 7, EXE, shot cube-root.png, '
    'F1, RIGHT, RIGHT, RIGHT, RIGHT, DOWN, EXE, 1, 0, COMMA, 3, RPAR, EXE, shot combination.png, '
    'OPTN, shot settings.png, EXE, shot radians-setting.png, EXIT, '
    'SIN, SHIFT, EXP, DIV, 2, EXE, shot radians.png, '
    'SHIFT, FD, COS, 1, 0, 0, EXE, shot gradians.png, MENU')
header,entries=saved(menus)
check(entries,'cbrt(27)',3);check(entries,'ncr(10,3)',120);check(entries,'sin(pi/2)',1);check(entries,'cos(100)',0)
assert header[3]==2
print('SuperH catalog, combinations, DEG/RAD/GRAD settings passed',flush=True)

memory=run('memory','4, 2, EXE, VARS, SHIFT, EXE, shot stored.png, EXIT, '
    'ALPHA, 1, PLUS, 8, EXE, shot recall.png, F4, shot help.png, RIGHT, shot help2.png, EXIT, MENU')
header,entries=saved(memory)
check(entries,'A+8',50);assert header[8]==42 # First memory value after Ans.
print('SuperH memory store/recall and help pages passed',flush=True)

long=run('long','1, PLUS, 2, PLUS, 3, PLUS, 4, PLUS, 5, PLUS, 6, PLUS, 7, PLUS, 8, PLUS, 9, PLUS, '
    '1, PLUS, 2, PLUS, 3, PLUS, 4, PLUS, 5, PLUS, 6, PLUS, 7, PLUS, 8, PLUS, 9, '
    'shot scrolling.png, SHIFT, LEFT, shot start.png, SHIFT, RIGHT, shot end.png, EXE, shot result.png, MENU')
_,entries=saved(long);assert entries[-1][2]==90
print('SuperH horizontal scrolling and start/end navigation passed',flush=True)
contrast=run('contrast','OPTN, DOWN, DOWN, DOWN, DOWN, EXE, shot contrast.png, '
    + ', '.join(['LEFT']*14)+', shot light.png, '
    + ', '.join(['RIGHT']*20)+', shot dark.png, '
    + ', '.join(['LEFT']*6)+', shot chosen.png, EXE, EXIT, shot calculator.png, MENU')
header,_=saved(contrast);assert header[-1]==4
run('contrast','shot restored.png, MENU')
assert ImageChops.difference(Image.open(contrast/'calculator.png'),Image.open(contrast/'restored.png')).getbbox() is None
light=Image.open(contrast/'light.png').convert('RGB').crop((15,40,380,190))
dark=Image.open(contrast/'dark.png').convert('RGB').crop((15,40,380,190))
assert min(sum(p) for p in light.get_flattened_data())>min(sum(p) for p in dark.get_flattened_data())+100
for shot in (contrast/'calculator.png',calc/'ans.png'):
    im=Image.open(shot).convert('RGB')
    # An empty footer region must contain only the fixed 16x16 LCD texture.
    phases={}
    for y in range(201,215):
        for x in range(10,385):
            phase=(x%16,y%16);color=im.getpixel((x,y))
            assert phase not in phases or phases[phase]==color, 'Extra text on main LCD'
            phases[phase]=color
    assert 1<len(set(phases.values()))<=3, 'Missing or excessive LCD grain'
print('Contrast clamps at 0/10, changes all LCD shades live, persists, and main LCD has no footer text',flush=True)

for shot in base.rglob('*.png'):
    im=Image.open(shot).convert('RGB');assert im.size==(396,224)
    assert im.getpixel((0,0))==im.getpixel((395,0))
print('Native 396x224 screenshots and screen bounds passed',flush=True)
