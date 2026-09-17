const fs = require('fs');
const path = require('path');

const srcDir = __dirname;
const wwwDir = path.join(__dirname, 'www');

if (fs.existsSync(wwwDir)) {
  fs.rmSync(wwwDir, { recursive: true, force: true });
}
fs.mkdirSync(wwwDir, { recursive: true });

function copyRecursive(src, dest) {
  const stats = fs.statSync(src);
  if (stats.isDirectory()) {
    if (!fs.existsSync(dest)) fs.mkdirSync(dest, { recursive: true });
    const files = fs.readdirSync(src);
    for (const file of files) {
      if (['node_modules', 'android', 'ios', 'www', '.git'].includes(file)) continue;
      copyRecursive(path.join(src, file), path.join(dest, file));
    }
  } else {
    fs.copyFileSync(src, dest);
  }
}

// Copy top level web files and data folder
const itemsToCopy = [
  'index.html',
  'manifest.json',
  'sw.js',
  'ztorus_logo.webp',
  'icon-192.webp',
  'icon-512.webp',
  'data'
];

for (const item of itemsToCopy) {
  const itemPath = path.join(srcDir, item);
  if (fs.existsSync(itemPath)) {
    copyRecursive(itemPath, path.join(wwwDir, item));
  }
}

// Copy any wav files if present
const allFiles = fs.readdirSync(srcDir);
for (const file of allFiles) {
  if (file.endsWith('.wav')) {
    fs.copyFileSync(path.join(srcDir, file), path.join(wwwDir, file));
  }
}

console.log('Successfully built web assets into www/');
