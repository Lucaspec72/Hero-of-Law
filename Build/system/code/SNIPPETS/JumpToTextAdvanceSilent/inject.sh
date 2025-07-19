if command -v py >/dev/null 2>&1; then
    py -3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0xC5130 0x48
else
    python3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0xC5130 0x48
fi
