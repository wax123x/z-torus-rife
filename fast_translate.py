import json

with open('rife/manevi_data.js', 'r', encoding='utf-8') as f:
    content = f.read()

translations = {
    "Bismillahirahmanirarahim": {
        "name_i18n": {"en-US": "Bismillah al-Rahman al-Rahim", "de-DE": "Bismillah al-Rahman al-Rahim"},
        "desc_i18n": {"en-US": "[ In the name of Allah, the Most Gracious, the Most Merciful. ] The beginning of every good deed and the starting frequency. Opens the body and soul to divine healing, protects from negative energies, invites abundance and facilitates matters.", "de-DE": "[ Im Namen Allahs, des Allerbarmers, des Barmherzigen. ] Der Beginn jeder guten Tat und die Startfrequenz. Öffnet Körper und Seele für göttliche Heilung."}
    },
    "Esmaül Hüsna (99 İsim)": {
        "name_i18n": {"en-US": "Esma-ul Husna (99 Names)", "de-DE": "Esma-ul Husna (99 Namen)"},
        "desc_i18n": {"en-US": "You can listen to or read the 99 beautiful names of Allah in order.", "de-DE": "Sie können die 99 schönen Namen Allahs nacheinander anhören oder lesen."}
    },
    "Fatiha Suresi": {
        "name_i18n": {"en-US": "Surah Al-Fatihah", "de-DE": "Sura Al-Fatiha"},
        "desc_i18n": {"en-US": "[ The Essence and Opening of the Quran ] The heart of the Quran and the greatest source of healing. Its purpose is to praise Allah, seek His guidance and help, and to purify from all material and spiritual diseases.", "de-DE": "[ Die Essenz und Eröffnung des Korans ] Das Herz des Korans und die größte Quelle der Heilung. Ihr Zweck ist es, Allah zu preisen, Seine Führung zu suchen und von allen Krankheiten zu reinigen."}
    },
    "Ayetel Kürsi": {
        "name_i18n": {"en-US": "Ayatul Kursi", "de-DE": "Ayatul Kursi"},
        "desc_i18n": {"en-US": "[ The Majesty and Power of Allah ] The greatest verse of protection and divine refuge. Its purpose is to protect the reciter from all evil eyes, visible and invisible dangers, and demonic energies, creating a spiritual armor.", "de-DE": "[ Die Majestät und Macht Allahs ] Der größte Vers des Schutzes und der göttlichen Zuflucht. Sein Zweck ist es, den Leser vor allen bösen Blicken und Gefahren zu schützen."}
    },
    "İhlâs Suresi": {
        "name_i18n": {"en-US": "Surah Al-Ikhlas", "de-DE": "Sura Al-Ikhlas"},
        "desc_i18n": {"en-US": "[ The Oneness and Uniqueness of Allah ] The most powerful surah explaining the oneness of Allah (Tawheed). Its purpose is to strengthen faith, purify the heart from polytheism, and achieve spiritual peace.", "de-DE": "[ Die Einheit und Einzigartigkeit Allahs ] Die mächtigste Sure, die die Einheit Allahs erklärt. Ihr Zweck ist es, den Glauben zu stärken und geistigen Frieden zu erreichen."}
    },
    "Felak Suresi": {
        "name_i18n": {"en-US": "Surah Al-Falaq", "de-DE": "Sura Al-Falaq"},
        "desc_i18n": {"en-US": "[ Seeking Refuge in the Lord of the Dawn ] The surah of seeking refuge in Allah from the evil of creatures, darkness, and the envious. Its purpose is to provide protection against material and spiritual harms, magic, and bad energies.", "de-DE": "[ Zuflucht suchen beim Herrn der Morgenröte ] Die Sure der Zuflucht bei Allah vor dem Bösen der Geschöpfe. Ihr Zweck ist es, Schutz vor materiellen und geistigen Schäden zu bieten."}
    },
    "Nâs Suresi": {
        "name_i18n": {"en-US": "Surah An-Nas", "de-DE": "Sura An-Nas"},
        "desc_i18n": {"en-US": "[ Seeking Refuge in the Lord of Mankind ] The surah of seeking refuge from the evil of jinn and human devils who whisper into hearts. Its purpose is to maintain mental balance by eliminating inner restlessness and psychological pressures.", "de-DE": "[ Zuflucht suchen beim Herrn der Menschen ] Die Sure der Zuflucht vor dem Bösen der Dschinn und Menschenteufel. Ihr Zweck ist es, das geistige Gleichgewicht zu erhalten."}
    },
    "İnşirah Suresi": {
        "name_i18n": {"en-US": "Surah Al-Inshirah", "de-DE": "Sura Al-Inshirah"},
        "desc_i18n": {"en-US": "[ The Expansion of the Breast and Relief ] The surah that gives the glad tidings that with hardship comes ease. Its purpose is to expand the narrowed chest, relieve distress, stress, and depression, and instill deep relief.", "de-DE": "[ Die Erweiterung der Brust und Erleichterung ] Die Sure, die verkündet, dass mit der Erschwernis die Erleichterung kommt. Ihr Zweck ist es, Stress und Depressionen zu lindern."}
    },
    "Mülk Suresi": {
        "name_i18n": {"en-US": "Surah Al-Mulk", "de-DE": "Sura Al-Mulk"},
        "desc_i18n": {"en-US": "[ The True Owner of Sovereignty ] The surah that reminds us that Allah is the absolute ruler of the universe. Its purpose is to protect from the punishment of the grave, to make one understand the wisdom of life and death.", "de-DE": "[ Der wahre Besitzer der Souveränität ] Die Sure, die uns daran erinnert, dass Allah der absolute Herrscher des Universums ist. Sie schützt vor der Strafe des Grabes."}
    },
    "Yâsin Suresi": {
        "name_i18n": {"en-US": "Surah Ya-Sin", "de-DE": "Sura Ya-Sin"},
        "desc_i18n": {"en-US": "[ The Heart of the Quran ] One of the most virtuous surahs read to eliminate the troubles of this world and the hereafter. Its purpose is to revive hearts, find healing for diseases, facilitate difficult matters, and attain divine light.", "de-DE": "[ Das Herz des Korans ] Eine der tugendhaftesten Suren, um die Probleme dieser Welt zu beseitigen. Ihr Zweck ist es, Heilung für Krankheiten zu finden und schwierige Angelegenheiten zu erleichtern."}
    },
    "Kehf Suresi": {
        "name_i18n": {"en-US": "Surah Al-Kahf", "de-DE": "Sura Al-Kahf"},
        "desc_i18n": {"en-US": "[ Protection from Trials and Divine Light ] A spiritual shield that protects from the trials of the end times and the evil of the antichrist. Its purpose is to bring light to the heart, remain steadfast in guidance, and enter under divine protection.", "de-DE": "[ Schutz vor Prüfungen und göttliches Licht ] Ein spiritueller Schild, der vor den Prüfungen der Endzeit schützt. Sein Zweck ist es, Licht ins Herz zu bringen und göttlichen Schutz zu erlangen."}
    },
    "Miftah-ul Rızık (Rızık Anahtarı)": {
        "name_i18n": {"en-US": "Miftah-ul Rizq (Key to Sustenance)", "de-DE": "Miftah-ul Rizq (Schlüssel zum Lebensunterhalt)"},
        "desc_i18n": {"en-US": "[ Divine Blessing and Abundance ] The recommended frequency to listen to for the opening of the doors of sustenance and the increase of material and spiritual abundance. Its purpose is to purify from poverty consciousness.", "de-DE": "[ Göttlicher Segen und Überfluss ] Die empfohlene Frequenz für das Öffnen der Türen des Lebensunterhalts und die Zunahme von materiellem und spirituellem Überfluss."}
    },
    "İsra Suresi 82. Ayet": {
        "name_i18n": {"en-US": "Surah Al-Isra, Verse 82", "de-DE": "Sura Al-Isra, Vers 82"},
        "desc_i18n": {"en-US": "[ Healing and Mercy from the Quran ] The healing frequency bearing the secret of the verse 'And We send down of the Quran that which is healing and mercy for the believers.' It is listened to find healing for physical and spiritual illnesses.", "de-DE": "[ Heilung und Barmherzigkeit aus dem Koran ] Die Heilfrequenz, die das Geheimnis des Verses trägt 'Und Wir senden vom Koran das hinab, was eine Heilung und Barmherzigkeit für die Gläubigen ist'."}
    },
    "Vakıa Suresi": {
        "name_i18n": {"en-US": "Surah Al-Waqi'a", "de-DE": "Sura Al-Waqi'a"},
        "desc_i18n": {"en-US": "[ The Door of Abundance and Sustenance ] The surah of abundance read to attract wealth and prosperity and to protect from material and spiritual poverty.", "de-DE": "[ Die Tür des Überflusses und Lebensunterhalts ] Die Sure des Überflusses, gelesen um Reichtum und Wohlstand anzuziehen und vor materieller und spiritueller Armut zu schützen."}
    }
}

new_content = content
for key, data in translations.items():
    name_str = f'name: "{key}"'
    i18n_str = f', name_i18n: {json.dumps(data["name_i18n"], ensure_ascii=False)}, desc_i18n: {json.dumps(data["desc_i18n"], ensure_ascii=False)}'
    new_content = new_content.replace(name_str, name_str + i18n_str)

with open('rife/manevi_data.js', 'w', encoding='utf-8') as f:
    f.write(new_content)

print("Injected fast translations successfully!")
