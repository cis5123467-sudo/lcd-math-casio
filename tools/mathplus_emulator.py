"""Drive the release using Graph Math+ logical key names on CG50 matrix hardware.

The app maps the old physical matrix positions using gint's Math+ table when
running on the CG50 emulator. This does not model Graph Math+ LCD/OS hardware.
"""
from pathlib import Path
import os
import subprocess,struct
ROOT=Path(__file__).resolve().parents[1]
ADDIN=ROOT/'release/lcd-math-v0.0.4.g3a'
EMULATOR=os.environ.get('CASIO_EMULATOR','/tmp/numplay-toolchain/emu-build/emucasiocalc')
MATRIX={
 'ON':'F1','HOME':'F2','PREVTAB':'F3','UP':'F4','NEXTTAB':'F5','PAGEUP':'F6',
 'SETTINGS':'SHIFT','BACK':'OPTN','LEFT':'VARS','OK':'MENU','RIGHT':'LEFT','PAGEDOWN':'UP',
 'SHIFT':'ALPHA','ALPHA':'SQUARE','VARIABLE':'POWER','DOWN':'EXIT','CATALOG':'DOWN','TOOLS':'RIGHT',
 'XVAR':'XOT','FRAC':'LOG','SQRT':'LN','POWER':'SIN','SQUARE':'COS','EXPFUN':'TAN',
 'COMMA':'FRAC','SIN':'FD','COS':'LPAR','TAN':'RPAR','LPAR':'COMMA','RPAR':'ARROW',
 'FORMAT':'NEG','EXE':'EXE','DEL':'DEL','AC':'AC','MULT':'MULT','DIV':'DIV','PLUS':'PLUS','SUB':'MINUS',
 'EXP':'EXP','DP':'DP',**{str(i):str(i) for i in range(10)}}
LETTERS=dict(zip('ABCDEFGHIJKLMNOPQRSTUVWXYZ',['XVAR','FRAC','SQRT','POWER','SQUARE','EXPFUN',
 'COMMA','SIN','COS','TAN','LPAR','RPAR','7','8','9','4','5','6','MULT','DIV','1','2','3','PLUS','SUB','0']))
def translate(script):
 output=[]
 for step in script.split(','):
  step=step.strip()
  if not step:continue
  if step.startswith(('shot ','wait ','scale ')):
   if step.startswith('shot '):output.append('wait 0.3s')
   output.append(step)
  else:output.append(MATRIX[step])
 return ', '.join(output)
def run(folder,script):
 folder=Path(folder);folder.mkdir(parents=True,exist_ok=True)
 completed=subprocess.run([EMULATOR,'--headless','--model','cg50','--clock','2026-11-18 12:00','--keys',
   'scale 1, wait 0.7s, '+translate(script),str(ADDIN)],cwd=folder,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=180)
 (folder/'run.log').write_text(completed.stdout)
 assert completed.returncode==0 and 'add-in exited after' in completed.stdout,completed.stdout
 assert 'stopped with an error' not in completed.stdout,completed.stdout
 return folder

def saved(folder):
 data=(Path(folder)/'storage/fls0/lcdmath.dat').read_bytes()
 assert len(data)==3952,len(data)
 header=struct.unpack_from('>IIiiiii29dII',data)
 assert header[:2]==(0x4c434431,2),header[:2]
 entries=[]
 for i in range(header[2]):
  offset=268+184*i
  expression=data[offset:offset+112].split(b'\0')[0].decode().replace(chr(28),'/')
  exact=data[offset+112:offset+176].split(b'\0')[0].decode()
  value=struct.unpack_from('>d',data,offset+176)[0]
  entries.append((expression,exact,value))
 h=2166136261
 for byte in data[:-4]:h=((h^byte)*16777619)&0xffffffff
 assert h==struct.unpack_from('>I',data,len(data)-4)[0]
 return header,entries

def catalog(index):
 if index>=34:return ', '.join(['CATALOG','ALPHA','0']+['RIGHT']*((index-34)//5)+['DOWN']*((index-34)%5)+['EXE'])+', '
 return ', '.join(['CATALOG']+['RIGHT']*(index//5)+['DOWN']*(index%5)+['EXE'])+', '
