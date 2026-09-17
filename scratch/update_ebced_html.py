import re

with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_code.js', 'r', encoding='utf-8') as f:
    manevi_js_code = f.read().strip()

def update_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Pattern to match const MANEVI_PROGRAMS = [ ... ];
    pattern = r'const\s+MANEVI_PROGRAMS\s*=\s*\[.*?\n\];'
    
    if re.search(pattern, content, re.DOTALL):
        content = re.sub(pattern, manevi_js_code, content, flags=re.DOTALL)
        with open(file_path, 'w', encoding='utf-8') as out:
            out.write(content)
        print(f"Updated {file_path} with exact Ebced durations.")
    else:
        print(f"Pattern not found in {file_path}")

update_file(r'c:\Users\W11\Desktop\z-torus siteden indirildi\12\index.html')
update_file(r'c:\Users\W11\Desktop\z-torus siteden indirildi\Gokhan\index.html')
