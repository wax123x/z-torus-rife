import re
import json

with open('rife/manevi_data.js', 'r', encoding='utf-8') as f:
    content = f.read()

translations = {
    "Bismillahirahmanirarahim": {
        "name_i18n": "بسم الله الرحمن الرحيم",
        "desc_i18n": "[ بسم الله الرحمن الرحيم. ] بداية كل عمل صالح وتردد البداية. يفتح الجسد والروح للشفاء الإلهي، ويحمي من الطاقات السلبية، ويدعو إلى الوفرة وييسر الأمور."
    },
    "Esmaül Hüsna (99 İsim)": {
        "name_i18n": "أسماء الله الحسنى (99 اسم)",
        "desc_i18n": "يمكنك الاستماع إلى أسماء الله الحسنى الـ 99 أو قراءتها بالترتيب."
    },
    "Fatiha Suresi": {
        "name_i18n": "سورة الفاتحة",
        "desc_i18n": "[ جوهر القرآن وفاتحته ] قلب القرآن الكريم وأعظم مصدر للشفاء. الغرض منها هو حمد الله، وطلب هدايته وعونه، والتطهر من كل الأمراض المادية والروحية."
    },
    "Ayetel Kürsi": {
        "name_i18n": "آية الكرسي",
        "desc_i18n": "[ عظمة وقدرة الله ] أعظم آية للحماية واللجوء الإلهي. الغرض منها حماية القارئ من كل عين حاسدة، ومخاطر ظاهرة وخفية، وطاقات شيطانية، وخلق درع روحي."
    },
    "İhlâs Suresi": {
        "name_i18n": "سورة الإخلاص",
        "desc_i18n": "[ وحدانية وتفرد الله ] أقوى سورة تشرح وحدانية الله (التوحيد). الغرض منها تقوية الإيمان، وتطهير القلب من الشرك، وتحقيق السلام الروحي."
    },
    "Felak Suresi": {
        "name_i18n": "سورة الفلق",
        "desc_i18n": "[ الاستعاذة برب الفلق ] سورة الاستعاذة بالله من شر المخلوقات، والظلام، والحاسدين. الغرض منها توفير الحماية ضد الأضرار المادية والروحية، والسحر، والطاقات السيئة."
    },
    "Nâs Suresi": {
        "name_i18n": "سورة الناس",
        "desc_i18n": "[ الاستعاذة برب الناس ] سورة الاستعاذة من شر الجن وشياطين الإنس الذين يوسوسون في الصدور. الغرض منها الحفاظ على التوازن العقلي من خلال القضاء على القلق الداخلي والضغوط النفسية."
    },
    "İnşirah Suresi": {
        "name_i18n": "سورة الشرح",
        "desc_i18n": "[ انشراح الصدر والراحة ] السورة التي تبشر بأن مع العسر يسراً. الغرض منها توسيع الصدر الضيق، وتخفيف الضيق والتوتر والاكتئاب، وغرس الراحة العميقة."
    },
    "Mülk Suresi": {
        "name_i18n": "سورة الملك",
        "desc_i18n": "[ المالك الحقيقي للملك ] السورة التي تذكرنا بأن الله هو الحاكم المطلق للكون. الغرض منها الحماية من عذاب القبر، وجعل المرء يدرك حكمة الحياة والموت."
    },
    "Yâsin Suresi": {
        "name_i18n": "سورة يس",
        "desc_i18n": "[ قلب القرآن ] من أكثر السور فضلاً تقرأ لإزالة هموم الدنيا والآخرة. الغرض منها إحياء القلوب، وإيجاد الشفاء للأمراض، وتسهيل الأمور الصعبة، وبلوغ النور الإلهي."
    },
    "Kehf Suresi": {
        "name_i18n": "سورة الكهف",
        "desc_i18n": "[ الحماية من الفتن والنور الإلهي ] درع روحي يحمي من فتن آخر الزمان وشر المسيح الدجال. الغرض منها جلب النور للقلب، والثبات على الهدى، والدخول في الحماية الإلهية."
    },
    "Miftah-ul Rızık (Rızık Anahtarı)": {
        "name_i18n": "مفتاح الرزق",
        "desc_i18n": "[ البركة الإلهية والوفرة ] التردد الموصى بالاستماع إليه لفتح أبواب الرزق وزيادة الوفرة المادية والروحية. الغرض منه هو التطهير من وعي الفقر."
    },
    "İsra Suresi 82. Ayet": {
        "name_i18n": "سورة الإسراء، الآية 82",
        "desc_i18n": "[ الشفاء والرحمة من القرآن ] تردد الشفاء الذي يحمل سر الآية 'وننزل من القرآن ما هو شفاء ورحمة للمؤمنين'. يُستمع إليه لإيجاد الشفاء للأمراض الجسدية والروحية."
    },
    "Vakıa Suresi": {
        "name_i18n": "سورة الواقعة",
        "desc_i18n": "[ باب الوفرة والرزق ] سورة الوفرة تُقرأ لجذب الثروة والازدهار والحماية من الفقر المادي والروحي."
    }
}

new_content = content
for key, data in translations.items():
    # Regex to find: name_i18n: { ... }, desc_i18n: { ... }
    # Because there might be escape sequences, we use re.sub
    # We look for the block immediately following name: "key"
    
    # We will search for: name: "key", name_i18n: {.*?}, desc_i18n: {.*?}
    pattern = r'(name: "' + re.escape(key) + r'", name_i18n: \{.*?)(?=\}), desc_i18n: \{.*?(?=\})'
    
    ar_name_json = json.dumps(data['name_i18n'], ensure_ascii=False)
    ar_desc_json = json.dumps(data['desc_i18n'], ensure_ascii=False)
    
    # We can just replace '}' with ', "ar-SA": "ar_name_json"}' in both dictionaries.
    def replacer(match):
        # match.group(0) is: name: "key", name_i18n: {"en-US": "...", "de-DE": "..."
        # Wait, the regex is a bit complex.
        pass
    
    # Simpler: just find the two JSON dicts for this key.
    # The string we inserted earlier looks exactly like:
    # name: "Bismillahirahmanirarahim", name_i18n: {"en-US": "...", "de-DE": "..."}, desc_i18n: {"en-US": "...", "de-DE": "..."}
    
    # Find the chunk starting from name: "key",
    start_idx = new_content.find(f'name: "{key}"')
    if start_idx != -1:
        # find the end of desc_i18n object
        # Since we just inserted it, it is on the same line or right after.
        # Find the first 'desc_i18n: {'
        desc_start = new_content.find('desc_i18n: {', start_idx)
        desc_end = new_content.find('}', desc_start)
        
        # We also need to find name_i18n
        name_start = new_content.find('name_i18n: {', start_idx)
        name_end = new_content.find('}', name_start)
        
        if desc_start != -1 and name_end != -1:
            # We must replace backwards to not mess up indices
            new_content = new_content[:desc_end] + f', "ar-SA": {ar_desc_json}' + new_content[desc_end:]
            new_content = new_content[:name_end] + f', "ar-SA": {ar_name_json}' + new_content[name_end:]

with open('rife/manevi_data.js', 'w', encoding='utf-8') as f:
    f.write(new_content)

print("Arabic translations injected successfully!")
