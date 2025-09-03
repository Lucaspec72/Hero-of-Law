from pathlib import Path
import shutil

from gitignore_parser import parse_gitignore  # pip install gitignore-parser


PRIVATE_REPO_ROOT = Path('.')
PUBLIC_REPO_ROOT = Path('../Hero-of-Law')

PRESERVE_WHITELIST_FILE = Path('public_repo_preserve_whitelist.txt')
COPY_BLACKLIST_FILE = Path('public_repo_copy_blacklist.txt')


# NOTE: command for comparing two HoL repos:
# diff -ru --strip-trailing-cr --exclude=.git hol-private Hero-of-Law > diff.txt


def delete(path: Path) -> None:
    if path.is_file():
        path.unlink()
    else:
        shutil.rmtree(path)


def main() -> None:
    should_preserve = parse_gitignore(PRESERVE_WHITELIST_FILE, base_dir=PUBLIC_REPO_ROOT.resolve())
    shouldnt_copy = parse_gitignore(COPY_BLACKLIST_FILE, base_dir=PUBLIC_REPO_ROOT.resolve())

    print('Step 1: delete almost everything from the public repo')
    def do(path: Path) -> None:
        if not should_preserve(path):
            if path.is_file():
                delete(path)
            else:
                for child in path.iterdir():
                    do(child)
                if not list(path.iterdir()):
                    delete(path)

    do(PUBLIC_REPO_ROOT)

    print('Step 2: copy *most* things from the private to the public repo')
    def do(rel_path: Path) -> None:
        private_path = PRIVATE_REPO_ROOT / rel_path
        public_path = PUBLIC_REPO_ROOT / rel_path

        if not should_preserve(public_path) and not shouldnt_copy(public_path):
            if private_path.is_file():
                public_path.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(private_path, public_path)
            else:
                for private_child in private_path.iterdir():
                    rel_child = private_child.relative_to(PRIVATE_REPO_ROOT)
                    do(rel_child)

    do(Path('.'))

    print('Done!')


if __name__ == '__main__':
    main()
