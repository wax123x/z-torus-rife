const fs = require('fs');
const path = require('path');

const maneviPath = path.join(__dirname, '..', 'rife', 'manevi_player.html');
if (fs.existsSync(maneviPath)) {
    let content = fs.readFileSync(maneviPath, 'utf8');
    
    // Remove old security.js script tags
    content = content.replace(/<script src="security\.js[^"]*"><\/script>\n?/g, '');
    
    // Ensure z_vault_core.js exists (if not, add it right after <head> or similar)
    if (!content.includes('z_vault_core.js')) {
        content = content.replace(/<head>/, '<head>\n  <script src="z_vault_core.js"></script>');
    }
    
    fs.writeFileSync(maneviPath, content);
    console.log("Cleaned and secured manevi_player.html");
} else {
    console.log("manevi_player.html not found.");
}
