#!/usr/bin/env python3
"""Configure fxSDK's colour target, then build through GenerateG3A."""
from pathlib import Path
import os, subprocess, shutil
root=Path(__file__).resolve().parents[1]
sdk=Path(os.environ.get('CASIO_SDK','/tmp/numplay-toolchain/prefix'))
cmake=os.environ.get('CMAKE',shutil.which('cmake') or '/tmp/numplay-toolchain/cmake-4.1.3-macos-universal/CMake.app/Contents/bin/cmake')
env={**os.environ,'PATH':str(sdk/'bin')+os.pathsep+os.environ['PATH']}
subprocess.run([cmake,'-S',str(root),'-B',str(root/'build-cg'),'-DCMAKE_TOOLCHAIN_FILE='+str(sdk/'lib/cmake/fxsdk/FXCG50.cmake'),'-DCMAKE_MODULE_PATH='+str(root/'cmake')+';'+str(sdk/'lib/cmake/fxsdk'),'-DCMAKE_BUILD_TYPE=Release','-DCMAKE_EXE_LINKER_FLAGS='],env=env,check=True)
subprocess.run([cmake,'--build',str(root/'build-cg'),'-j4'],env=env,check=True)
print(root/'release/lcd-math-v0.0.1.g3a')
