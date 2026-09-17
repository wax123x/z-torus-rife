import json

with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_programs.json', 'r', encoding='utf-8') as f:
    manevi_list = json.load(f)

# Convert to JS code definition
js_code = "const MANEVI_PROGRAMS = " + json.dumps(manevi_list, ensure_ascii=False, indent=2) + ";\n"

with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_code.js', 'w', encoding='utf-8') as out:
    out.write(js_code)

print("Generated manevi_code.js successfully!")
