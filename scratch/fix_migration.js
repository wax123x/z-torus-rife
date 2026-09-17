const fs = require('fs');
const path = require('path');

const dirs = ['standart', 'rife', 'rodin', 'frekans', 'titresim'];

dirs.forEach(dir => {
    const file = path.join(__dirname, '..', dir, 'z_vault_core.js');
    if (fs.existsSync(file)) {
        let content = fs.readFileSync(file, 'utf8');
        
        // 1. Remove any existing getDeviceID(); top-level call just in case
        content = content.replace(/\ngetDeviceID\(\);\s*\/\/\s*Force migration on load/g, '');
        
        // 2. Change startsWith("ZT-") to length > 4 to catch ALL old IDs (like RIFE-, ZT-, etc)
        content = content.replace(/if\s*\(\s*oldId\s*&&\s*oldId\.startsWith\("ZT-"\)\s*\)/g, 'if (oldId && oldId.length > 4)');
        
        // 3. Inject getDeviceID(); right after getDeviceID function block
        const match = content.match(/function getDeviceID\(\) \{[\s\S]*?\n\}/);
        if (match) {
            const funcBlock = match[0];
            const replacement = funcBlock + "\n\ngetDeviceID(); // Force migration on load";
            content = content.replace(funcBlock, replacement);
        }
        
        fs.writeFileSync(file, content, 'utf8');
        console.log(`Patched ${file}`);
    }
});
