if command -v py >/dev/null 2>&1; then
    py -3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x79F18 0x480
else
    python3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x79F18 0x480
fi
