#!/usr/bin/env python3
"""Test the dedicated FRAC key (0x51), rather than the DIV key, in the shipped binary."""
from pathlib import Path
import json,subprocess,struct
root=Path(__file__).resolve().parents[1]
addin=root/'release/lcd-math-1.0.1.g3a'
base=root/'verification/fraction-key-1.0.1'
cases={
 'after-numerator':'1, FRAC, shot empty-denominator.png, 3, shot input.png, EXE, shot result.png, FD, shot decimal.png, MENU',
 'template-first':'FRAC, shot fraction-first.png, 1, DOWN, 3, shot input.png, EXE, shot result.png, MENU',
 'sum':'1, FRAC, 3, PLUS, 1, FRAC, 6, shot input.png, EXE, shot result.png, MENU',
 'grouped-numerator':'LPAR, 1, PLUS, 2, RPAR, FRAC, 4, shot input.png, EXE, shot result.png, MENU',
 'grouped-denominator':'1, FRAC, LPAR, 2, PLUS, 3, RPAR, shot input.png, EXE, shot result.png, MENU',
 'numerator-edit':'1, FRAC, 3, LEFT, LEFT, 2, shot input.png, EXE, shot result.png, MENU',
}
results={}
for name,script in cases.items():
 folder=base/name;folder.mkdir(parents=True,exist_ok=True)
 assert not (folder/'storage').exists(),'Use a fresh verification directory'
 completed=subprocess.run(['/tmp/numplay-toolchain/emu-build/emucasiocalc','--headless','--model','cg50','--keys',
  'scale 1, wait 0.7s, '+script.replace('shot ','wait 0.3s, shot '),str(addin)],cwd=folder,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=90)
 (folder/'run.log').write_text(completed.stdout)
 assert completed.returncode==0 and 'add-in exited after' in completed.stdout,completed.stdout
 data=(folder/'storage/fls0/lcdmath.dat').read_bytes()
 count=struct.unpack_from('>i',data,8)[0]
 records=[]
 for i in range(count):
  offset=112+184*i
  records.append({'expression':data[offset:offset+112].split(b'\0')[0].decode(),
   'exact':data[offset+112:offset+176].split(b'\0')[0].decode(), 'value':struct.unpack_from('>d',data,offset+176)[0]})
 results[name]=records
expected={'after-numerator':('1/3','1/3',1/3),'sum':('1/3+1/6','1/2',.5),
 'grouped-numerator':('(1+2)/4','3/4',.75),'grouped-denominator':('1/(2+3)','1/5',.2),
 'numerator-edit':('12/3','4',4)}
for name,(expr,exact,value) in expected.items():
 row=results[name][-1];assert row['expression']==expr and row['exact']==exact and abs(row['value']-value)<1e-12,(name,row)
assert results['template-first']==[],results['template-first']
(base/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))
print('Dedicated FRAC key: numerator-first, fraction addition, grouped terms and cursor editing pass. Template-first entry fails.')
