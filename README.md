<h1>Hero of Law Source Code</h1>

French Localisation Tracking Spreadsheet : https://docs.google.com/spreadsheets/d/195A2_gr2lPV8qkYqUiV6T7dXbidNvra4HLRoYEGLthQ/edit?gid=0#gid=0

Tested on WSL.

To build a ROM:

1. Install the toolchain:
<pre>

    sudo apt-get install git gcc unzip mercurial automake libelf-dev libperl-dev libgtk2.0-dev libgmp-dev libmpfr-dev

    git clone https://github.com/glankk/n64.git

    cd n64

    ./install_deps

    ./configure --prefix=/opt/n64 --enable-vc

    sudo make all-toolchain

    sudo make install-toolchain

    sudo make install-sys

    sudo rm -rf /opt/n64/mips64/include/z64hdr

    git clone https://github.com/z64tools/z64hdr.git 

    sudo mkdir /opt/n64/mips64/include/z64hdr 

    sudo mv -f z64hdr/* /opt/n64/mips64/include/z64hdr && sudo rm -rf z64hdr

    cd ../

    git clone https://github.com/z64utils/nOVL.git

    cd nOVL

    gcc -o novl -s -Os -DNOVL_DEBUG=1 -flto src/*.c `pkg-config --cflags --libs libelf glib-2.0`

    sudo mv novl /opt/n64/bin/novl
</pre>
        
3. Run <code>git clone https://github.com/Newer-Team/Hero-of-Law.git</code>
4. Go into /Build and place <code>oot-1.0-dec.z64</code> there. This has to be a 1.0 US Zelda Ocarina of Time ROM, decompressed.
5. Run <code>python3 prepare.py</code>
6. Get <a href="https://github.com/skawo/OoT-NPC-Maker/releases/tag/v.3.65.617">this</a> release of NPC Maker. Copy the NPCMaker 3.65 folder from within it to Tool/ and rename it to "NPCMAKER".
7. [Optional] Obtain a WAD of Hero of Law by patching an Ocarina of Time WAD on <a href="https://newerteam.com/hol/">our website</a> and put it into <code>/Build/tool/gzinject</code> as "Zelda Ocarina N64 NTSC.wad"
8. [Optional] Obtain Wii common key and put it as <code>common_key.bin</code> in <code>/Build/tool/gzinject</code>.
9. Run <code>./rebuild.sh -a -r</code>
