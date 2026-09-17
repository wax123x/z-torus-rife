import re
text = open('rife/translations.js', encoding='utf-8').read()
langs = re.findall(r'"([a-z]{2}-[A-Z]{2})":\s*\{', text)
print(langs)
