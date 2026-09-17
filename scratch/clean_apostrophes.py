import re

file_path = r'c:\Users\W11\Desktop\z-torus siteden indirildi\12\index.html'
with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Fix unescaped single quotes in Kur'an / mü'minler inside string literals
content_fixed = content.replace("mü'minler", "müminler").replace("Kur'an", "Kuran").replace("Allah'ın", "Allahın").replace("zatın'ın", "zatının")

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content_fixed)

print("Apostrophes in string values cleaned.")
