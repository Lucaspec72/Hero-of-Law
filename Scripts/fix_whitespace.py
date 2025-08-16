import json
from pathlib import Path
import sys

path = Path(sys.argv[1])

with path.open('r', encoding='utf-8') as f:
    start = f.read(1000)
newline = '\n' if '\r\n' not in start else '\r\n'

with path.open('r', encoding='utf-8') as f:
    data = json.load(f)

def fix_whitespace(lines: list[str], *, is_message: bool = False) -> list[str]:

    lines_temp = list(lines)
    num_newlines_at_end = 0
    while lines_temp and not lines_temp[-1]:
        num_newlines_at_end += 1
        lines_temp.pop()

    for i, line in enumerate(lines):
        line = line.rstrip()
        if is_message:
            while '  ' in line:
                line = line.replace('  ', ' ')
        if is_message and '<PAUSE' in line and line.endswith('>') and '>' not in line[line.rfind('<PAUSE'):-1]:
            line += ' '
        lines[i] = line

    while lines and not lines[-1]:
        lines.pop()

    return lines + [''] * num_newlines_at_end

for npc in data['Entries']:
    print(npc['NPCName'])
    for script in npc['Scripts']:
        script['TextLines'] = fix_whitespace(script['TextLines'])
    for message in npc['Messages']:
        message['MessageTextLines'] = fix_whitespace(message['MessageTextLines'], is_message=True)
    for localization in npc['Localization']:
        for message in localization['Messages']:
            message['MessageTextLines'] = fix_whitespace(message['MessageTextLines'], is_message=True)
    npc['EmbeddedOverlayCode']['CodeLines'] = fix_whitespace(npc['EmbeddedOverlayCode']['CodeLines'])
for header in data['GlobalHeaders']:
    print(header['Name'])
    header['TextLines'] = fix_whitespace(header['TextLines'])

if len(sys.argv) > 2:
    out_path = Path(sys.argv[2])
else:
    out_path = path
out_path.write_text(json.dumps(data, indent=2, ensure_ascii=False).replace('\n', newline), encoding='utf-8')
