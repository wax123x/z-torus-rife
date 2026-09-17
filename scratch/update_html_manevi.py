import json

# Read manevi_code.js
with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_code.js', 'r', encoding='utf-8') as f:
    manevi_js_code = f.read()

def update_html_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Insert MANEVI_PROGRAMS right after <script> tag before UNIFIED_PROGRAMS
    old_unified_target = "let UNIFIED_PROGRAMS = ["
    if "const MANEVI_PROGRAMS =" not in content:
        content = content.replace(old_unified_target, manevi_js_code + "\nlet UNIFIED_PROGRAMS = [...MANEVI_PROGRAMS, ")

    # Update loadFullDatabase to prepend MANEVI_PROGRAMS
    old_db_line = "UNIFIED_PROGRAMS = fullDb;"
    new_db_line = "UNIFIED_PROGRAMS = [...MANEVI_PROGRAMS, ...fullDb];"
    content = content.replace(old_db_line, new_db_line)

    # Update renderPrograms category match for manevi to be strictly p.cat === 'manevi'
    old_manevi_match = """    } else if (currentCategory === 'manevi') {
      const maneviKeywords = ['manevi', 'ruhsal', 'zihin', 'esma', 'dua', 'meditasyon', 'cakra', 'aura', 'huzur', 'stres', 'nazar', 'arinma', 'duygusal', 'zihinsel', 'baris', 'sevgi', 'sezgi', 'frekans'];
      const textAll = nameNorm + ' ' + descNorm;
      matchCat = (p.cat === 'manevi') || maneviKeywords.some(kw => textAll.includes(kw));"""
    
    new_manevi_match = """    } else if (currentCategory === 'manevi') {
      matchCat = (p.cat === 'manevi');"""

    content = content.replace(old_manevi_match, new_manevi_match)

    with open(file_path, 'w', encoding='utf-8') as out:
        out.write(content)

    print(f"Updated {file_path} successfully.")

update_html_file(r'c:\Users\W11\Desktop\z-torus siteden indirildi\12\index.html')
update_html_file(r'c:\Users\W11\Desktop\z-torus siteden indirildi\Gokhan\index.html')
