import os
from PIL import Image

src_path = os.path.join(os.path.dirname(__file__), 'ztorus_logo.webp')
if not os.path.exists(src_path):
    print("Source image not found!")
    exit(1)

img = Image.open(src_path).convert('RGBA')

# 1. iOS AppIcon
ios_appicon_dir = os.path.join(os.path.dirname(__file__), 'ios', 'App', 'App', 'Assets.xcassets', 'AppIcon.appiconset')
if os.path.exists(ios_appicon_dir):
    img_1024 = img.resize((1024, 1024), Image.Resampling.LANCZOS)
    img_1024.save(os.path.join(ios_appicon_dir, 'AppIcon-512@2x.png'))
    print("Updated iOS AppIcon-512@2x.png (1024x1024)")

# 2. iOS Splash Screen
ios_splash_dir = os.path.join(os.path.dirname(__file__), 'ios', 'App', 'App', 'Assets.xcassets', 'Splash.imageset')
if os.path.exists(ios_splash_dir):
    splash_img = Image.new('RGBA', (2732, 2732), (18, 24, 38, 255))
    logo_resized = img.resize((1024, 1024), Image.Resampling.LANCZOS)
    splash_img.paste(logo_resized, ((2732 - 1024) // 2, (2732 - 1024) // 2), logo_resized)
    
    for f in ['splash-2732x2732.png', 'splash-2732x2732-1.png', 'splash-2732x2732-2.png']:
        splash_img.save(os.path.join(ios_splash_dir, f))
    print("Updated iOS Splash screen images (2732x2732)")

# 3. Android Mipmap Launcher Icons & Splash
android_res = os.path.join(os.path.dirname(__file__), 'android', 'app', 'src', 'main', 'res')
if os.path.exists(android_res):
    densities = {
        'mipmap-mdpi': (48, 108),
        'mipmap-hdpi': (72, 162),
        'mipmap-xhdpi': (96, 216),
        'mipmap-xxhdpi': (144, 324),
        'mipmap-xxxhdpi': (192, 432)
    }
    
    for folder, (ic_size, fg_size) in densities.items():
        folder_path = os.path.join(android_res, folder)
        if not os.path.exists(folder_path):
            os.makedirs(folder_path, exist_ok=True)
            
        # ic_launcher & ic_launcher_round
        ic = img.resize((ic_size, ic_size), Image.Resampling.LANCZOS)
        ic.save(os.path.join(folder_path, 'ic_launcher.png'))
        ic.save(os.path.join(folder_path, 'ic_launcher_round.png'))
        
        # ic_launcher_foreground
        fg = img.resize((fg_size, fg_size), Image.Resampling.LANCZOS)
        fg.save(os.path.join(folder_path, 'ic_launcher_foreground.png'))
        
    # Android Splash
    splash_android = Image.new('RGBA', (2732, 2732), (18, 24, 38, 255))
    logo_resized = img.resize((1024, 1024), Image.Resampling.LANCZOS)
    splash_android.paste(logo_resized, ((2732 - 1024) // 2, (2732 - 1024) // 2), logo_resized)
    
    drawable_dir = os.path.join(android_res, 'drawable')
    if os.path.exists(drawable_dir):
        splash_android.save(os.path.join(drawable_dir, 'splash.png'))
        
    print("Updated Android Mipmap Launcher Icons & Splash screen")

print("All mobile icons and splash screens generated successfully!")
