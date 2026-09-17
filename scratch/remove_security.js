const fs = require('fs');
const path = require('path');

const apps = ['standart', 'rife', 'rodin', 'wav-tool'];

for (const app of apps) {
    const dir = path.join(__dirname, '..', app);
    
    // Process player.html
    const playerPath = path.join(dir, 'player.html');
    if (fs.existsSync(playerPath)) {
        let content = fs.readFileSync(playerPath, 'utf8');
        content = content.replace(/<script src="security\.js[^"]*"><\/script>\n?/g, '');
        fs.writeFileSync(playerPath, content);
        console.log("Cleaned player.html in " + app);
    }
    
    // Process info.html
    const infoPath = path.join(dir, 'info.html');
    if (fs.existsSync(infoPath)) {
        let content = fs.readFileSync(infoPath, 'utf8');
        content = content.replace(/<script src="security\.js[^"]*"><\/script>\n?/g, '<script src="z_vault_core.js"></script>\n');
        fs.writeFileSync(infoPath, content);
        console.log("Cleaned info.html in " + app);
    }

    // Also check index.html just in case
    const indexPath = path.join(dir, 'index.html');
    if (fs.existsSync(indexPath)) {
        let content = fs.readFileSync(indexPath, 'utf8');
        content = content.replace(/<script src="security\.js[^"]*"><\/script>\n?/g, '');
        fs.writeFileSync(indexPath, content);
        console.log("Cleaned index.html in " + app);
    }
}
