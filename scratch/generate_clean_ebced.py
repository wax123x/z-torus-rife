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

# Manual clean definitions for Surahs to prevent regex escaping issues
surahs_clean = [
    {
        "id": "manevi_surah_0",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Bismillahirahmanirarahim",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Rahman ve Rahim olan Allah'ın adıyla. ] Her hayrın başı ve başlangıç frekansı. Bedeni ve ruhu ilahi şifaya açarak negatif enerjilerden korur; bereketi davet edip işleri kolaylaştırır ve tüm kapıların rahmetle açılmasını sağlar.",
        "freqs": [{"hz": 786.0, "min": 0.317}]
    },
    {
        "id": "manevi_surah_1",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Fatiha Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Kur'an'ın Özü ve Açılışı ] Kuran-ı Kerim'in kalbi ve en büyük şifa kaynağıdır. Amacı, Allah'a hamd etmek, O'ndan hidayet ve yardım dilemek, maddi ve manevi her türlü hastalıktan arınmaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 9361.0, "min": 2.067}]
    },
    {
        "id": "manevi_surah_2",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Ayetel Kürsi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Allah'ın Yüceliği ve Kudreti ] En büyük koruma ayeti ve ilahi sığınaktır. Amacı, okuyanı her türlü nazardan, kötülükten, görünür görünmez tehlikelerden ve şeytani enerjilerden korumak, manevi bir zırh oluşturmaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 5950.0, "min": 2.833}]
    },
    {
        "id": "manevi_surah_3",
        "cat": "manevi",
        "emoji": "📖",
        "name": "İhlâs Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Allah'ın Birliği ve Eşsizliği ] Allah'ın tevhidini (birliğini) anlatan en güçlü suredir. Amacı, inancı sağlamlaştırmak, kalbi şirkten arındırmak ve manevi huzura kavuşmaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 216.0, "min": 0.783}]
    },
    {
        "id": "manevi_surah_4",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Felak Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Sabahın Rabbine Sığınma ] Yaratıkların şerrinden, karanlıklardan ve haset edenlerden Allah'a sığınılan suredir. Amacı, dışarıdan gelebilecek maddi ve manevi zararlara, büyüye ve kötü enerjilere karşı koruma sağlamaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 518.0, "min": 1.183}]
    },
    {
        "id": "manevi_surah_5",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Nâs Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ İnsanların Rabbine Sığınma ] Kalplere vesvese veren, gizlice fısıldayan cin ve insan şeytanlarının şerrinden sığınılan suredir. Amacı, içsel huzursuzlukları, şüpheleri ve psikolojik baskıları bertaraf ederek ruhsal dengeyi korumaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 728.0, "min": 1.333}]
    },
    {
        "id": "manevi_surah_6",
        "cat": "manevi",
        "emoji": "📖",
        "name": "İnşirah Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Göğsün Genişlemesi ve Ferahlık ] Zorlukla beraber kolaylığın olduğunu müjdeleyen suredir. Amacı, daralan göğsü genişletmek, sıkıntı, stres ve depresyonu gidermek, kalbe derin bir ferahlık ve umut aşılamaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 4028.0, "min": 1.717}]
    },
    {
        "id": "manevi_surah_7",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Mülk Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Mülkün Gerçek Sahibi ] Kainatın mutlak hakiminin Allah olduğunu hatırlatan suredir. Amacı, kabir azabından korumak, hayatın ve ölümün hikmetini kavratmak, manevi bir uyanış ve kalıcı huzur sağlamaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 14442.0, "min": 21.783}]
    },
    {
        "id": "manevi_surah_8",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Yâsin Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Kur'an'ın Kalbi ] Dünya ve ahiret sıkıntılarının giderilmesi için okunan en faziletli surelerden biridir. Amacı, kalpleri diriltmek, hastalıklara şifa bulmak, zor işleri kolaylaştırmak ve ilahi nura kavuşmaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 32082.0, "min": 49.483}]
    },
    {
        "id": "manevi_surah_9",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Kehf Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Fitnelerden Korunma ve İlahi Nur ] Ahir zaman fitnelerinden ve deccalin şerrinden koruyan manevi bir kalkandır. Amacı, kalbe nur indirmek, hidayette sabit kalmak ve ilahi koruma altına girmektir.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 432.0, "min": 24.0}]
    },
    {
        "id": "manevi_surah_10",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Miftah-ul Rızık (Rızık Anahtarı)",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ İlahi Bereket ve Bolluk ] Rızık kapılarının açılması, maddi ve manevi bereketin artması için dinlenmesi tavsiye edilen frekanstır. Amacı, yokluk bilincinden arınmak ve ilahi bolluk enerjisine uyumlanmaktır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 558.0, "min": 1.667}]
    },
    {
        "id": "manevi_surah_11",
        "cat": "manevi",
        "emoji": "📖",
        "name": "İsra Suresi 82. Ayet",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Kur'an'dan Şifa ve Rahmet ] Biz Kur'an'dan, mü'minler için şifa ve rahmet olacak şeyler indiriyoruz ayetinin sırrını taşıyan şifa frekansıdır.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 3022.0, "min": 0.95}]
    },
    {
        "id": "manevi_surah_12",
        "cat": "manevi",
        "emoji": "📖",
        "name": "Vakıa Suresi",
        "source": "Kuran-ı Kerim / Manevi",
        "desc": "[ Bereket ve Rızık Kapısı ] Zenginlik ve bolluğu çekmek, maddi ve manevi fakirlikten korunmak amacıyla okunan bereket suresidir.",
        "freqs": [{"hz": 786.0, "min": 0.317}, {"hz": 9842.0, "min": 28.067}]
    }
]

manevi_list.extend(surahs_clean)
print(f"Total Manevi Programs: {len(manevi_list)}")

js_code = "const MANEVI_PROGRAMS = " + json.dumps(manevi_list, ensure_ascii=False, indent=2) + ";\n"
with open(r'c:\Users\W11\Desktop\z-torus siteden indirildi\scratch\manevi_code.js', 'w', encoding='utf-8') as out:
    out.write(js_code)

print("Generated clean manevi_code.js!")
