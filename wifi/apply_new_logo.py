import os
from PIL import Image

generated_logo_path = r"C:\Users\W11\.gemini\antigravity-ide\brain\f1fe7634-66f9-43c9-8411-198a76e2edcb\bio_resonance_pemf_torus_logo_1789629761554.png"
wifi_dir = os.path.join(os.path.dirname(__file__))

if os.path.exists(generated_logo_path):
    img = Image.open(generated_logo_path).convert('RGBA')
    
    # 1. ztorus_logo.webp (1024x1024)
    img_1024 = img.resize((1024, 1024), Image.Resampling.LANCZOS)
    img_1024.save(os.path.join(wifi_dir, 'ztorus_logo.webp'), 'WEBP', quality=95)
    img_1024.save(os.path.join(wifi_dir, 'ztorus_logo.png'), 'PNG')
    
    # 2. icon-512.webp & png
    img_512 = img.resize((512, 512), Image.Resampling.LANCZOS)
    img_512.save(os.path.join(wifi_dir, 'icon-512.webp'), 'WEBP', quality=95)
    img_512.save(os.path.join(wifi_dir, 'icon-512.png'), 'PNG')
    
    # 3. icon-192.webp & png
    img_192 = img.resize((192, 192), Image.Resampling.LANCZOS)
    img_192.save(os.path.join(wifi_dir, 'icon-192.webp'), 'WEBP', quality=95)
    img_192.save(os.path.join(wifi_dir, 'icon-192.png'), 'PNG')
    
    print("New Bio-Resonance & PEMF logo applied to wifi assets!")
else:
    print("Generated logo file not found!")
