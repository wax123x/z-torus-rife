import json
import re

manevi_js_path = r'c:\Users\W11\Desktop\z-torus siteden indirildi\rife\manevi_data.js'
with open(manevi_js_path, 'r', encoding='utf-8') as f:
    text = f.read()

# We can evaluate or parse JS object literals
# Simple regex extractor for objects
def extract_objects(js_str):
    # Find all { ... } blocks
    blocks = re.findall(r'\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}', js_str, re.DOTALL)
    return blocks

esma_text = re.search(r'const\s+esmaulHusnaData\s*=\s*(\[.*?\]);', text, re.DOTALL).group(1)
manevi_text = re.search(r'const\s+maneviData\s*=\s*(\[.*?\]);', text, re.DOTALL).group(1)

manevi_list = []

# Parse Esmaul Husna (99 names)
# Each has: name, freq, duration, desc
esma_items = re.findall(r'\{\s*name:\s*"([^"]+)",\s*freq:\s*([\d.]+),\s*duration:\s*([\d.]+),\s*desc:\s*"([^"]+)"\s*\}', esma_text)

idx = 0
for name, freq, dur, desc in esma_items:
    manevi_list.append({
        "id": f"manevi_esma_{idx}",
        "cat": "manevi",
        "emoji": "✨",
        "name": name,
        "source": "Esmaül Hüsna",
        "desc": desc,
        "freqs": [{"hz": float(freq), "min": 5.0}]
    })
    idx += 1

print(f"Parsed {len(manevi_list)} Esmaul Husna items.")

# Parse Manevi Data (Surahs)
# We can extract each object block in manevi_text
surah_blocks = extract_objects(manevi_text)

surah_idx = 0
for b in surah_blocks:
    if "isCategory: true" in b:
        continue # skip category header
    
    name_m = re.search(r'name:\s*"([^"]+)"', b)
    freq_m = re.search(r'freq:\s*([\d.]+)', b)
    desc_m = re.search(r'desc:\s*"([^"]+)"', b)
    seq_m = re.search(r'sequence:\s*\[(.*?)\]', b, re.DOTALL)
    
    if name_m and desc_m:
        name = name_m.group(1)
        desc = desc_m.group(1)
        freqs_arr = []
        
        if seq_m:
            seq_items = re.findall(r'\{\s*freq:\s*([\d.]+),\s*duration:\s*([\d.]+)\s*\}', seq_m.group(1))
            for f_val, d_val in seq_items:
                # Convert duration seconds to minutes (minimum 1 min for UI display)
                mins = round(float(d_val) / 60.0, 1)
                if mins <= 0: mins = 0.5
                freqs_arr.append({"hz": float(f_val), "min": mins})
        elif freq_m:
            freqs_arr.append({"hz": float(freq_m.group(1)), "min": 5.0})
            
        if freqs_arr:
            manevi_list.append({
                "id": f"manevi_surah_{surah_idx}",
                "cat": "manevi",
                "emoji": "📖",
                "name": name,
                "source": "Kuran-ı Kerim / Manevi",
                "desc": desc,
                "freqs": freqs_arr
            })
            surah_idx += 1

print(f"Total Manevi Programs Parsed: {len(manevi_list)}")

with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_programs.json', 'w', encoding='utf-8') as out:
    json.dump(manevi_list, out, ensure_ascii=False, indent=2)

print("Saved to scratch/manevi_programs.json")
