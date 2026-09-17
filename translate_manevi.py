import re
import json
import time
from deep_translator import GoogleTranslator

# Map our lang codes to googletrans lang codes
langs = {
    'en-US': 'en',
    'de-DE': 'de',
    'fr-FR': 'fr',
    'es-ES': 'es',
    'pt-PT': 'pt',
    'is-IS': 'is',
    'ar-SA': 'ar',
    'az-AZ': 'az'
}

with open('rife/manevi_data.js', 'r', encoding='utf-8') as f:
    content = f.read()

# We need to find every occurrence of:
# name: "...",
# and
# desc: "..."
# and inject name_i18n: {...}, and desc_i18n: {...}, right after them.

def translate_text(text):
    results = {}
    for app_lang, g_lang in langs.items():
        try:
            # simple retry logic
            for _ in range(3):
                try:
                    translated = GoogleTranslator(source='tr', target=g_lang).translate(text)
                    results[app_lang] = translated
                    break
                except Exception as e:
                    time.sleep(1)
            else:
                results[app_lang] = text # fallback
        except Exception as e:
            results[app_lang] = text
    return results

def escape_js_string(s):
    # Escape quotes and backslashes
    return s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n')

print("Translating names and descriptions...")
# Find all name: "..." and desc: "..."
# Since some might have commas, we match up to the quote.
# Because regex replacement with async/slow operations in python is easier with re.sub and a function, but re.sub doesn't work well with state if we want to print progress.
# Let's do it manually.

new_content = ""
last_pos = 0

pattern = re.compile(r'(name|desc):\s*"([^"\\]*(?:\\.[^"\\]*)*)"')

count = 0
for match in pattern.finditer(content):
    key = match.group(1)
    val = match.group(2)
    
    # decode JS escaped string for translation
    val_unsec = val.replace('\\"', '"').replace('\\\\', '\\')
    
    count += 1
    print(f"Translating {count} ({key}): {val_unsec[:30]}...")
    
    translations = translate_text(val_unsec)
    
    # Format translations as JS object string
    i18n_str = ", " + key + "_i18n: {" + ", ".join([f'"{k}": "{escape_js_string(v)}"' for k, v in translations.items()]) + "}"
    
    # Append everything up to the match, then the match itself, then the new i18n
    new_content += content[last_pos:match.end()] + i18n_str
    last_pos = match.end()

new_content += content[last_pos:]

with open('rife/manevi_data_translated.js', 'w', encoding='utf-8') as f:
    f.write(new_content)

print("Translation completed. Saved to rife/manevi_data_translated.js")
