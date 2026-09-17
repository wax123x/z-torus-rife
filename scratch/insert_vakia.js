const fs = require('fs');
const path = require('path');

const filePath = path.join(__dirname, '..', 'rife', 'manevi_data.js');

if (fs.existsSync(filePath)) {
    let content = fs.readFileSync(filePath, 'utf8');
    
    const newEntry = `,\n    { name: "Vakıa Suresi", freq: 9842, duration: 1703, desc: "[ Bereket ve Rızık Kapısı ] Zenginlik ve bolluğu çekmek, maddi ve manevi fakirlikten korunmak amacıyla okunan bereket suresidir.", sequence: [{freq: 786, duration: 19}, {freq: 9842, duration: 1684}] }`;
    
    // Find the last occurrence of ']' which closes the maneviData array.
    // It's safer to replace the exact closing sequence of maneviData.
    // The last item in maneviData is İsra Suresi. We can just replace the final \n]; with the new entry and \n];
    content = content.replace(/\n];\s*$/, newEntry + '\n];\n');
    
    fs.writeFileSync(filePath, content);
    console.log("Vakıa Suresi added successfully.");
} else {
    console.error("File not found:", filePath);
}
