#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

BUILD_OPTIONS='10D -j "OPTIMIZATION=-Os -DDEBUGVER=1"'
run_script_rebuild=false
run_snippet_rebuild=false
musid=""
skip_actors=false
only_rom=false
MAKE_CMD="make clean && make"
is_wsl=false
scripts_only=false

case "$(uname -r)" in
  *microsoft*|*WSL*) is_wsl=true ;;
  *)
    if grep -qi "microsoft" /proc/version 2>/dev/null || [ -n "$WSL_DISTRO_NAME" ]; then
      is_wsl=true
    fi
    ;;
esac

if [ "$is_wsl" = true ]; then 
    printf "${YELLOW} == WSL MODE == ${NC}\n"
fi

# Parse command line options
while getopts "jsnarm:uhl:" opt; do
  case $opt in
    j)
      only_rom=true
      ;;
    s)
      run_script_rebuild=true
      ;;
    n)
      run_snippet_rebuild=true
      ;;
    a)
      run_script_rebuild=true
      run_snippet_rebuild=true
      add_clean=true
      ;;     
    r)
      BUILD_OPTIONS='10 -j "OPTIMIZATION=-Os -DEBUGVER=0"'
      ;;
    m)
      musid="$OPTARG"
      skip_actors=true
      if [[ ! "$musid" =~ ^[0-9]+$ ]]; then
        printf "Invalid music ID: %s\n" "$musid" >&2
        exit 1
      fi
      ;;   
    u)
      deploy=true     
      ;;
    l)
      scripts_only=true 
      ;;         
    h)
      printf "\n"
      printf "${RED}Available options:${NC}\n"
      printf "\n"
      printf "${YELLOW}By default, the ACTORS, AUDIO, SYSTEM OVERLAYS, ROM and WAD are built."
      printf "\n"
      printf "\n"
      printf "${YELLOW}-j${NC} -- Only builds the ROM and WAD, skips all other options provided\n"
      printf "${YELLOW}-s${NC} -- Also rebuilds the scripts using NPC Maker\n"
      printf "${YELLOW}-n${NC} -- Also rebuilds the snippets and applies patches to code.bin\n"
      printf "${YELLOW}-a${NC} -- Rebuilds everything\n"
      printf "${YELLOW}-r${NC} -- Builds the ${YELLOW}RELEASE ${NC}version (DEBUG is default)\n"
      printf "${YELLOW}-u${NC} -- Upload to Summercart 64 (Windows only)\n"
      printf "${YELLOW}-m ${GREEN}[number]${NC} -- Enables music debug mode with provided Music ID\n"
      exit 1
      ;;
    \?)
      printf "\n"
      printf "Invalid option: -%s\n" "$OPTARG" >&2
      printf "See -h for available options.\n"
      exit 1
      ;;
  esac
done

printf "Starting build~!\n"

if [ "$scripts_only" = false ]; then 

    printf "${GREEN}================== Object concat process... ==================${NC}\n"

    # Go through every folder in the object directory
    for folder in object/*/; do
        # Check if it's actually a directory
        if [ -d "$folder" ]; then

            folder_name=$(basename "$folder")
            concat_path="$folder/concat"
            
            # Check if the concat subfolder exists
            if [ -d "$concat_path" ]; then
            
                if command -v py >/dev/null 2>&1; then
                    py -3 tool/concatFiles.py "$concat_path" "$folder/zobj.zobj";
                else
                    python3 tool/concatFiles.py "$concat_path" "$folder/zobj.zobj";
                fi
            fi
        fi
    done
    
    printf "${GREEN}================== Object concat process complete ==================${NC}\n"
    printf "\n"        
fi

if [ "$only_rom" = false ]; then 

    if [ "$skip_actors" = false ] && [ "$scripts_only" = false ]; then 
        printf "\n"
        printf "${YELLOW}================== Building actors ==================${NC}\n"
        cd "actor/_custom-1.0/" || exit 1

        for dir in */; do
        (
          cd "$dir" || exit 1
          if [ ! -f Makefile ]; then
            exit 0
          fi
          eval "$MAKE_CMD $BUILD_OPTIONS"
        ) &
        done
        wait
        printf "${GREEN}================== Actor build complete ==================${NC}\n"
        printf "\n"
        cd ../..
    fi
       
    if [ "$scripts_only" = false ]; then 

        printf "\n"
        printf "${YELLOW}================== Building system overlays ==================${NC}\n"
        cd system/ovl_opening

        if [ -n "$musid" ]; then
            BUILD_OPTIONS="${BUILD_OPTIONS%\"} -DMUSID=${musid}\""
        fi
        
        eval "$MAKE_CMD $BUILD_OPTIONS"
        cd ../ovl_dummy
        eval "$MAKE_CMD"
        cd ../..

        printf "${GREEN}================== System overlays build complete ==================${NC}\n"
        printf "\n"
    
    fi
    
    
    if [ "$scripts_only" = false ]; then 

        printf "\n"
        printf "${YELLOW}================== Building audio ==================${NC}\n"
        cd tool
        if command -v py >/dev/null 2>&1; then
            py -3 "build_audio.py"
        else
            python3 "build_audio.py"
        fi
        cd ..
        printf "${GREEN}================== Audio build complete ==================${NC}\n"
        printf "\n"
    
    fi
    

    if [ "$run_snippet_rebuild" = true ]; then
        printf "\n"
        printf "${YELLOW}================== Building snippets ==================${NC}\n"
        
        rm -f "oot-1.0-dec.z64"
        if command -v py >/dev/null 2>&1; then
            py -3 "tool//patcher.py" "assets/oot-1.0.z64" "patch//rom.txt" "oot-1.0-dec.z64"
        else
            python3 "tool//patcher.py" "assets/oot-1.0.z64" "patch//rom.txt" "oot-1.0-dec.z64"
        fi

        rm -f "system//code//code.bin"
        if command -v py >/dev/null 2>&1; then
            py -3 "tool//patcher.py" "system//code//_vanilla-1.0//code.bin" "patch//code.txt" "system//code//code.bin"
        else
            python3 "tool//patcher.py" "system//code//_vanilla-1.0//code.bin" "patch//code.txt" "system//code//code.bin"
        fi

        rm -f "system//ovl_title//ovl_title.zovl"
        if command -v py >/dev/null 2>&1; then
            py -3 "tool//patcher.py" "system//ovl_title//_vanilla-1.0//ovl_title.zovl" "patch//z_title.txt" "system//ovl_title//ovl_title.zovl"
        else
            python3 "tool//patcher.py" "system//ovl_title//_vanilla-1.0//ovl_title.zovl" "patch//z_title.txt" "system//ovl_title//ovl_title.zovl"
        fi

        cd "system//code//SNIPPETS"
        ./build.sh 
        cd ../../..
        
        printf "${GREEN}================== Snippet build complete ==================${NC}\n"
        printf "\n"
    fi
    

    if [ "$run_script_rebuild" = true ] || [ "$scripts_only" = true ]; then
        printf "\n"
        printf "${YELLOW}================== Building scripts ==================${NC}\n"
        
        cd ..
        if command -v py >/dev/null 2>&1 || [ "$is_wsl" = true ]; then
            ./Tool/NPCMAKER/NPC\ Maker.exe "Scripts/Object 120 - Scenes.json" "Build/object/120 - NPCM_Scenes/zobj.zobj"
            ./Tool/NPCMAKER/NPC\ Maker.exe "Scripts/Object 121 - Wanted.json" "Build/object/121 - NPCM_Wanted/zobj.zobj"
        else
            mono ./Tool/NPCMAKER/NPC\ Maker.exe "Scripts/Object 120 - Scenes.json" "Build/object/120 - NPCM_Scenes/zobj.zobj"              
            mono ./Tool/NPCMAKER/NPC\ Maker.exe "Scripts/Object 121 - Wanted.json" "Build/object/121 - NPCM_Wanted/zobj.zobj"
        fi
        cd Build
        
        printf "${GREEN}================== Script build complete ==================${NC}\n"
        printf "\n"
    fi
fi




printf "\n"
printf "${YELLOW}================== Building ROM ==================${NC}\n"

if command -v py >/dev/null 2>&1 || [ "$is_wsl" = true ]; then
    printf "y\n" | ./zzrtl-hol.exe oot_build.rtl
else
    printf "y\n" | ./zzrtl-hol oot_build.rtl
fi

printf "${GREEN}================== ROM built! ==================${NC}\n"
printf "\n"





printf "\n"
printf "${YELLOW}================== Creating WAD ==================${NC}\n"

cp build-yaz.z64 build-yaz-cp.z64

if command -v py >/dev/null 2>&1; then
    py -3 "tool//truncate.py" "build-yaz-cp.z64"
else
    python3 "tool//truncate.py" "build-yaz-cp.z64"
fi

printf "Creating WAD file...\n"
cd tool
if command -v py >/dev/null 2>&1 || [ "$is_wsl" = true ]; then
    ./gzinject/gzinject.exe -a inject -w "gzinject/Zelda Ocarina N64 NTSC.wad" -m "../build-yaz-cp.z64" -o "../build-yaz.wad" -i "NHLE" -r 1 -p "gzinject/patches/hol.gzi" -k "gzinject/common-key.bin" -t "Hero of Law" --cleanup
else
    # There's a native Linux build of gzinject in this repo, but it
    # seems to be bugged and not apply .gzi patches correctly, so we
    # need to use the Windows build with Wine instead
    wine ./gzinject/gzinject.exe -a inject -w "gzinject/Zelda Ocarina N64 NTSC.wad" -m "../build-yaz.z64" -o "../build-yaz.wad" -i "NHLE" -r 1 -p "gzinject/patches/hol.gzi" -k "gzinject/common-key.bin" -t "Hero of Law" --cleanup
fi
cd ..
printf "\n"

rm "build-yaz-cp.z64"

printf "${RED}~~~~${YELLOW}  Build complete!  ${RED}~~~~${NC}\n"

if [ "$deploy" = true ]; then
    cd ..
    ./Tool/sc64deployer.exe upload "Build/build-yaz.z64" "--tv=ntsc" 
    
#"-tsram" -s "Build/build-yaz.sav"     
    
    ./Tool/sc64deployer.exe debug --isv 0x3FF0000
    cd Build
fi