const fs = require('fs');
const path = require('path');

const apps = ['standart', 'rife', 'rodin', 'wav-tool'];

for (const app of apps) {
    const swPath = path.join(__dirname, '..', app, 'sw.js');
    if (fs.existsSync(swPath)) {
        let content = fs.readFileSync(swPath, 'utf8');
        content = content.replace(/"security\.js\?v=1\.1",?\s*/g, '');
        content = content.replace(/"security\.js",?\s*/g, '');
        fs.writeFileSync(swPath, content);
        console.log("Cleaned sw.js in " + app);
    }
}
