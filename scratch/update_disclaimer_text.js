const fs = require('fs');
const path = require('path');

const apps = ['standart', 'rife', 'rodin', 'wav-tool'];

const newHtml = `<h3 style='color:#FFD700;text-align:center;font-size:15px;margin-bottom:10px;'>⚠️ BİO-FREKANS / BİO-REZONANS GÜVENLİĞİ</h3><p style='margin-bottom:12px;font-size:12px;color:#ccc;'>Z-Torus Bio-Frekans Sistemi, genel iyilik halini (wellness) desteklemek amacıyla tasarlanmış güvenli bir teknolojidir. Ancak elektromanyetik alanlar ve frekans uygulamaları bazı kullanıcı grupları için risk oluşturabilir. Güvenliğiniz için, aşağıdaki durumlardan herhangi birine sahipseniz <strong>cihazı ve uygulamayı kullanmamalısınız.</strong></p><div style='background:rgba(255,50,50,0.1);padding:10px;border-radius:8px;margin-bottom:12px;border:1px solid rgba(255,50,50,0.2);'><h4 style='color:#ff5555;margin-bottom:8px;font-size:13px;'>🚫 Kesinlikle Kullanmaması Gereken Kişiler</h4><ul style='padding-left:15px;margin:0;font-size:11.5px;color:#eee;display:flex;flex-direction:column;gap:5px;'><li><strong>Elektronik İmplant Taşıyanlar:</strong> Kalp pili (pacemaker), implante edilmiş defibrilatör (ICD), insülin pompası vb.</li><li><strong>Hamileler:</strong> Gebelik dönemindeki kadınlar ve gebelik şüphesi olanlar.</li><li><strong>Epilepsi ve Nöbet Geçmişi Olanlar:</strong> Epilepsi (sara) tanısı almış kişiler.</li><li><strong>Organ Nakli Yaptırmış Olanlar:</strong> İmmünosupresif ilaç kullananlar.</li></ul></div><div style='background:rgba(255,165,0,0.1);padding:10px;border-radius:8px;margin-bottom:12px;border:1px solid rgba(255,165,0,0.2);'><h4 style='color:#ffa500;margin-bottom:8px;font-size:13px;'>⚠️ Sadece Doktor Kontrolünde Kullanması Gerekenler</h4><p style='margin-bottom:6px;font-size:11.5px;color:#ccc;'>Aşağıdaki durumlarda uygulamayı kullanmadan önce <strong>mutlaka hekiminize danışmalı</strong> ve onay almalısınız:</p><ul style='padding-left:15px;margin:0;font-size:11.5px;color:#eee;display:flex;flex-direction:column;gap:5px;'><li><strong>Kanser/Tümör:</strong> Aktif kanser tedavisi görenler.</li><li><strong>Akut Ateşli Hastalıklar:</strong> Yüksek ateşli enfeksiyonlar.</li><li><strong>Kalp Ritim Bozuklukları:</strong> Ağır aritmi hastaları.</li><li><strong>Metal Platin/Protezler:</strong> Uygulama bölgesinde büyük metal taşıyanlar.</li><li><strong>Ağır Psikiyatrik Rahatsızlıklar:</strong> Şizofreni, bipolar vb.</li></ul></div><div style='background:rgba(100,100,255,0.1);padding:10px;border-radius:8px;border:1px solid rgba(100,100,255,0.2);'><h4 style='color:#77aaff;margin-bottom:6px;font-size:13px;'>📢 Yasal Uyarı</h4><p style='margin:0;font-size:11px;color:#ccc;'>Bu uygulamada sunulan programlar ve bio-frekans cihazı, <strong>herhangi bir hastalığı teşhis etmek, tedavi etmek, iyileştirmek veya önlemek amacıyla tasarlanmamıştır.</strong> Tıbbi bir cihaz değildir. Uygulamanın kullanımı tamamen kullanıcının kendi sorumluluğundadır.</p></div>`;

for (const app of apps) {
    const vaultPath = path.join(__dirname, '..', app, 'z_vault_core.js');
    if (fs.existsSync(vaultPath)) {
        let content = fs.readFileSync(vaultPath, 'utf8');
        
        // Use regex to replace the specific tr.disclaimer_html string
        content = content.replace(/(tr:\s*{[^}]*disclaimer_html:\s*)"[^"]*"(?=\s*})/s, `$1"${newHtml.replace(/"/g, '\\"')}"`);
        
        fs.writeFileSync(vaultPath, content);
        console.log("Updated disclaimer in " + app);
    }
}
