#!/usr/bin/env python3

import os
import sys
import subprocess
import argparse
import re
import platform
import shutil
from pathlib import Path

# Color constants
RED = '\033[0;31m'
GREEN = '\033[0;32m'
YELLOW = '\033[1;33m'
NC = '\033[0m'  # No Color

# Default values
BUILD_OPTIONS = '10D -j "OPTIMIZATION=-Os -DDEBUGVER=1"'
run_script_rebuild = False
run_snippet_rebuild = False
musid = ""
skip_actors = False
only_rom = False
MAKE_CMD = "make clean && make"
is_wsl = False
scripts_only = False
deploy = False

def flush_output():
    """Flush both stdout and stderr to ensure proper ordering"""
    sys.stdout.flush()
    sys.stderr.flush()

def print_with_flush(*args, **kwargs):
    """Print and immediately flush output"""
    print(*args, **kwargs)
    flush_output()

def is_wsl_environment():
    """Check if running in WSL environment"""
    uname_r = platform.uname().release
    if 'microsoft' in uname_r.lower() or 'wsl' in uname_r.lower():
        return True
    
    try:
        with open('/proc/version', 'r') as f:
            if 'microsoft' in f.read().lower():
                return True
    except:
        pass
    
    return os.environ.get('WSL_DISTRO_NAME') is not None

def is_msys2_environment():
    """Check if running in MSYS2 environment"""
    return os.environ.get('MSYSTEM') is not None

def run_command(cmd, cwd=None, shell=True, capture_output=False):
    """Run a command and return the result"""
    try:
        if capture_output:
            # For commands where we want to capture and display output line by line
            if isinstance(cmd, list):
                process = subprocess.Popen(cmd, cwd=cwd, shell=False,
                                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                         text=True, bufsize=1, universal_newlines=True)
            else:
                process = subprocess.Popen(cmd, shell=shell, cwd=cwd,
                                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                         text=True, bufsize=1, universal_newlines=True)
            
            # Read output line by line and print immediately
            for line in process.stdout:
                print(line.rstrip())
                flush_output()
            
            process.wait()
            return_code = process.returncode
        else:
            # For regular commands, let them inherit our stdout/stderr
            if isinstance(cmd, list):
                result = subprocess.run(cmd, cwd=cwd, check=True, 
                                      text=True, shell=False,
                                      stdout=sys.stdout, stderr=sys.stderr)
            else:
                result = subprocess.run(cmd, shell=shell, cwd=cwd, check=True, 
                                      text=True,
                                      stdout=sys.stdout, stderr=sys.stderr)
            return_code = result.returncode
        
        flush_output()
        return return_code == 0
        
    except subprocess.CalledProcessError as e:
        print_with_flush(f"Command failed with return code {e.returncode}")
        return False
    except Exception as e:
        print_with_flush(f"Error running command: {e}")
        return False

def get_python_command():
    """Get the appropriate Python command"""
    if shutil.which('py'):
        return 'py -3'
    elif shutil.which('python3'):
        return 'python3'
    else:
        return 'python'

def print_help():
    print_with_flush()
    print_with_flush(f"{RED}Available options:{NC}")
    print_with_flush()
    print_with_flush(f"{YELLOW}By default, the ACTORS, AUDIO, SYSTEM OVERLAYS, ROM and WAD are built.")
    print_with_flush()
    print_with_flush()
    print_with_flush(f"{YELLOW}-j{NC} -- Only builds the ROM and WAD, skips all other options provided")
    print_with_flush(f"{YELLOW}-s{NC} -- Also rebuilds the scripts using NPC Maker")
    print_with_flush(f"{YELLOW}-n{NC} -- Also rebuilds the snippets and applies patches to code.bin")
    print_with_flush(f"{YELLOW}-a{NC} -- Rebuilds everything")
    print_with_flush(f"{YELLOW}-r{NC} -- Builds the {YELLOW}RELEASE {NC}version (DEBUG is default)")
    print_with_flush(f"{YELLOW}-u{NC} -- Upload to Summercart 64 (Windows only)")
    print_with_flush(f"{YELLOW}-m {GREEN}[number]{NC} -- Enables music debug mode with provided Music ID")

def parse_arguments():
    global run_script_rebuild, run_snippet_rebuild, musid, skip_actors
    global only_rom, BUILD_OPTIONS, deploy, scripts_only
    
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('-j', action='store_true', help='Only builds ROM and WAD')
    parser.add_argument('-s', action='store_true', help='Rebuild scripts')
    parser.add_argument('-n', action='store_true', help='Rebuild snippets')
    parser.add_argument('-a', action='store_true', help='Rebuild everything')
    parser.add_argument('-r', action='store_true', help='Release build')
    parser.add_argument('-m', type=str, help='Music ID for debug mode')
    parser.add_argument('-u', action='store_true', help='Upload to Summercart 64')
    parser.add_argument('-l', type=str, help='Scripts only with file ID')
    parser.add_argument('-h', action='store_true', help='Show help')
    
    args = parser.parse_args()
    
    if args.h:
        print_help()
        sys.exit(0)
    
    if args.j:
        only_rom = True
    
    if args.s:
        run_script_rebuild = True
    
    if args.n:
        run_snippet_rebuild = True
    
    if args.a:
        run_script_rebuild = True
        run_snippet_rebuild = True
    
    if args.r:
        BUILD_OPTIONS = '10 -j "OPTIMIZATION=-Os -DEBUGVER=0"'
    
    if args.m:
        musid = args.m
        skip_actors = True
        if not re.match(r'^\d+$', musid):
            print_with_flush(f"Invalid music ID: {musid}", file=sys.stderr)
            sys.exit(1)
    
    if args.u:
        deploy = True
    
    if args.l:
        scripts_only = True
        musid = args.l
        if not re.match(r'^\d+$', musid):
            print_with_flush(f"Invalid file ID: {musid}", file=sys.stderr)
            sys.exit(1)

def object_concat_process():
    print_with_flush(f"{GREEN}================== Object concat process... =================={NC}")
    
    object_dir = Path("object")
    if object_dir.exists():
        for folder in object_dir.iterdir():
            if folder.is_dir():
                concat_path = folder / "concat"
                if concat_path.exists():
                    python_cmd = get_python_command()
                    zobj_path = folder / "zobj.zobj"
                    cmd = f'{python_cmd} tool/concatFiles.py "{concat_path}" "{zobj_path}"'
                    run_command(cmd)
    
    print_with_flush(f"{GREEN}================== Object concat process complete =================={NC}")
    print_with_flush()

def build_actors():
    print_with_flush()
    print_with_flush(f"{YELLOW}================== Building actors =================={NC}")
    
    original_dir = os.getcwd()
    actor_path = "actor/_custom-1.0/"
    
    try:
        os.chdir(actor_path)
        current_dir = os.getcwd()
        
        # Get all subdirectories
        for item in os.listdir('.'):
            item_path = os.path.join(current_dir, item)
            if os.path.isdir(item_path):
                makefile_path = os.path.join(item_path, 'Makefile')
                if os.path.exists(makefile_path):
                    print_with_flush(f"Building actor in: {item}")
                    os.chdir(item_path)
                    run_command(f"{MAKE_CMD} {BUILD_OPTIONS}")
                    os.chdir(current_dir)  # Go back to actor/_custom-1.0/
        
        os.chdir(original_dir)  # Go back to original directory
        
    except FileNotFoundError as e:
        print_with_flush(f"Error: Could not find directory {actor_path}")
        print_with_flush(f"Current working directory: {os.getcwd()}")
        print_with_flush(f"Directory contents: {os.listdir('.')}")
        os.chdir(original_dir)
        return
    
    print_with_flush(f"{GREEN}================== Actor build complete =================={NC}")
    print_with_flush()

def build_system_overlays():
    global BUILD_OPTIONS
    
    print_with_flush()
    print_with_flush(f"{YELLOW}================== Building system overlays =================={NC}")
    
    os.chdir("system/ovl_opening")
    
    if musid:
        # Remove the closing quote and add MUSID, then add quote back
        BUILD_OPTIONS = BUILD_OPTIONS.rstrip('"') + f' -DMUSID={musid}"'
    
    run_command(f"{MAKE_CMD} {BUILD_OPTIONS}")
    
    os.chdir("../ovl_dummy")
    run_command(MAKE_CMD)
    os.chdir("../..")
    
    print_with_flush(f"{GREEN}================== System overlays build complete =================={NC}")
    print_with_flush()

def build_audio():
    print_with_flush()
    print_with_flush(f"{YELLOW}================== Building audio =================={NC}")
    
    os.chdir("tool")
    python_cmd = get_python_command()
    run_command(f'{python_cmd} "build_audio.py"')
    os.chdir("..")
    
    print_with_flush(f"{GREEN}================== Audio build complete =================={NC}")
    print_with_flush()

def build_snippets():
    print_with_flush()
    print_with_flush(f"{YELLOW}================== Building snippets =================={NC}")
    
    python_cmd = get_python_command()
    
    # Remove existing files and run patcher
    if os.path.exists("oot-1.0-dec.z64"):
        os.remove("oot-1.0-dec.z64")
    run_command(f'{python_cmd} "tool/patcher.py" "assets/oot-1.0.z64" "patch/rom.txt" "oot-1.0-dec.z64"')
    
    if os.path.exists("system/code/code.bin"):
        os.remove("system/code/code.bin")
    run_command(f'{python_cmd} "tool/patcher.py" "system/code/_vanilla-1.0/code.bin" "patch/code.txt" "system/code/code.bin"')
    
    if os.path.exists("system/ovl_title/ovl_title.zovl"):
        os.remove("system/ovl_title/ovl_title.zovl")
    run_command(f'{python_cmd} "tool/patcher.py" "system/ovl_title/_vanilla-1.0/ovl_title.zovl" "patch/z_title.txt" "system/ovl_title/ovl_title.zovl"')
    
    os.chdir("system/code/SNIPPETS")
    
    # Fix for MSYS2: use bash explicitly for shell scripts
    if is_msys2_environment():
        run_command("bash build.sh")
    else:
        # Make sure the script is executable
        if os.path.exists("build.sh"):
            os.chmod("build.sh", 0o755)
        run_command("./build.sh")
    
    os.chdir("../../..")
    
    print_with_flush(f"{GREEN}================== Snippet build complete =================={NC}")
    print_with_flush()

def build_scripts():
    print_with_flush()
    print_with_flush(f"{YELLOW}================== Building scripts =================={NC}")
    
    os.chdir("..")
    
    is_msys2 = is_msys2_environment()
    
    if shutil.which('py') or is_wsl or is_msys2:
        base_cmd = ["./Tool/NPCMAKER/NPC Maker.exe"]
    else:
        base_cmd = ["mono", "./Tool/NPCMAKER/NPC Maker.exe"]
    
    cmd = base_cmd + ["Scripts/Object 120 - Scenes.json", "Build/object/120 - NPCM_Scenes/zobj.zobj"]
    print_with_flush("Building Object 120 - Scenes...")
    run_command(cmd, capture_output=True)

    cmd = base_cmd + ["Scripts/Object 121 - Wanted.json", "Build/object/121 - NPCM_Wanted/zobj.zobj"]
    print_with_flush("Building Object 121 - Wanted...")
    run_command(cmd, capture_output=True)
    
    os.chdir("Build")
    
    print_with_flush(f"{GREEN}================== Script build complete =================={NC}")
    print_with_flush()

def build_rom():
    print_with_flush()
    print_with_flush(f"{YELLOW}================== Building ROM =================={NC}")
    
    is_msys2 = is_msys2_environment()
    
    if is_msys2 or shutil.which('py') or is_wsl:
        rtl_cmd = "./zzrtl-hol.exe"
    else:
        rtl_cmd = "./zzrtl-hol"
    
    # Make sure the executable has proper permissions
    if os.path.exists(rtl_cmd):
        os.chmod(rtl_cmd, 0o755)
    
    try:
        flush_output()
        process = subprocess.Popen([rtl_cmd, "oot_build.rtl"], 
                                 stdin=subprocess.PIPE,
                                 stdout=sys.stdout,  # Let it write directly
                                 stderr=sys.stderr,  # Let it write directly
                                 text=True)
        
        # Send 'y' and newline
        process.stdin.write("y\n")
        process.stdin.flush()
        process.stdin.close()
        
        # Just wait for completion
        process.wait()
        flush_output()
        
    except Exception as e:
        print_with_flush(f"Error running ROM builder: {e}")
        return False
    
    print_with_flush(f"{GREEN}================== ROM built! =================={NC}")
    print_with_flush()


def create_wad():
    print_with_flush()
    print_with_flush(f"{YELLOW}================== Creating WAD =================={NC}")
    
    # Copy the ROM file
    shutil.copy("build-yaz.z64", "build-yaz-cp.z64")
    
    python_cmd = get_python_command().split()
    run_command(python_cmd + ["tool/truncate.py", "build-yaz-cp.z64"])
    
    print_with_flush("Creating WAD file...")
    os.chdir("tool")
    
    is_msys2 = is_msys2_environment()
    
    if shutil.which('py') or is_wsl or is_msys2:
        gzinject_cmd = ["./gzinject/gzinject.exe"]
        rom_file = "../build-yaz-cp.z64"
    else:
        gzinject_cmd = ["wine", "./gzinject/gzinject.exe"]
        rom_file = "../build-yaz.z64"
    
    wad_cmd = gzinject_cmd + [
        "-a", "inject",
        "-w", "gzinject/Zelda Ocarina N64 NTSC.wad",
        "-m", rom_file,
        "-o", "../build-yaz.wad",
        "-i", "NHLE",
        "-r", "1",
        "-p", "gzinject/patches/hol.gzi",
        "-k", "gzinject/common-key.bin",
        "-t", "Hero of Law",
        "--cleanup"
    ]
    
    # Use capture_output=True for gzinject to see its output
    run_command(wad_cmd, capture_output=True)
    os.chdir("..")
    print_with_flush()
    
    # Clean up
    if os.path.exists("build-yaz-cp.z64"):
        os.remove("build-yaz-cp.z64")

def deploy_to_summercart():
    os.chdir("..")
    run_command('./Tool/sc64deployer.exe upload "Build/build-yaz.z64" "--tv=ntsc"', capture_output=True)
    run_command('./Tool/sc64deployer.exe debug --isv 0x3FF0000', capture_output=True)
    os.chdir("Build")

def main():
    global is_wsl
    
    # Force stdout to be line buffered
    sys.stdout.reconfigure(line_buffering=True)
    sys.stderr.reconfigure(line_buffering=True)
    
    is_wsl = is_wsl_environment()
    is_msys2 = is_msys2_environment()
    
    if is_wsl:
        print_with_flush(f"{YELLOW} == WSL MODE == {NC}")
    elif is_msys2:
        print_with_flush(f"{YELLOW} == MSYS2 MODE == {NC}")
    
    parse_arguments()
    
    print_with_flush("Starting build~!")
    
    if not scripts_only:
        object_concat_process()
    
    if not only_rom:
        if not skip_actors and not scripts_only:
            build_actors()
        
        if not scripts_only:
            build_system_overlays()
            build_audio()
        
        if run_snippet_rebuild:
            build_snippets()
        
        if run_script_rebuild or scripts_only:
            build_scripts()
    
    build_rom()
    create_wad()
    
    print_with_flush(f"{RED}~~~~{YELLOW}  Build complete!  {RED}~~~~{NC}")
    
    if deploy:
        deploy_to_summercart()

if __name__ == "__main__":
    main()
