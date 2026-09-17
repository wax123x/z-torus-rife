import os
import re
import json
import ast
import base64
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
import hashlib

def evp_kdf(passphrase, salt, key_len=32, iv_len=16):
    d = b''
    d_i = b''
    while len(d) < key_len + iv_len:
        d_i = hashlib.md5(d_i + passphrase + salt).digest()
        d += d_i
    return d[:key_len], d[key_len:key_len + iv_len]

def decrypt_cryptojs(enc_str, passphrase):
    raw = base64.b64decode(enc_str.strip())
    salt = raw[8:16]
    ciphertext = raw[16:]
    key, iv = evp_kdf(passphrase.encode('utf-8'), salt)
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
    decryptor = cipher.decryptor()
    padded = decryptor.update(ciphertext) + decryptor.finalize()
    pad_len = padded[-1]
    return padded[:-pad_len].decode('utf-8')

def clean_name(n):
    if ':' in n and len(n) > 50:
        return n.split(':')[0].strip()
    return n.strip()

def main():
    passphrase = 'Z-Torus-Pro-Security-Key-2026'

    base_dir = r'c:\Users\W11\Desktop\z-torus siteden indirildi'

    with open(os.path.join(base_dir, 'rodin/data/lists_encrypted.txt'), 'r', encoding='utf-8') as f:
        tr_lists = json.loads(decrypt_cryptojs(f.read(), passphrase))

    with open(os.path.join(base_dir, 'rodin/data/lists_encrypted_en.txt'), 'r', encoding='utf-8') as f:
        en_lists = json.loads(decrypt_cryptojs(f.read(), passphrase))

    with open(os.path.join(base_dir, 'rodin/data/descriptions_encrypted.txt'), 'r', encoding='utf-8') as f:
        tr_descs = json.loads(decrypt_cryptojs(f.read(), passphrase))

    with open(os.path.join(base_dir, 'rodin/data/descriptions_encrypted_en.txt'), 'r', encoding='utf-8') as f:
        en_descs = json.loads(decrypt_cryptojs(f.read(), passphrase))

    # Build mapping tables for Rife/PEMF DB
    tr_map_name = {}
    tr_map_freqs = {}
    for i, item in enumerate(tr_lists):
        en_item = en_lists[i] if i < len(en_lists) else {}
        raw_tr_name = item.get('name', '')
        tr_name = clean_name(raw_tr_name)
        en_name = clean_name(en_item.get('name', ''))
        
        tr_d = tr_descs.get(tr_name, '')
        en_d = en_descs.get(en_name, '') or en_descs.get(tr_name, '')
        
        val = (en_name, en_d)
        tr_map_name[tr_name.lower()] = val
        freqs = tuple(item.get('frequencies', []))
        tr_map_freqs[freqs] = val

    # 1. Update 11/data/frequency_db.json
    db_path = os.path.join(base_dir, '11/data/frequency_db.json')
    with open(db_path, 'r', encoding='utf-8') as f:
        db = json.load(f)

    updated_db = 0
    for p in db:
        name = p['name'].strip()
        freqs = tuple(f['hz'] for f in p['freqs'])
        
        found = tr_map_name.get(name.lower()) or tr_map_freqs.get(freqs)
        if found:
            en_name, en_d = found
            p['name_en'] = en_name
            if en_d:
                p['desc_en'] = en_d
            else:
                p['desc_en'] = p.get('desc', '')
        else:
            if p['id'] == 'prog_7_cakra':
                p['name_en'] = '7 Chakras'
                p['desc_en'] = 'Holistic 7 chakra frequency series balancing Root (396 Hz), Sacral (417 Hz), Solar Plexus (528 Hz), Heart (639 Hz), Throat (741 Hz), Third Eye (852 Hz) and Crown (963 Hz) chakras respectively.'
            elif p['id'] == 'pemf_delta_sleep':
                p['name_en'] = 'Deep Delta Sleep & Regeneration'
                p['desc_en'] = 'Deep sleep, cellular regeneration and physical recovery frequency set.'
            elif p['id'] == 'pemf_atp_charge':
                p['name_en'] = 'PEMF Mitochondria ATP Charge Set'
                p['desc_en'] = 'Mitochondrial activation, cellular ATP energy recharge and vitality set.'
            else:
                p['name_en'] = p['name']
                p['desc_en'] = p.get('desc', '')
        updated_db += 1

    with open(db_path, 'w', encoding='utf-8') as f:
        json.dump(db, f, ensure_ascii=False, indent=2)

    print(f'Successfully updated {updated_db} items in {db_path}')

    # 2. Update MANEVI_PROGRAMS in 11/index.html
    html_path = os.path.join(base_dir, '11/index.html')
    with open(html_path, 'r', encoding='utf-8') as f:
        html_content = f.read()

    # Parse Esmaul Husna from rife/manevi_data_translated.js
    with open(os.path.join(base_dir, 'rife/manevi_data_translated.js'), 'r', encoding='utf-8') as f:
        raw_esma = f.read()

    m_esma = re.search(r'const esmaulHusnaData = (\[.*?\]);', raw_esma, re.DOTALL)
    arr_str = m_esma.group(1) if m_esma else ''
    arr_str = re.sub(r'([{,])\s*([a-zA-Z0-9_]+)\s*:', r'\1"\2":', arr_str)
    esma_data = json.loads(arr_str)

    # Parse Surah translations from fast_translate.py
    with open(os.path.join(base_dir, 'fast_translate.py'), 'r', encoding='utf-8') as f:
        fast_code = f.read()

    m_trans = re.search(r'translations = (\{.*?\n\})', fast_code, re.DOTALL)
    surah_trans = ast.literal_eval(m_trans.group(1)) if m_trans else {}

    esma_map = {}
    for e in esma_data:
        n_tr = e['name']
        n_en = e.get('name_i18n', {}).get('en-US', n_tr)
        d_tr = e.get('desc', '')
        d_en = e.get('desc_i18n', {}).get('en-US', d_tr)
        esma_map[n_tr] = (n_en, d_en)

    # Extract MANEVI_PROGRAMS array string from 11/index.html
    m_manevi = re.search(r'const MANEVI_PROGRAMS = (\[.*?\]);', html_content, re.DOTALL)
    if not m_manevi:
        print('MANEVI_PROGRAMS block not found in 11/index.html!')
        return

    manevi_list = json.loads(m_manevi.group(1))
    updated_manevi = 0

    for item in manevi_list:
        name = item['name']
        if name in surah_trans:
            item['name_en'] = surah_trans[name]['name_i18n']['en-US']
            item['desc_en'] = surah_trans[name]['desc_i18n']['en-US']
            updated_manevi += 1
        elif name in esma_map:
            item['name_en'], item['desc_en'] = esma_map[name]
            updated_manevi += 1
        else:
            # Check prefix matching (e.g. "El-Melik" vs "El-Melik (91 Hz)")
            matched = False
            for k, (n_en, d_en) in esma_map.items():
                if name.startswith(k) or k.startswith(name):
                    item['name_en'] = n_en
                    item['desc_en'] = d_en
                    matched = True
                    break
            if not matched:
                item['name_en'] = name
                item['desc_en'] = item.get('desc', '')
            updated_manevi += 1

    new_manevi_json = json.dumps(manevi_list, ensure_ascii=False, indent=6)
    new_manevi_block = f'const MANEVI_PROGRAMS = {new_manevi_json};'

    new_html = html_content[:m_manevi.start()] + new_manevi_block + html_content[m_manevi.end():]

    with open(html_path, 'w', encoding='utf-8') as f:
        f.write(new_html)

    print(f'Successfully updated {updated_manevi} MANEVI_PROGRAMS in 11/index.html')

if __name__ == '__main__':
    main()
