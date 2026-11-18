#!/usr/bin/env python3
from pathlib import Path
from PIL import Image, ImageDraw
root=Path(__file__).resolve().parents[1]
im=Image.open(root/'assets/font5x7.png').convert('RGB')
rows=[]
for code in range(128):
    x,y=code%16*7+1,code//16*9+1
    rows.append([sum((1<<(4-c)) for c in range(5) if sum(im.getpixel((x+c,y+r)))<384) for r in range(7)])
(root/'src/font.h').write_text('// 5 x 7 LCD glyphs; generated from assets/font5x7.png\nstatic const unsigned char lcd_font[128][7]={\n'+',\n'.join('{'+','.join(map(str,row))+'}' for row in rows)+'\n};\n')
for selected in (False,True):
    icon=Image.new('RGB',(92,64),'#ffffff' if not selected else '#ffb52b')
    d=ImageDraw.Draw(icon)
    d.rounded_rectangle((7,5,84,58),radius=6,fill='#354039')
    d.rectangle((11,9,80,54),fill='#cedcc1')
    def text(x,y,t,s=1):
        for ch in t:
            for r,bits in enumerate(rows[ord(ch)]):
                for c in range(5):
                    if bits&(1<<(4-c)): d.rectangle((x+c*s,y+r*s,x+(c+1)*s-1,y+(r+1)*s-1),fill='#22291f')
            x+=6*s
    text(16,14,'MATH  D'); text(17,27,'1+2'); text(62,40,'3',2)
    icon.save(root/'assets'/('icon-sel.png' if selected else 'icon-uns.png'))
