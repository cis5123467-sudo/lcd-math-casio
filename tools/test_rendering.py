#!/usr/bin/env python3
"""Exercise natural templates and symbol rendering in the released SuperH binary."""
from pathlib import Path
import json,math,struct,subprocess,shutil
from PIL import Image
root=Path(__file__).resolve().parents[1]
base=root/'verification/render-v0.0.1'
addin=root/'release/lcd-math-v0.0.1.g3a'

def catalog(index):
 return ', '.join(['F1']+['RIGHT']*(index//5)+['DOWN']*(index%5)+['EXE'])+', '

cases={
 'fraction-template':('FRAC, shot placeholder.png, 1, shot numerator.png, DOWN, shot denominator-placeholder.png, 3, shot input.png, RIGHT, EXE',1/3,'1/3'),
 'fraction-capture':('1, 2, FRAC, 3, shot input.png, EXE',4,'4'),
 'fraction-sum':('FRAC, 1, DOWN, 3, RIGHT, PLUS, FRAC, 1, DOWN, 6, RIGHT, shot input.png, EXE',.5,'1/2'),
 'nested-fractions':('FRAC, FRAC, 1, DOWN, 2, RIGHT, DOWN, 3, shot input.png, EXE',1/6,'1/6'),
 'fraction-denominator':('FRAC, 1, DOWN, FRAC, 2, DOWN, 3, RIGHT, RIGHT, shot input.png, EXE',1.5,'3/2'),
 'tall-brackets':('LPAR, FRAC, 1, DOWN, 2, RIGHT, PLUS, 3, RPAR, MULT, 4, shot input.png, EXE',14,'14'),
 'square-root':('SHIFT, SQUARE, shot placeholder.png, 8, shot input.png, RIGHT, EXE',math.sqrt(8),'2*sqrt(2)'),
 'root-fraction':('SHIFT, SQUARE, FRAC, 1, DOWN, 2, RIGHT, RIGHT, shot input.png, EXE',math.sqrt(.5),'(sqrt(2))/2'),
 'nested-roots':('SHIFT, SQUARE, 1, PLUS, SHIFT, SQUARE, 9, RIGHT, RIGHT, shot input.png, EXE',2,None),
 'cube-root':(catalog(1)+'2, 7, shot input.png, EXE',3,None),
 'nth-root':(catalog(20)+'3, 2, COMMA, 5, shot input.png, EXE',2,None),
 'log-base':(catalog(17)+'8, COMMA, 2, shot input.png, EXE',3,None),
 'power':('2, POWER, shot placeholder.png, 3, shot input.png, RIGHT, EXE',8,'8'),
 'negative-power':('2, POWER, NEG, 3, shot input.png, DOWN, EXE',.125,'1/8'),
 'fractional-power':('9, POWER, FRAC, 1, DOWN, 2, RIGHT, RIGHT, shot input.png, EXE',3,'3'),
 'nested-powers':('2, POWER, 3, POWER, shot placeholder.png, 2, RIGHT, RIGHT, shot input.png, EXE',512,'512'),
 'bracket-power':('LPAR, 2, PLUS, 3, RPAR, SQUARE, shot input.png, EXE',25,'25'),
 'inverse-trig':('SHIFT, SIN, shot placeholder.png, 0, DP, 5, shot input.png, EXE',30,None),
 'pi':('2, MULT, SHIFT, EXP, shot input.png, EXE',2*math.pi,'2*pi'),
 'division':('8, DIV, 2, MULT, LPAR, 1, PLUS, 1, RPAR, shot input.png, EXE',8,'8'),
 'scientific':('6, DP, 0, 2, EXP, shot placeholder.png, 2, 3, shot input.png, EXE',6.02e23,None),
 'negative-fraction':('FRAC, NEG, 1, DOWN, 2, shot input.png, EXE',-.5,'-1/2'),
 'decimal-denominator':('FRAC, 1, DOWN, 0, DP, 2, 5, shot input.png, EXE',4,'4'),
 'edit-fraction':('FRAC, 1, DOWN, 3, UP, DEL, shot placeholder.png, 2, DOWN, shot input.png, EXE',2/3,'2/3'),
 'delete-template':('FRAC, DEL, 4, 2, shot input.png, EXE',42,'42'),
 'fraction-squared':('FRAC, 1, DOWN, 2, RIGHT, SQUARE, shot input.png, EXE',.25,'1/4'),
 'memory-fraction':('6, EXE, VARS, SHIFT, EXE, EXIT, EXIT, FRAC, ALPHA, 1, DOWN, 3, shot input.png, EXE',2,None),
 'exponential':(catalog(18)+'2, shot input.png, EXE',math.exp(2),None),
 'trig-fraction':('SIN, FRAC, 9, 0, DOWN, 3, RIGHT, RIGHT, shot input.png, EXE',.5,'1/2'),
}
results={}
for name,(script,value,exact) in cases.items():
 folder=base/name
 if folder.exists():shutil.rmtree(folder)
 folder.mkdir(parents=True)
 completed=subprocess.run(['/tmp/numplay-toolchain/emu-build/emucasiocalc','--headless','--model','cg50','--keys',
  'scale 1, wait 0.7s, '+script.replace('shot ','wait 0.25s, shot ')+', wait 0.3s, shot result.png, MENU',str(addin)],
  cwd=folder,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=90)
 (folder/'run.log').write_text(completed.stdout)
 assert completed.returncode==0 and 'add-in exited after' in completed.stdout,completed.stdout
 data=(folder/'storage/fls0/lcdmath.dat').read_bytes()
 count=struct.unpack_from('>i',data,8)[0];assert count>0,(name,completed.stdout)
 offset=112+184*(count-1)
 expression=data[offset:offset+112].split(b'\0')[0].decode()
 result=data[offset+112:offset+176].split(b'\0')[0].decode()
 number=struct.unpack_from('>d',data,offset+176)[0]
 assert abs(number-value)<1e-10*max(1,abs(value)),(name,expression,number,value)
 if exact is not None:assert result==exact,(name,result,exact)
 for shot in folder.glob('*.png'):assert Image.open(shot).size==(396,224)
 results[name]={'input':expression,'exact':result,'value':number}
 print(name+': PASS',flush=True)
(base/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(f'{len(cases)} released-binary template/symbol cases passed, with screenshots.')
