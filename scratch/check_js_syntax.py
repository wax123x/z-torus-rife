with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\12\index.html', 'r', encoding='utf-8') as f:
    code = f.read()

import re
scripts = re.findall(r'<script>(.*?)</script>', code, re.DOTALL)
print(f"Found {len(scripts)} script blocks.")

for i, s in enumerate(scripts):
    try:
        # Check basic bracket matching
        open_b = s.count('{')
        close_b = s.count('}')
        open_p = s.count('(')
        close_p = s.count(')')
        open_sq = s.count('[')
        close_sq = s.count(']')
        print(f"Script {i}: {{: {open_b}/{close_b}, (: {open_p}/{close_p}, [: {open_sq}/{close_sq}")
    except Exception as e:
        print(f"Script {i} error:", e)
