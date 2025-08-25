from __future__ import annotations

from dataclasses import dataclass, field
from itertools import zip_longest
import json
from pathlib import Path
import sys


TESTIMONY_CROSS_EXAM_PAIRS = [
    ('SCENE_IMPA_1', 'SCENE_IMPA_CROSSEXAM'),
    ('SCENE_ZELDA_1', 'SCENE_ZELDA_1_CROSSEXAM'),
    ('SCENE_ZELDA_2', 'SCENE_ZELDA_2_CROSSEXAM'),
    ('SCENE_ZELDA_3', 'SCENE_ZELDA_3_CROSSEXAM'),
    ('SCENE_INGO_1', 'SCENE_INGO_1_CROSSEXAM'),
    ('SCENE_INGO_2', 'SCENE_INGO_2_CROSSEXAM'),
    ('SCENE_INGO_3', 'SCENE_INGO_3_CROSSEXAM'),
    ('SCENE_THIEF_SHOPK_TESTIMONY', 'SCENE_THIEF_SHOPK_CROSSEXAM'),
    ('SCENE_THIEF_OWL_TESTIMONY', 'SCENE_THIEF_OWL_CROSSEXAM'),
]

DIFFERENTLY_NAMED_TESTIMONY_CE_MESSAGE_PAIRS = {
    'SCENE_ZELDA_2': {'ZELDA4': 'ZELDA5'},
    'SCENE_INGO_1': {'INGO5': 'INGO6'},
}

IGNORE_TESTIMONY_TAGS = [
    '<DI>',
    '<DC>',
    '<PAUSE>',
    *(f'<PAUSE:{n}>' for n in range(20)),
    *(f'<SPEED:{n}>' for n in range(200)),
    '<SOUND:CLUE>',
    '<SOUND:EXPLOSION>',
    '<SOUND:HINT>',
    '<SOUND:OWLHOOT>',
    '<SOUND:STAB>',
    '<SOUND:WHAP>',
    '<SQUARE>',
]

@dataclass
class MessagePairDef:
    scene_1_name: str
    message_1_name: str
    scene_2_name: str
    message_2_name: str
    replacements: dict[str, str] = field(default_factory=dict)
    ignore_linebreaks: bool = False

MANUAL_MESSAGE_PAIRS = [
    # System
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'WiiVCNotice', 'SCENE_THIEF_TRSPAKCHECK', 'WiiUVCNotice',
        {'Wii': 'Wii U'}),
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'WiiVCNotice', 'SCENE_THIEF_TRSPAKCHECK', 'GCNNotice',
        {'Wii Virtual Console emulator': "<R>Legend of Zelda: Collector's\nEdition<W> emulator for Nintendo GameCube",
         'la Console Virtuelle Wii': "<R>The Legend of Zelda: Collector's\nEdition<W>"},
        ignore_linebreaks=True),
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'NoThiefContinue', 'SCENE_THIEF_TRSPAKCHECK', 'NoThiefNoSave',
        {'Save in slot $': 'Defaults'}),
    MessagePairDef('SCENE_THIEF_TRSPAKCHECK', 'NoPakContinue', 'SCENE_THIEF_TRSPAKCHECK', 'NoPak',
        {'\n<NEW_BOX>\nContinue anyway?\n(Default name will be used.)\n<TWO_CHOICES><G>Yes\nNo<W>': '<SQUARE>',
         '\n<NEW_BOX>\nContinuer malgré tout ?\n(Le nom par défaut sera utilisé.)\n<TWO_CHOICES><G>Oui\nNon<W>': '<SQUARE>'}),

    # Court Record
    MessagePairDef('OBJ_COURT_RECORD_DATA', 'Koholink', 'OBJ_COURT_RECORD_DATA', 'Koholink2',
        {'THIEF': '<FONT_TOGGLE>$$$$$<FONT_TOGGLE>',
         'VOYOU': '<FONT_TOGGLE>$$$$$<FONT_TOGGLE>'}),
    MessagePairDef('OBJ_COURT_RECORD_DATA', 'Owl', 'OBJ_COURT_RECORD_DATA', 'Owl2',
        {'THIEF': '<FONT_TOGGLE>$$$$$<FONT_TOGGLE>',
         'VOYOU': '<FONT_TOGGLE>$$$$$<FONT_TOGGLE>'}),

    # Bad ending
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE60', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE40'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE70', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE50'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_JUDGE50', 'SCENE_ZELDA_3_CROSSEXAM', 'JUDGE50'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_LINK40', 'SCENE_ZELDA_3_CROSSEXAM', 'LINK30'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_LINK50', 'SCENE_ZELDA_3_CROSSEXAM', 'LINK40'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_JUDGE60', 'SCENE_ZELDA_3_CROSSEXAM', 'JUDGE60'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_JUDGE70', 'SCENE_ZELDA_3_CROSSEXAM', 'JUDGE70'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE80', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE60'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE85', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE70'),
    MessagePairDef('SCENE_ZELDAS_NEW_EVIDENCE', '1_DRAGMIRE90', 'SCENE_ZELDA_3_CROSSEXAM', 'DRAGMIRE80'),

    # Flashbacks
    MessagePairDef('SCENE_THIEF_TRIALSTART', 'JUDGE140', 'SCENE_THIEF_VERDICT', 'JUDGE_FLSH'),
    MessagePairDef('SCENE_THIEF_OWL_TESTIMONY', 'OWL3', 'SCENE_THIEF_OWL_CROSSEXAM', 'OWLFL10'),

    # Antipiracy
    MessagePairDef('SCENE_ANTIPIRACY_EDGB', 'JUDGE10', 'SCENE_ANTIPIRACY_EZJR', 'JUDGE10'),
    MessagePairDef('SCENE_ANTIPIRACY_EDGB', 'JUDGE20', 'SCENE_ANTIPIRACY_EZJR', 'JUDGE20'),
    MessagePairDef('SCENE_ANTIPIRACY_EDGB', 'JUDGE30', 'SCENE_ANTIPIRACY_EZJR', 'JUDGE30'),

    # Other
    MessagePairDef('SCENE_THINKING_RANCH', 'MustRethink', 'SCENE_THINKING_THIEF', 'MustRethink'),
]


class Message:
    scene: Scene
    default_data: dict
    localization_data: dict

    def __init__(self, scene: Scene, data: dict, localization_data: dict):
        self.scene = scene
        self.data = data
        self.localization_data = localization_data

    @property
    def name(self) -> str:
        return self.data['Name']

    def text_lines(self, language: str | None) -> list[str]:
        if language is None:
            return self.data['MessageTextLines']
        else:
            return self.localization_data[language]['MessageTextLines']

    def text(self, language: str | None) -> str:
        return '\n'.join(self.text_lines(language))

    def available_languages(self) -> set[str | None]:
        return set(self.localization_data) | {None}


class Scene:
    data: dict

    def __init__(self, data: dict):
        self.data = data

    @property
    def name(self) -> str:
        return self.data['NPCName']

    _messages = None
    @property
    def messages(self) -> list[Message]:
        if self._messages is None:
            localizations_map = {}
            for localization in self.data['Localization']:
                localization_name = localization['Language']
                for m in localization['Messages']:
                    msg_name = m['Name']
                    localizations_map.setdefault(msg_name, {})
                    localizations_map[msg_name][localization_name] = m

            self._messages = [
                Message(self, m, localizations_map.get(m['Name'], {}))
                for m in self.data['Messages']
            ]

        return self._messages

    def get_message(self, name: str) -> Message:
        for message in self.messages:
            if message.name == name:
                return message

        raise KeyError(f'message "{name}" not found')

    def available_languages(self) -> set[str | None]:
        langs = set()
        for m in self.messages:
            langs |= m.available_languages()
        return langs


def print_message_names_line(
    message_1: Message,
    message_2: Message,
) -> None:
    if message_1.scene.name == message_2.scene.name:
        first_half = message_1.scene.name
    else:
        first_half = f'{message_1.scene.name} + {message_2.scene.name}'

    if message_1.name == message_2.name:
        second_half = message_1.name
    else:
        second_half = f'{message_1.name} + {message_2.name}'

    print(f'{first_half} / {second_half}')


class MessageComparison:
    def __init__(
        self,
        title_1: str,
        message_1: Message,
        title_2: str,
        message_2: Message,
        language: str | None,
        replacements: dict[str, str] | None = None,
        *,
        ignore_linebreaks: bool = False,
        is_testimony_and_ce: bool = False,
    ):
        self.title_1 = title_1
        self.message_1 = message_1
        self.title_2 = title_2
        self.message_2 = message_2
        self.language = language
        self.replacements = replacements
        self.ignore_linebreaks = ignore_linebreaks
        self.is_testimony_and_ce = is_testimony_and_ce

        self.do_comparison()

    def do_comparison(self) -> None:
        text_1 = self.message_1.text(self.language)
        text_2 = self.message_2.text(self.language)

        if self.replacements:
            for key, value in self.replacements.items():
                text_1 = text_1.replace(key, value)

        self.is_match = True
        self.issue = ''

        if self.is_testimony_and_ce:
            text_1_clean = text_1.removesuffix('<NS>')
            for to_remove in IGNORE_TESTIMONY_TAGS:
                text_1_clean = text_1_clean.replace(to_remove, '')
            text_1_clean = text_1_clean.replace('<W>', '<G>')

            text_2_clean = text_2

            if not text_2_clean.startswith(('<DI><G>', '<G><DI>')):
                is_match = False
                self.issue = 'CE text should start with <DI><G> or <G><DI>'
            text_2_clean = text_2_clean.removeprefix('<DI><G>').removeprefix('<G><DI>')

            if not text_2_clean.endswith('<PERSISTENT>'):
                is_match = False
                self.issue = 'CE text should end with <PERSISTENT>'
            text_2_clean = text_2_clean.removesuffix('<PERSISTENT>')

            if '<sound' in text_2_clean.lower():
                is_match = False
                self.issue = 'CE text should not include <SOUND> tags'

            self.text_1_clean = '\n'.join(line.strip() for line in text_1_clean.splitlines())
            self.text_2_clean = '\n'.join(line.strip() for line in text_2_clean.splitlines())

        else:
            self.text_1_clean = text_1
            self.text_2_clean = text_2

        if self.ignore_linebreaks:
            self.text_1_clean = self.text_1_clean.replace('\n', ' ')
            self.text_2_clean = self.text_2_clean.replace('\n', ' ')

        if self.text_1_clean != self.text_2_clean:
            self.is_match = False
            self.issue = "Cleaned text doesn't match"

    def make_comparison_str_list(self, *, indent: int = 0) -> list[str]:
        EACH_WIDTH = 70
        lines = []

        if self.issue:
            lines.append(f'{self.issue}:')

        text_1_lines = self.message_1.text(self.language).splitlines()
        text_2_lines = self.message_2.text(self.language).splitlines()
        text_1_clean_lines = self.text_1_clean.splitlines()
        text_2_clean_lines = self.text_2_clean.splitlines()

        lines.append(f'{self.title_1}: '.ljust(EACH_WIDTH, '-') + '| ' + f'{self.title_1} (cleaned): '.ljust(EACH_WIDTH, '-'))

        for a, b in zip_longest(text_1_lines, text_1_clean_lines, fillvalue=''):
            lines.append(a.ljust(EACH_WIDTH) + '| ' + b)

        lines.append(f'{self.title_2}: '.ljust(EACH_WIDTH, '-') + '| ' + f'{self.title_2} (cleaned): '.ljust(EACH_WIDTH, '-'))

        for a, b in zip_longest(text_2_lines, text_2_clean_lines, fillvalue=''):
            lines.append(a.ljust(EACH_WIDTH) + '| ' + b)

        lines.append('-' * EACH_WIDTH + '| ' + '-' * EACH_WIDTH)

        return [' ' * indent + line for line in lines]

    def make_comparison_str(self, *, indent: int = 0) -> str:
        return '\n'.join(self.make_comparison_str_list(indent=indent))

    def print_comparison_str(self, *, indent: int = 0) -> None:
        print(self.make_comparison_str(indent=indent))


def main() -> None:
    path = Path(sys.argv[1])

    with path.open('r', encoding='utf-8') as f:
        data = json.load(f)

    npcs_by_name = {npc['NPCName']: npc for npc in data['Entries']}

    langs = set()
    for testimony_name, ce_name in TESTIMONY_CROSS_EXAM_PAIRS:
        testimony = Scene(npcs_by_name[testimony_name])
        ce = Scene(npcs_by_name[ce_name])
        langs |= testimony.available_languages()
        langs |= ce.available_languages()
    for pair in MANUAL_MESSAGE_PAIRS:
        scene_1 = Scene(npcs_by_name[pair.scene_1_name])
        scene_2 = Scene(npcs_by_name[pair.scene_2_name])
        langs |= scene_1.available_languages()
        langs |= scene_2.available_languages()
    langs = sorted(langs, key=lambda n: n or '')

    for lang in langs:
        title = f' {lang or "English"} '
        print()
        print(f'{title:=^80}')
        print()

        # Testimony vs. cross-exams
        for testimony_name, ce_name in TESTIMONY_CROSS_EXAM_PAIRS:
            testimony = Scene(npcs_by_name[testimony_name])
            ce = Scene(npcs_by_name[ce_name])

            for testimony_msg in testimony.messages:
                ce_msg_name = DIFFERENTLY_NAMED_TESTIMONY_CE_MESSAGE_PAIRS \
                    .get(testimony.name, {}) \
                    .get(testimony_msg.name, testimony_msg.name)

                for ce_msg in ce.messages:
                    if ce_msg.name == ce_msg_name:
                        is_title = testimony_msg.name == 'Title'

                        print_message_names_line(testimony_msg, ce_msg)

                        comparison = MessageComparison(
                            'Testimony',
                            testimony_msg,
                            'Cross-Exam',
                            ce_msg,
                            lang,
                            is_testimony_and_ce=(not is_title),
                        )
                        if not comparison.is_match:
                            comparison.print_comparison_str(indent=4)

                        break

                else:
                    raise ValueError(f"Couldn't find counterpart of {testimony_msg['Name']} in {ce_name}")

        # Everything else
        for pair in MANUAL_MESSAGE_PAIRS:
            scene_1 = Scene(npcs_by_name[pair.scene_1_name])
            scene_2 = Scene(npcs_by_name[pair.scene_2_name])
            message_1 = scene_1.get_message(pair.message_1_name)
            message_2 = scene_2.get_message(pair.message_2_name)

            print_message_names_line(message_1, message_2)

            comparison = MessageComparison(
                'Message 1',
                message_1,
                'Message 2',
                message_2,
                lang,
                pair.replacements,
                ignore_linebreaks=pair.ignore_linebreaks,
            )
            if not comparison.is_match:
                comparison.print_comparison_str(indent=4)


if __name__ == '__main__':
    main()
