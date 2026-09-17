const fs = require('fs');
const path = require('path');

const projectRoot = path.join(__dirname, '..');

// Function to recursively find all files with specific extensions
function findFiles(dir, extensions, fileList = []) {
    const files = fs.readdirSync(dir);
    for (const file of files) {
        const filePath = path.join(dir, file);
        const stat = fs.statSync(filePath);
        if (stat.isDirectory()) {
            if (file !== 'images' && file !== 'icons' && file !== 'scratch' && file !== '.git') {
                findFiles(filePath, extensions, fileList);
            }
        } else {
            const ext = path.extname(file).toLowerCase();
            if (extensions.includes(ext)) {
                fileList.push(filePath);
            }
        }
    }
    return fileList;
}

// Function to check unused images in a specific app's image folder
function checkUnused(appDirName) {
    const imagesDir = path.join(projectRoot, appDirName, 'images');
    if (!fs.existsSync(imagesDir)) return;
    
    console.log(`\n--- Checking ${appDirName}/images ---`);
    const images = fs.readdirSync(imagesDir).filter(f => /\.(png|jpg|jpeg|gif|webp|svg)$/i.test(f));
    
    const sourceFiles = [
        path.join(projectRoot, 'index.html'),
        ...findFiles(path.join(projectRoot, appDirName), ['.html', '.css', '.js', '.json']).filter(f => !f.endsWith('sw.js'))
    ];
    
    let contents = '';
    for (const file of sourceFiles) {
        if (fs.existsSync(file)) {
            contents += fs.readFileSync(file, 'utf8') + '\n';
        }
    }
    
    const unused = [];
    for (const img of images) {
        if (!contents.includes(img)) {
            unused.push(img);
        }
    }
    
    if (unused.length > 0) {
        console.log(`Found ${unused.length} unused images out of ${images.length}:`);
        unused.forEach(img => console.log('  - ' + img));
    } else {
        console.log(`All ${images.length} images are used!`);
    }
}

// Check Standart, Rife, Rodin
checkUnused('standart');
checkUnused('rife');
checkUnused('rodin');
