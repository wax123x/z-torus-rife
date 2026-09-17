const fs = require('fs');

const MASTER_SALT = "Z-TORUS-VULT-SAFE-@2024!";

function decryptData(enc) {
    try {
        const binary = atob(enc);
        const bytes = new Uint8Array(binary.length);
        for (let i = 0; i < binary.length; i++) {
            bytes[i] = binary.charCodeAt(i) ^ MASTER_SALT.charCodeAt(i % MASTER_SALT.length);
        }
        return JSON.parse(new TextDecoder().decode(bytes));
    } catch (e) { 
        return { error: e.message }; 
    }
}

const fileContent = fs.readFileSync('standart/data.js', 'utf8');
const match = fileContent.match(/window\.RIFE_DATA_ENCRYPTED\s*=\s*"(.*?)";/);
if (match) {
    const enc = match[1];
    console.log("Found encrypted payload, length:", enc.length);
    const result = decryptData(enc);
    if (result.error) {
        console.error("Decryption failed:", result.error);
    } else {
        console.log("Decryption SUCCESS. Programs count:", result.programs ? result.programs.length : 'no programs field');
    }
} else {
    console.log("Could not find payload.");
}
