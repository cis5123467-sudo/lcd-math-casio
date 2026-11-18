#!/usr/bin/env python3
"""Compile and run calculator checks in isolated storage with ASan and UBSan."""
from pathlib import Path
import os,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
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
        '-I',str(root/'src'),str(root/'tests/mathview_test.c'),str(root/'src/mathview.c'),'-o',str(exe)],check=True)
    result=subprocess.run([str(exe)],cwd=folder,check=True,text=True,stdout=subprocess.PIPE)
    (root/'verification/layout-summary.txt').write_text(result.stdout)
    print(result.stdout,end='')
