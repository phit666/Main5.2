from pathlib import Path
import re

ROOT = Path(".")
EXTS = {".h", ".hpp", ".cpp", ".c", ".cc", ".cxx"}

patterns = [
    r'#\s*include\s*[<"]windows\.h[>"]',
    r'#\s*include\s*[<"]WinSock2\.h[>"]',
    r'#\s*include\s*[<"]winsock\.h[>"]',
    r'#\s*include\s*[<"]ws2tcpip\.h[>"]',
    r'#\s*include\s*[<"]mmsystem\.h[>"]',
    r'#\s*include\s*[<"]shellapi\.h[>"]',
    r'#\s*include\s*[<"]commdlg\.h[>"]',
    r'#\s*include\s*[<"]objbase\.h[>"]',
    r'#\s*include\s*[<"]process\.h[>"]',
    r'#\s*include\s*[<"]io\.h[>"]',
    r'#\s*include\s*[<"]direct\.h[>"]',
]

combined = re.compile("|".join(f"({p})" for p in patterns), re.IGNORECASE)

changed = []

for path in ROOT.rglob("*"):
    if path.suffix.lower() not in EXTS:
        continue

    text = path.read_text(errors="ignore")
    new_text = combined.sub('#include "mu_win_compat.h"', text)

    # remove duplicate consecutive includes
    new_text = re.sub(
        r'(#include "mu_win_compat\.h"\s*)+',
        '#include "mu_win_compat.h"\n',
        new_text
    )

    if new_text != text:
        path.write_text(new_text)
        changed.append(str(path))

print("Changed files:")
for f in changed:
    print(f)

print(f"\nTotal changed: {len(changed)}")