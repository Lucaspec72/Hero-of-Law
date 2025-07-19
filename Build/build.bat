echo off

echo y | zzrtl-hol.exe oot_build.rtl

cd tool
echo Building WAD...
cd gzinject
gzinject.exe -a inject -w "Zelda Ocarina N64 NTSC.wad" -m "../../build-yaz.z64" -o "../../build-yaz.wad" -i "NHLE" -r 1 -p "patches/hol.gzi" -k "common-key.bin" -t "Hero of Law" --cleanup

cd ..
cd ..

echo All done!
