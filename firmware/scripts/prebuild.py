import os
from SCons.Script import DefaultEnvironment
import shutil
import subprocess
from pathlib import Path

env = DefaultEnvironment()

def build_react_and_copy(source, target, env):
    PROJECT_DIR = Path(os.getcwd())
    REACT_DIR = PROJECT_DIR.parent / "frontend"  # adjust if your React project is here
    DIST_DIR = REACT_DIR / "dist"                # CRA build folder
    DATA_DIR = PROJECT_DIR / "data"

    print("⚡ Building React frontend...")

    if not REACT_DIR.exists():
        raise FileNotFoundError(f"Frontend directory not found: {REACT_DIR}")

    subprocess.run(["npm", "run", "build"], cwd=str(REACT_DIR), check=True, shell=True)

    # Copy build to data/
    if (DATA_DIR / "web").exists():
        shutil.rmtree(DATA_DIR / "web")
    shutil.copytree(DIST_DIR, DATA_DIR / "web")
    
    if not (DATA_DIR / "config.json").exists():
        shutil.copyfile((PROJECT_DIR / "config.example.json"), (PROJECT_DIR / "data" / "config.json"))

env.AddPreAction("upload", build_react_and_copy)