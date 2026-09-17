import os
from PIL import Image

def make_icons(src_path, dest_dir):
    try:
        img = Image.open(src_path)
        
        # Crop to square
        width, height = img.size
        min_dim = min(width, height)
        left = (width - min_dim) / 2
        top = (height - min_dim) / 2
        right = (width + min_dim) / 2
        bottom = (height + min_dim) / 2
        img_cropped = img.crop((left, top, right, bottom))
        
        # Ensure RGBA for PNG
        if img_cropped.mode != 'RGBA':
            img_cropped = img_cropped.convert('RGBA')
            
        # Create 512x512
        icon_512 = img_cropped.resize((512, 512), Image.Resampling.LANCZOS)
        icon_512.save(os.path.join(dest_dir, 'icon-512.png'), 'PNG')
        
        # Create 192x192
        icon_192 = img_cropped.resize((192, 192), Image.Resampling.LANCZOS)
        icon_192.save(os.path.join(dest_dir, 'icon-192.png'), 'PNG')
        
        # Create small webp for popup
        if img_cropped.mode == 'RGBA':
            # Convert to RGB for webp if needed or keep RGBA if webp supports it
            pass
        icon_small = img_cropped.resize((128, 128), Image.Resampling.LANCZOS)
        icon_small.save(os.path.join(dest_dir, 'icon-popup.webp'), 'WEBP', quality=80)
        
        print("Success! Created icon-512.png, icon-192.png, and icon-popup.webp")
        print(f"Sizes:")
        print(f"512: {os.path.getsize(os.path.join(dest_dir, 'icon-512.png')) // 1024} KB")
        print(f"192: {os.path.getsize(os.path.join(dest_dir, 'icon-192.png')) // 1024} KB")
        print(f"Popup: {os.path.getsize(os.path.join(dest_dir, 'icon-popup.webp')) // 1024} KB")
        
    except Exception as e:
        print(f"Error: {e}")

make_icons(
    'rife/images/menu_spiritual.webp',
    'rife/images'
)
