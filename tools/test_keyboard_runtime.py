#!/usr/bin/env python3
"""Exercise the physical Graph Math+ key map in the actual release binary."""
import math,shutil,struct
from PIL import Image,ImageChops,ImageStat
from mathplus_emulator import ROOT,run,saved,catalog,LETTERS
base=ROOT/'verification/keyboard-v0.0.3'
if base.exists():shutil.rmtree(base)
def drive(name,keys):return run(base/name,keys)
def check(name,keys,value,expression=None):
 folder=drive(name,keys+', shot result.png, HOME');header,entries=saved(folder)
 assert entries,(name,'no calculation')
 entry=entries[-1];assert math.isclose(entry[2],value,rel_tol=1e-10,abs_tol=1e-10),(name,entry,value)
 if expression is not None:assert entry[0]==expression,(name,entry,expression)
 print(name+': PASS',flush=True);return folder,header,entries
# All 26 red alphabet legends store distinct values; repeated app launches retain them.
script=[]
for index,(letter,key) in enumerate(LETTERS.items()):
 script.extend(list(str(index+1))+['SHIFT','XVAR','ALPHA',key,'EXE','AC'])
folder=drive('alphabet',', '.join(script)+', VARIABLE, shot variables.png, HOME')
header,_=saved(folder);assert header[8:34]==tuple(range(1,27)),header[8:34]
assert header[-2]==(1<<26)-1
run(folder,'XVAR, PLUS, ALPHA, SUB, EXE, shot x-alias.png, HOME')
assert saved(folder)[1][-1][2]==49 # primary x aliases X; ALPHA − enters Y
sum_keys=['AC']
for i,key in enumerate(LETTERS.values()):
 if i:sum_keys.append('PLUS')
 sum_keys.extend(['ALPHA',key])
run(folder,', '.join(sum_keys)+', EXE, shot all-recalled.png, HOME')
assert saved(folder)[1][-1][2]==351
print('All A–Z stores/recalls, persistence and x/X alias: PASS',flush=True)
check('theta','7, SHIFT, XVAR, SHIFT, 2, EXE, AC, SHIFT, 2, PLUS, 1, EXE',8)
# Variable r is item R (index 26) in the variable menu: ALPHA 6.
check('radius','9, SHIFT, XVAR, VARIABLE, ALPHA, 6, EXE, AC, VARIABLE, ALPHA, 6, PLUS, 1, EXE',10)
check('nth-root','SHIFT, SQRT, shot index-placeholder.png, 5, RIGHT, 3, 2, EXE',2,'root(32,5)')
check('log-base','SHIFT, POWER, shot base-placeholder.png, 2, RIGHT, 8, EXE',3,'logb(8,2)')
check('log10','SHIFT, SQUARE, 1, 0, 0, EXE',2,'log(100)')
check('ln-exp','SHIFT, EXPFUN, EXPFUN, 2, EXE',2,'ln(exp(2))')
check('inverse-cos','SHIFT, COS, 0, DP, 5, EXE',60,'acos(0.5)')
check('inverse-tan','SHIFT, TAN, 1, EXE',45,'atan(1)')
check('mixed-fraction','SHIFT, FRAC, shot mixed-placeholder.png, 2, RIGHT, 1, DOWN, 3, RIGHT, EXE',7/3,'mixed(2,1,3)')
check('pi-radians','SIN, LPAR, SHIFT, 7, DIV, 2, RPAR, SHIFT, 1, RPAR, EXE',1)
check('dms','1, 2, SHIFT, PLUS, 3, 0, SHIFT, PLUS, EXE, FORMAT, 3, shot dms.png',12.5)
check('negative','5, EXE, SHIFT, SUB, 2, EXE',-2,chr(25)+'2')
check('subtraction','5, EXE, SUB, 2, EXE',3,'Ans-2')
check('ans','5, EXE, SHIFT, FORMAT, PLUS, 2, EXE',7,'Ans+2')
folder,header,_=check('approximate','FRAC, 1, DOWN, 3, SHIFT, EXE',1/3)
assert header[6]&4
folder,header,_=check('exact','FRAC, 1, DOWN, 3, EXE',1/3)
assert not(header[6]&4)
# Native numbered shortcuts, unmodified A–L keys and ALPHA M–Z.
check('menu-digit','5, CATALOG, 7, EXE',120,'5!')
check('menu-A','CATALOG, XVAR, 3, 0, EXE',.5,'sin(30)')
check('menu-L','CATALOG, RPAR, 5, RIGHT, 3, 2, EXE',2,'root(32,5)')
check('menu-M','CATALOG, ALPHA, 7, 1, 0, COMMA, 3, EXE',120,'ncr(10,3)')
check('menu-Z-more','CATALOG, ALPHA, 0, shot more.png, 1, 0, EXE',0,'asinh(0)')
check('clipboard-copy','1, 2, 3, 4, 5, LEFT, LEFT, SHIFT, 0, LEFT, LEFT, shot selected.png, OK, 1, AC, SHIFT, DP, EXE',23,'23')
check('clipboard-cut','1, 2, 3, 4, 5, LEFT, LEFT, SHIFT, 0, LEFT, LEFT, OK, 2, EXE',145,'145')
check('insert-capture','8, LEFT, SHIFT, DEL, SQRT, EXE',math.sqrt(8),'sqrt(8)')
# ALPHA lock is live until cancelled. The quotes/space row must enter literal text.
folder=drive('literal','SHIFT, ALPHA, XVAR, FRAC, ALPHA, ALPHA, DP, ALPHA, EXP, SHIFT, COMMA, SHIFT, LPAR, SHIFT, 4, SHIFT, 5, SHIFT, 8, SHIFT, 9, shot punctuation.png, EXE, shot error.png, HOME')
assert not saved(folder)[1]
# Actual capture action writes a valid BMP of the clean LCD, not the SHIFT screen.
folder=drive('capture','SQRT, 8, EXE, SHIFT, EXP, shot screen.png, HOME')
bmp=Image.open(folder/'storage/fls0/lcdshot.bmp').convert('RGB')
png=Image.open(folder/'screen.png').convert('RGB')
assert bmp.size==(396,224)
assert max(max(pair) for pair in ImageChops.difference(bmp,png).getextrema())<=7
print('Literal punctuation/text rejection, ALPHA lock and BMP screen capture: PASS',flush=True)
# Backplates must be a subtle uniform delta from same-phase LCD paper. Glyphs
# themselves are excluded by testing only blank edge pixels of each active pad.
folder=drive('status','shot blank.png, SHIFT, shot shift.png, SHIFT, ALPHA, shot alpha.png, ALPHA, 1, EXE, shot history.png, HOME')
blank=Image.open(folder/'blank.png').convert('RGB')
shift=Image.open(folder/'shift.png').convert('RGB')
alpha=Image.open(folder/'alpha.png').convert('RGB')
for frame,x,y in [(blank,45,12),(blank,78,12),(shift,13,13),(alpha,27,13)]:
 delta=[a-b for a,b in zip(frame.getpixel((x+112,y+32)),frame.getpixel((x,y)))]
 assert all(1<=v<=9 for v in delta),(x,y,delta)
print('Active icon backplates have a slight LCD shade difference: PASS',flush=True)
# Migrate the previous SH wire format, including its checksum and old X slot.
folder=base/'legacy';storage=folder/'storage/fls0';storage.mkdir(parents=True)
data=bytearray(3796)
struct.pack_into('>IIiiiii10dHH',data,0,0x4c434431,1,1,0,0,6,0,42,42,0,0,0,0,0,24,0,0,(1<<0)|(1<<6),3)
data[112:115]=b'42\0';data[224:227]=b'42\0';struct.pack_into('>d',data,288,42)
h=2166136261
for b in data[:-4]:h=((h^b)*16777619)&0xffffffff
struct.pack_into('>I',data,len(data)-4,h);(storage/'lcdmath.dat').write_bytes(data)
run(folder,'shot restored.png, HOME');header,entries=saved(folder)
assert header[8]==42 and header[31]==24 and header[-1]==3,(header,entries)
assert header[-2]==(1<<0)|(1<<23)
assert entries[-1]==('42','42',42)
print('Version 1 save migrated to 28 variables without losing history or contrast: PASS',flush=True)
print('Released-binary keyboard/menu/clipboard/format/status/save regression suite passed.')
