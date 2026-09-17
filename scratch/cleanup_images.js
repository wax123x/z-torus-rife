const fs = require('fs');
const path = require('path');

const root = path.join(__dirname, '..');

const standartUnused = [
  "1-yeni_logo_kare.png",
  "header_banner.png",
  "hub_skin_v2.webp",
  "logo.webp",
  "menu_abundance.webp",
  "menu_animal.webp",
  "menu_app_bg.jpg",
  "menu_site_bg.webp",
  "menu_site_bg_v3.webp",
  "menu_spiritual.webp",
  "o_logo_final.webp"
];

const rifeUnused = [
  "header_banner.png",
  "hub_skin_v2.webp",
  "logo.webp",
  "menu_app_bg.jpg",
  "menu_site_bg.webp",
  "menu_site_bg_v3.webp",
  "o_logo_final.webp"
];

function cleanupApp(appDir, unusedFiles) {
    console.log(`\n--- Cleaning ${appDir} ---`);
    const imagesDir = path.join(root, appDir, 'images');
    const swPath = path.join(root, appDir, 'sw.js');
    
    // 1. Delete files
    for (const file of unusedFiles) {
        const filePath = path.join(imagesDir, file);
        if (fs.existsSync(filePath)) {
            fs.unlinkSync(filePath);
            console.log(`Deleted: ${file}`);
        }
    }
    
    // 2. Update sw.js
    if (fs.existsSync(swPath)) {
        let swContent = fs.readFileSync(swPath, 'utf8');
        
        // Remove lines that contain these unused images
        for (const file of unusedFiles) {
            // Regex to match the exact string "images/filename", with or without trailing comma
            const regex = new RegExp(`[ \t]*"images/${file.replace(/\./g, '\\.')}",?\n?`, 'g');
            swContent = swContent.replace(regex, '');
        }
        
        // Increment CACHE_NAME (e.g., zt-std-v1 -> zt-std-v2, or append timestamp)
        // Let's just append or increment the version number
        const newVersion = Date.now().toString().slice(-6); // use a unique ID for the version
        swContent = swContent.replace(/const CACHE_NAME = ['"](.*?)['"];/, (match, p1) => {
            // If it ends with a number, increment it, or just append the hash
            let base = p1.split('-v')[0];
            return `const CACHE_NAME = '${base}-v${newVersion}';`;
        });
        
        fs.writeFileSync(swPath, swContent);
        console.log(`Updated sw.js and bumped CACHE_NAME.`);
    }
}

cleanupApp('standart', standartUnused);
cleanupApp('rife', rifeUnused);

// Also let's just make sure rodin's cache is bumped so its changes (if any) are applied
const rodinSwPath = path.join(root, 'rodin', 'sw.js');
if(fs.existsSync(rodinSwPath)){
    let swContent = fs.readFileSync(rodinSwPath, 'utf8');
    const newVersion = Date.now().toString().slice(-6);
    swContent = swContent.replace(/const CACHE_NAME = ['"](.*?)['"];/, (match, p1) => {
        let base = p1.split('-v')[0];
        return `const CACHE_NAME = '${base}-v${newVersion}';`;
    });
    fs.writeFileSync(rodinSwPath, swContent);
    console.log(`\nUpdated rodin/sw.js and bumped CACHE_NAME.`);
}

console.log("\nCleanup Complete!");
