import json
import re

manevi_js_path = r'c:\Users\W11\Desktop\z-torus siteden indirildi\rife\manevi_data.js'
with open(manevi_js_path, 'r', encoding='utf-8') as f:
    text = f.read()

# Extract esmaulHusnaData
esma_match = re.search(r'const\s+esmaulHusnaData\s*=\s*(\[.*?\]);', text, re.DOTALL)
manevi_match = re.search(r'const\s+maneviData\s*=\s*(\[.*?\]);', text, re.DOTALL)

print("Esma match found:", bool(esma_match))
print("Manevi match found:", bool(manevi_match))
