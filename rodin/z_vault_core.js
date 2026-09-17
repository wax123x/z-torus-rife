/**
 * Z-Vault Core v2.5.0 - BULLETPROOF DISCLAIMER EDITION
 */

// 1. GLOBAL CONSTANTS
const MASTER_SALT = "Z-TORUS-VULT-SAFE-@2024!";
console.log("🛡️ Z-Vault Core v2.5.0 Active for Z-TORUS RODIN PEMF");


// --- DISCLAIMER LOGIC START ---
const i18nVault = {
    tr: {
        disclaimer_agree: "Okudum, anladım ve tüm sorumluluğu kabul ediyorum.",
        disclaimer_btn: "ONAYLA VE DEVAM ET",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;font-size:15px;margin-bottom:10px;'>⚠️ BİO-FREKANS / BİO-REZONANS GÜVENLİĞİ</h3><p style='margin-bottom:12px;font-size:12px;color:#ccc;'>Z-Torus Bio-Frekans Sistemi, genel iyilik halini (wellness) desteklemek amacıyla tasarlanmış güvenli bir teknolojidir. Ancak elektromanyetik alanlar ve frekans uygulamaları bazı kullanıcı grupları için risk oluşturabilir. Güvenliğiniz için, aşağıdaki durumlardan herhangi birine sahipseniz <strong>cihazı ve uygulamayı kullanmamalısınız.</strong></p><div style='background:rgba(255,50,50,0.1);padding:10px;border-radius:8px;margin-bottom:12px;border:1px solid rgba(255,50,50,0.2);'><h4 style='color:#ff5555;margin-bottom:8px;font-size:13px;'>🚫 Kesinlikle Kullanmaması Gereken Kişiler</h4><ul style='padding-left:15px;margin:0;font-size:11.5px;color:#eee;display:flex;flex-direction:column;gap:5px;'><li><strong>Elektronik İmplant Taşıyanlar:</strong> Kalp pili (pacemaker), implante edilmiş defibrilatör (ICD), insülin pompası vb.</li><li><strong>Hamileler:</strong> Gebelik dönemindeki kadınlar ve gebelik şüphesi olanlar.</li><li><strong>Epilepsi ve Nöbet Geçmişi Olanlar:</strong> Epilepsi (sara) tanısı almış kişiler.</li><li><strong>Organ Nakli Yaptırmış Olanlar:</strong> İmmünosupresif ilaç kullananlar.</li></ul></div><div style='background:rgba(255,165,0,0.1);padding:10px;border-radius:8px;margin-bottom:12px;border:1px solid rgba(255,165,0,0.2);'><h4 style='color:#ffa500;margin-bottom:8px;font-size:13px;'>⚠️ Sadece Doktor Kontrolünde Kullanması Gerekenler</h4><p style='margin-bottom:6px;font-size:11.5px;color:#ccc;'>Aşağıdaki durumlarda uygulamayı kullanmadan önce <strong>mutlaka hekiminize danışmalı</strong> ve onay almalısınız:</p><ul style='padding-left:15px;margin:0;font-size:11.5px;color:#eee;display:flex;flex-direction:column;gap:5px;'><li><strong>Kanser/Tümör:</strong> Aktif kanser tedavisi görenler.</li><li><strong>Akut Ateşli Hastalıklar:</strong> Yüksek ateşli enfeksiyonlar.</li><li><strong>Kalp Ritim Bozuklukları:</strong> Ağır aritmi hastaları.</li><li><strong>Metal Platin/Protezler:</strong> Uygulama bölgesinde büyük metal taşıyanlar.</li><li><strong>Ağır Psikiyatrik Rahatsızlıklar:</strong> Şizofreni, bipolar vb.</li></ul></div><div style='background:rgba(100,100,255,0.1);padding:10px;border-radius:8px;border:1px solid rgba(100,100,255,0.2);'><h4 style='color:#77aaff;margin-bottom:6px;font-size:13px;'>📢 Yasal Uyarı</h4><p style='margin:0;font-size:11px;color:#ccc;'>Bu uygulamada sunulan programlar ve bio-frekans cihazı, <strong>herhangi bir hastalığı teşhis etmek, tedavi etmek, iyileştirmek veya önlemek amacıyla tasarlanmamıştır.</strong> Tıbbi bir cihaz değildir. Uygulamanın kullanımı tamamen kullanıcının kendi sorumluluğundadır.</p></div>"
    },
    en: {
        disclaimer_agree: "I have read and accept all responsibility.",
        disclaimer_btn: "CONFIRM & CONTINUE",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ SAFETY WARNING</h3><p>Z-Torus System is safe. However, <strong>do not use the device</strong> if you have:</p><ul><li>Pacemakers or electronic implants.</li><li>Pregnancy.</li><li>History of seizures.</li><li>Recent organ transplants.</li></ul><p><b>Disclaimer:</b> This is not a medical device. It does not diagnose or treat. All responsibility belongs to the user.</p>"
    },
    de: {
        disclaimer_agree: "Ich habe dies gelesen und akzeptiere die gesamte Verantwortung.",
        disclaimer_btn: "BESTÄTIGEN & WEITER",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ SICHERHEITSHINWEIS</h3><p>Das Z-Torus-System ist eine sichere Technologie. Sie sollten das Gerät jedoch in folgenden Fällen <strong>nicht verwenden:</strong></p><ul><li>Personen mit Herzschrittmachern oder elektronischen medizinischen Implantaten.</li><li>Schwangere oder bei Verdacht auf Schwangerschaft.</li><li>Personen mit Epilepsie in der Vorgeschichte.</li><li>Personen mit einer kürzlichen Organtransplantation.</li></ul><p><b>Haftungsausschluss:</b> Diese Anwendung ist kein medizinisches Gerät. Sie dient nicht der Diagnose oder Behandlung. Die gesamte Verantwortung liegt beim Benutzer.</p>"
    },
    fr: {
        disclaimer_agree: "J'ai lu et j'accepte toute responsabilité.",
        disclaimer_btn: "CONFIRMER & CONTINUER",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ AVERTISSEMENT DE SÉCURITÉ</h3><p>Le système Z-Torus est une technologie sûre. Cependant, vous <strong>ne devez pas utiliser l'appareil</strong> dans les cas suivants :</p><ul><li>Personnes porteuses d'un stimulateur cardiaque ou d'un implant médical électronique.</li><li>Femmes enceintes ou en cas de suspicion de grossesse.</li><li>Personnes ayant des antécédents d'épilepsie.</li><li>Personnes ayant subi une transplantation d'organe récente.</li></ul><p><b>Clause de non-responsabilité :</b> Cette application n'est pas un dispositif médical. Elle n'est pas destinée au diagnostic ou au traitement. L'utilisateur assume l'entière responsabilité.</p>"
    },
    es: {
        disclaimer_agree: "He leído y acepto toda la responsabilidad.",
        disclaimer_btn: "CONFIRMAR Y CONTINUAR",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ ADVERTENCIA DE SEGURIDAD</h3><p>El Sistema Z-Torus es una tecnología segura. Sin embargo, <strong>no debe utilizar el dispositivo</strong> en los siguientes casos:</p><ul><li>Personas con marcapasos o implantes médicos electrónicos.</li><li>Mujeres embarazadas o con sospecha de embarazo.</li><li>Personas con antecedentes de epilepsia.</li><li>Personas con trasplante de órgano reciente.</li></ul><p><b>Aviso Legal:</b> Esta aplicación no es un dispositivo médico. No está destinada al diagnóstico o tratamiento. Toda la responsabilidad recae en el usuario.</p>"
    },
    it: {
        disclaimer_agree: "Ho letto e accetto ogni responsabilità.",
        disclaimer_btn: "CONFERMA & CONTINUA",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ AVVERTENZA DI SICUREZZA</h3><p>Il sistema Z-Torus è una tecnologia sicura. Tuttavia, <strong>non si deve utilizzare il dispositivo</strong> nei seguenti casi:</p><ul><li>Persone con pacemaker o impianti medici elettronici.</li><li>Donne in gravidanza o in caso di sospetta gravidanza.</li><li>Persone con storia di epilessia.</li><li>Persone con recente trapianto di organi.</li></ul><p><b>Dichiarazione di non responsabilità:</b> Questa applicazione non è un dispositivo medico. Non è destinata alla diagnosi o al trattamento. Tutta la responsabilità appartiene all'utente.</p>"
    },
    pt: {
        disclaimer_agree: "Li e aceito toda a responsabilidade.",
        disclaimer_btn: "CONFIRMAR & CONTINUAR",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ AVISO DE SEGURANÇA</h3><p>O Sistema Z-Torus é uma tecnologia segura. No entanto, você <strong>não deve usar o dispositivo</strong> nas seguintes situações:</p><ul><li>Pessoas com marca-passo ou implantes médicos eletrônicos.</li><li>Grávidas ou suspeita de gravidez.</li><li>Pessoas com histórico de epilepsia.</li><li>Pessoas com transplante de órgão recente.</li></ul><p><b>Aviso Legal:</b> Este aplicativo não é um dispositivo médico. Não se destina ao diagnóstico ou tratamento. Toda a responsabilidade é do usuário.</p>"
    },
    is: {
        disclaimer_agree: "Ég hef lesið og samþykki alla ábyrgð.",
        disclaimer_btn: "STAÐFESTA & HALDA ÁFRAM",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ ÖRYGGISVIÐVÖRUN</h3><p>Z-Torus kerfið er örugg tækni. Hins vegar ættir þú <strong>ekki að nota tækið</strong> í eftirfarandi tilfellum:</p><ul><li>Fólk með gangráð eða rafræn lækningatæki.</li><li>Barnshafandi konur eða ef grunur leikur á þungun.</li><li>Fólk með sögu um flogaveiki.</li><li>Fólk sem hefur nýlega gengist unter líffæraígræðslu.</li></ul><p><b>Fyrirvari:</b> Þetta forrit er ekki lækningatæki. Það er ekki ætlað til greiningar eða meðferðar. Öll ábyrgð er notandans.</p>"
    },
    az: {
        disclaimer_agree: "Oxudum, başa düşdüm və bütün məsuliyyəti qəbul edirəm.",
        disclaimer_btn: "TƏSDİQLƏ VƏ DAVAM ET",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ TƏHLÜKƏSİZLİK XƏBƏRDARLIĞI</h3><p>Z-Torus Sistemi təhlükəsiz texnologiyadır. Lakin aşağıdakı hallarda <strong>cihazdan istifadə etməməlisiniz:</strong></p><ul><li>Kardiostimulyator və ya elektron tibbi implantı olanlar.</li><li>Hamilələr və ya hamiləlik şübhəsi olanlar.</li><li>Epilepsiya (sara) keçmişi olanlar.</li><li>Yeni orqan köçürülməsi etdirənlər.</li></ul><p><b>Məsuliyyətdən imtina:</b> Bu proqram tibbi cihaz deyil. Diaqnoz və ya müalicə məqsədi daşımır. Bütün məsuliyyət istifadəçiyə məxsusdur.</p>"
    },
    ru: {
        disclaimer_agree: "Я прочитал(а) и принимаю всю ответственность.",
        disclaimer_btn: "ПОДТВЕРДИТЬ И ПРОДОЛЖИТЬ",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ ПРЕДУПРЕЖДЕНИЕ О БЕЗОПАСНОСТИ</h3><p>Система Z-Torus является безопасной технологией. Однако вам <strong>не следует использовать устройство</strong> в следующих случаях:</p><ul><li>Люди с кардиостимуляторами или электронными медицинскими имплантатами.</li><li>Беременные женщины или при подозрении на беременность.</li><li>Люди с эпилепсией в анамнезе.</li><li>Люди после недавней трансплантации органов.</li></ul><p><b>Отказ от ответственности:</b> Это приложение не является медицинским устройством. Оно не предназначено для диагностики или лечения. Вся ответственность лежит на пользователе.</p>"
    },
    ar: {
        disclaimer_agree: "لقد قرأت وأتحمل كامل المسؤولية.",
        disclaimer_btn: "تأكيد ومتابعة",
        disclaimer_html: "<h3 style='color:#FFD700;text-align:center;'>⚠️ تحذير سلامة</h3><p>نظام Z-Torus هو تقنية آمنة. ومع ذلك، <strong>يجب عدم استخدام الجهاز</strong> في الحالات التالية:</p><ul><li>الأشخاص الذين لديهم أجهزة تنظيم ضربات القلب أو غرسات طبية إلكترونية.</li><li>النساء الحوامل أو عند وجود شك في الحمل.</li><li>الأشخاص الذين لديهم تاريخ من الصرع.</li><li>الأشخاص الذين خضعوا لعملية زراعة أعضاء حديثة.</li></ul><p><b>إخلاء المسؤولية:</b> هذا التطبيق ليس جهازًا طبيًا. ولا يهدف للتشخيص أو العلاج. المسؤولية الكاملة تقع على عاتق المستخدم.</p>"
    }
};

function getVaultLang() {
    let saved = localStorage.getItem('zlang');
    if (!saved) saved = navigator.language || navigator.userLanguage || 'en';
    return saved.split('-')[0].toLowerCase();
}

function continueToApp() {
    const pending = localStorage.getItem('z_vault_pending_url_rodin');
    if (pending && pending !== 'index.html') {
        localStorage.removeItem('z_vault_pending_url_rodin');
        window.location.href = pending;
    } else {
        window.location.href = 'index.html?v=' + Date.now();
    }
}

function showDisclaimerUI() {
    const t = i18nVault[getVaultLang()] || i18nVault['en'];
    let overlay = document.getElementById('zVaultOverlay') || document.createElement('div');
    overlay.id = 'zVaultOverlay';
    
    if (!document.getElementById('zVaultStyles')) {
        const style = document.createElement('style');
        style.id = 'zVaultStyles';
        style.textContent = `
            #zVaultOverlay { position: fixed; inset: 0; background: #0D0F21; z-index: 10000; display: flex; align-items: center; justify-content: center; font-family: sans-serif; color: white; padding: 20px; }
            .z-vault-card { background: rgba(255,255,255,0.05); border: 1px solid rgba(255,255,255,0.1); padding: 40px 25px; border-radius: 30px; backdrop-filter: blur(20px); width: 100%; text-align: center; }
            .z-vault-logo { font-size: 22px; font-weight: 800; margin-bottom: 25px; }
            .z-vault-btn { width: 100%; background: linear-gradient(135deg, #AD5AE2, #7B2CBF); color: white; border: none; padding: 15px; border-radius: 15px; font-weight: 700; cursor: pointer; }
            .z-vault-btn:disabled { opacity: 0.3; }
        `;
        document.head.appendChild(style);
    }
    
    if (!document.body.contains(overlay)) document.body.appendChild(overlay);

    overlay.innerHTML = `
        <div class="z-vault-card" style="max-width:460px;">
            <div class="z-vault-logo">Z-TORUS</div>
            <div style="max-height:250px;overflow-y:auto;background:rgba(0,0,0,0.2);padding:15px;border-radius:15px;font-size:13px;text-align:left;line-height:1.5;margin-bottom:20px;">
                ${t.disclaimer_html}
            </div>
            <label style="display:flex;align-items:center;gap:10px;cursor:pointer;font-size:13px;margin-bottom:20px;">
                <input type="checkbox" id="zvCheck">
                <span style="color:#fff;">${t.disclaimer_agree}</span>
            </label>
            <button id="zvConfirm" class="z-vault-btn" disabled>${t.disclaimer_btn}</button>
        </div>
    `;

    const chk = document.getElementById('zvCheck');
    const btn = document.getElementById('zvConfirm');
    chk.onchange = () => btn.disabled = !chk.checked;
    btn.onclick = () => {
        localStorage.setItem('z_vault_disclaimer_accepted_rodin', 'true');
        overlay.remove();
        continueToApp();
    };
}
// --- DISCLAIMER LOGIC END ---

// 2. STABLE DEVICE ID
function getDeviceID() {
    let id = localStorage.getItem('z_vault_device_id_rodin');
    if (!id || !id.startsWith("RODIN-")) {
        let oldId = localStorage.getItem('z_vault_device_id');
        let code = "";
        if (oldId && oldId.length > 4) {
            code = oldId.replace(/^[A-Za-z]+-/, '');
        } else if (id && id.length > 4) {
            code = id.replace(/^[A-Za-z]+-/, '');
        } else {
            code = Math.random().toString(36).substring(2, 10).toUpperCase();
        }
        id = "RODIN-" + code;
        localStorage.setItem('z_vault_device_id_rodin', id);
        let oldKey = localStorage.getItem('z_vault_active_key');
        if (oldKey) localStorage.setItem('z_vault_active_key_rodin', oldKey);
        let oldDisclaimer = localStorage.getItem('z_vault_disclaimer_accepted');
        if (oldDisclaimer) localStorage.setItem('z_vault_disclaimer_accepted_rodin', oldDisclaimer);
    }
    return id;
}

getDeviceID(); // Force migration on load

// 3. CRYPTO ENGINE
function xorCipher(text, key) {
    let result = "";
    for (let i = 0; i < text.length; i++) {
        result += String.fromCharCode(text.charCodeAt(i) ^ key.charCodeAt(i % key.length));
    }
    return result;
}

function b64ToUtf8(str) {
    let b64 = str.replace(/[\s\r\n\t]/g, '');
    while (b64.length % 4 !== 0) b64 += '=';
    try {
        const binary = atob(b64);
        const encoded = Array.from(binary).map(c => '%' + ('00' + c.charCodeAt(0).toString(16)).slice(-2)).join('');
        return decodeURIComponent(encoded);
    } catch (e) {
        return atob(b64);
    }
}

function verifyKey(inputKey) {
    if (!inputKey) return false;
    try {
        const sanitizedKey = inputKey.replace(/[\s\r\n\t]/g, '');
        let decrypted = "";

        try {
            const decodedUtf8 = b64ToUtf8(sanitizedKey);
            decrypted = xorCipher(decodedUtf8, MASTER_SALT);
        } catch (e) {
            let legacyB64 = sanitizedKey;
            while (legacyB64.length % 4 !== 0) legacyB64 += '=';
            const decodedLegacy = atob(legacyB64);
            decrypted = xorCipher(decodedLegacy, MASTER_SALT);
        }

        const parts = decrypted.split('|');
        if (parts.length < 2) return false;
        
        const keyID = parts[0].trim().toUpperCase().replace(/^[A-Z]+-/, '');
        const currentID = getDeviceID().trim().toUpperCase().replace(/^[A-Z]+-/, '');
        
        const expiry = parseInt(parts[1]);
        if (expiry !== 0 && Date.now() > expiry) return false;
        
        return keyID === currentID;
    } catch (e) { return false; }
}

// 4. CORE OBJECT
window.ZVault = {
    isActivated: function() {
        const key = localStorage.getItem('z_vault_active_key_rodin');
        if (!key) return false;
        return verifyKey(key);
    },
    isDisclaimerAccepted: function() {
        return localStorage.getItem('z_vault_disclaimer_accepted_rodin') === 'true';
    },
    init: function() {
        if (this.isActivated()) {
            if (!this.isDisclaimerAccepted()) {
                showDisclaimerUI();
            }
            return;
        }
        this.injectUI();
    },
    decryptData: function(enc) {
        if (!this.isActivated() || !this.isDisclaimerAccepted()) return null;
        try {
            const binary = atob(enc);
            const bytes = new Uint8Array(binary.length);
            for (let i = 0; i < binary.length; i++) {
                bytes[i] = binary.charCodeAt(i) ^ MASTER_SALT.charCodeAt(i % MASTER_SALT.length);
            }
            return JSON.parse(new TextDecoder().decode(bytes));
        } catch (e) { return null; }
    },
    injectUI: function() {
        if (document.getElementById('zVaultOverlay')) return;
        const deviceId = getDeviceID();
        const waLink = "https://api.whatsapp.com/send?phone=905357443436&text=Aktivasyon%20Kodum:%20" + deviceId;
        
        const html = `
            <div id="zVaultOverlay" style="position:fixed;inset:0;background:#0D0F21;z-index:10000;display:flex;align-items:center;justify-content:center;color:white;font-family:sans-serif;">
                <div style="background:rgba(255,255,255,0.05);padding:40px;border-radius:30px;border:1px solid rgba(255,255,255,0.1);max-width:350px;width:90%;text-align:center;">
                    <h2 style="color:#FFD700;margin-bottom:10px;">Z-TORUS RODIN PEMF</h2>
                    <p style="font-size:12px;opacity:0.8;margin-bottom:5px;">CİHAZ KODU (TIKLA KOPYALA)<br><span style="color:#FFD700;">Yöneticiye Gönder, Giriş Anahtarı İste</span></p>
                    <div id="zCopy" style="font-family:monospace;font-size:20px;color:#5BA4FF;margin:15px 0;cursor:pointer;">${deviceId}</div>
                    <input type="text" id="zKey" placeholder="Şifreyi Giriniz" style="width:100%;padding:15px;border-radius:12px;border:none;background:rgba(255,255,255,0.1);color:white;text-align:center;margin-bottom:15px;">
                    <button id="zBtn" style="width:100%;padding:15px;border-radius:12px;border:none;background:#AD5AE2;color:white;font-weight:bold;cursor:pointer;">SİSTEMİ AÇ</button>
                    <div id="zErr" style="color:#f44336;font-size:13px;margin-top:10px;display:none;">Hatalı şifre veya kod!</div>
                    <a href="${waLink}" target="_blank" style="display:block;margin-top:20px;color:#25D366;text-decoration:none;font-size:14px;">WhatsApp Destek</a>
                </div>
            </div>
        `;
        document.body.insertAdjacentHTML('afterbegin', html);
        
        document.getElementById('zCopy').onclick = () => {
            navigator.clipboard.writeText(deviceId);
            alert("Kopyalandı!");
        };

        document.getElementById('zBtn').onclick = async function() {
            const key = document.getElementById('zKey').value.trim();
            if (verifyKey(key)) {
                this.innerText = "AÇILIYOR...";
                localStorage.setItem('z_vault_active_key_rodin', key);
                
                // ⚠️ CRITICAL: Hard Wait to ensure Storage is saved
                setTimeout(() => {
                    if (!window.ZVault.isDisclaimerAccepted()) {
                        document.getElementById('zVaultOverlay').remove();
                        showDisclaimerUI();
                    } else {
                        continueToApp();
                    }
                }, 300);
            } else {
                document.getElementById('zErr').style.display = 'block';
            }
        };
    }
};

// 5. GLOBAL CLICK HANDLER
window.handleAppClick = function(e) {
    if (!window.ZVault.isActivated() || !window.ZVault.isDisclaimerAccepted()) {
        if (e && e.preventDefault) e.preventDefault();
        const url = e.currentTarget ? e.currentTarget.getAttribute('href') : null;
        if (url) localStorage.setItem('z_vault_pending_url_rodin', url);
        window.ZVault.init();
        return false;
    }
    return true;
};

// 6. AUTO-INIT
document.addEventListener('DOMContentLoaded', () => {
    if (document.body.hasAttribute('data-zvault-block')) {
        window.ZVault.init();
    }
});
