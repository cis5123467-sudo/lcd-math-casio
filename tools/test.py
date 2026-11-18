#!/usr/bin/env python3
"""Compile and run calculator checks in isolated storage with ASan and UBSan."""
from pathlib import Path
import os,subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[1]
(root/'verification').mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='lcd-math-check-') as folder:
    exe=Path(folder)/'checks'
    subprocess.run([os.environ.get('CC','cc'),'-std=c11','-Wall','-Wextra','-fsanitize=address,undefined','-g',
        '-I',str(root/'src'),str(root/'tests/calculator_test.c'),str(root/'src/calculator.c'),str(root/'src/cas.c'),
        '-lm','-o',str(exe)],check=True)
    result=subprocess.run([str(exe)],cwd=folder,check=True,text=True,stdout=subprocess.PIPE)
    (root/'verification/host-summary.txt').write_text(result.stdout)
    print(result.stdout,end='')

with tempfile.TemporaryDirectory(prefix='lcd-layout-check-') as folder:
    exe=Path(folder)/'checks'
    subprocess.run([os.environ.get('CC','cc'),'-std=c11','-Wall','-Wextra','-fsanitize=address,undefined','-g',
        '-I',str(root/'src'),str(root/'tests/mathview_test.c'),str(root/'src/mathview.c'),str(root/'src/text.c'),'-o',str(exe)],check=True)
    result=subprocess.run([str(exe)],cwd=folder,check=True,text=True,stdout=subprocess.PIPE)
    (root/'verification/layout-summary.txt').write_text(result.stdout)
    print(result.stdout,end='')

sdk=Path(os.environ.get('CASIO_SDK','/tmp/numplay-toolchain/prefix'))
include=subprocess.check_output([str(sdk/'bin/fxsdk'),'path','include'],text=True).strip()
with tempfile.TemporaryDirectory(prefix='lcd-keyboard-check-') as folder:
    exe=Path(folder)/'checks'
    (Path(folder)/'gint').mkdir()
    shutil.copyfile(Path(include)/'gint/keycodes.h',Path(folder)/'gint/keycodes.h')
    subprocess.run([os.environ.get('CC','cc'),'-std=c11','-Wall','-Wextra','-fsanitize=address,undefined','-g',
        '-I',str(root/'src'),'-I',folder,str(root/'tests/keyboard_test.c'),str(root/'src/keyboard.c'),str(root/'src/text.c'),'-o',str(exe)],check=True)
    result=subprocess.run([str(exe)],check=True,text=True,stdout=subprocess.PIPE)
    (root/'verification/keyboard-summary.txt').write_text(result.stdout)
    print(result.stdout,end='')
    dump=subprocess.check_output([str(exe),'--dump'],text=True)
    (root/'verification/keyboard-legends.tsv').write_text(dump)
