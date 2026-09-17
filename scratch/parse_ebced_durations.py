import json
import re

manevi_js_path = r'c:\Users\W11\Desktop\z-torus siteden indirildi\rife\manevi_data.js'
with open(manevi_js_path, 'r', encoding='utf-8') as f:
    text = f.read()

def extract_objects(js_str):
    blocks = re.findall(r'\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}', js_str, re.DOTALL)
    return blocks

esma_text = re.search(r'const\s+esmaulHusnaData\s*=\s*(\[.*?\]);', text, re.DOTALL).group(1)
manevi_text = re.search(r'const\s+maneviData\s*=\s*(\[.*?\]);', text, re.DOTALL).group(1)

manevi_list = []

# Parse Esmaul Husna (99 names) using exact duration in seconds converted to minutes
esma_items = re.findall(r'\{\s*name:\s*"([^"]+)",\s*freq:\s*([\d.]+),\s*duration:\s*([\d.]+),\s*desc:\s*"([^"]+)"\s*\}', esma_text)

idx = 0
for name, freq, dur, desc in esma_items:
    dur_sec = float(dur)
    dur_min = round(dur_sec / 60.0, 3) # Exact duration in minutes from Ebced seconds!
    manevi_list.append({
        "id": f"manevi_esma_{idx}",
        "cat": "manevi",
        "emoji": "✨",
        "name": name,
        "source": "Esmaül Hüsna",
        "desc": desc,
        "freqs": [{"hz": float(freq), "min": dur_min}]
    })
    idx += 1

print(f"Parsed {len(manevi_list)} Esmaul Husna items with exact Ebced durations.")

# Parse Surahs
surah_blocks = extract_objects(manevi_text)
surah_idx = 0
for b in surah_blocks:
    if "isCategory: true" in b:
        continue
    
    name_m = re.search(r'name:\s*"([^"]+)"', b)
    freq_m = re.search(r'freq:\s*([\d.]+)', b)
    dur_m = re.search(r'duration:\s*([\d.]+)', b)
    desc_m = re.search(r'desc:\s*"([^"]+)"', b)
    seq_m = re.search(r'sequence:\s*\[(.*?)\]', b, re.DOTALL)
    
    if name_m and desc_m:
        name = name_m.group(1)
        desc = desc_m.group(1)
        freqs_arr = []
        
        if seq_m:
            seq_items = re.findall(r'\{\s*freq:\s*([\d.]+),\s*duration:\s*([\d.]+)\s*\}', seq_m.group(1))
            for f_val, d_val in seq_items:
                d_sec = float(d_val)
                mins = round(d_sec / 60.0, 3)
                freqs_arr.append({"hz": float(f_val), "min": mins})
        elif freq_m and dur_m:
            d_sec = float(dur_m.group(1))
            mins = round(d_sec / 60.0, 3)
            freqs_arr.append({"hz": float(freq_m.group(1)), "min": mins})
            
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

print(f"Total Manevi Programs: {len(manevi_list)}")

with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_programs_ebced.json', 'w', encoding='utf-8') as out:
    json.dump(manevi_list, out, ensure_ascii=False, indent=2)

js_code = "const MANEVI_PROGRAMS = " + json.dumps(manevi_list, ensure_ascii=False, indent=2) + ";\n"
with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_code.js', 'w', encoding='utf-8') as out:
    out.write(js_code)

print("Generated manevi_code.js with exact Ebced durations!")
