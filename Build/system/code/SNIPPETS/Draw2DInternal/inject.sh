if command -v py >/dev/null 2>&1; then
    py -3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x64650 0x2BA0
else
    python3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x64650 0x2BA0
fi
