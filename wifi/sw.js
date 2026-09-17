// Z-TORUS RİFE - Tam Çevrimdışı Service Worker v11
// İnternet OLMADAN da çalışır (ESP32 WiFi, BLE, frekans listesi)
const CACHE_NAME = 'zt-rife-v11';

const ASSETS_TO_CACHE = [
  './',
  './index.html',
  './manifest.json',
  './sw.js',
  './ztorus_logo.webp',
  './icon-192.webp',
  './icon-512.webp',
  './data/frequency_db.json'
];

// Kurulum: Tüm uygulama dosyalarını önbelleğe al
self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      console.log('[SW] Uygulama dosyaları önbelleğe alınıyor...');
      // Her dosyayı tek tek ekle (biri başarısız olursa diğerleri devam etsin)
      return Promise.allSettled(
        ASSETS_TO_CACHE.map(url =>
          cache.add(url).catch(err => console.warn('[SW] Önbelleğe alınamadı:', url, err))
        )
      );
    })
  );
  self.skipWaiting();
});

// Aktivasyon: Eski önbellekleri temizle
self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keys) => {
      return Promise.all(
        keys.map((key) => {
          if (key !== CACHE_NAME) {
            console.log('[SW] Eski önbellek siliniyor:', key);
            return caches.delete(key);
          }
        })
      );
    })
  );
  self.clients.claim();
});

// Fetch: Önce önbellekten sun, yoksa ağdan al, o da yoksa boş döndür
self.addEventListener('fetch', (event) => {
  if (event.request.method !== 'GET') return;

  const url = event.request.url;
  if (!url.startsWith('http://') && !url.startsWith('https://')) return;

  // WebSocket, ESP32 API ve yerel ağ isteklerini ASLA önbelleğe alma
  if (
    url.startsWith('ws:') ||
    url.startsWith('wss:') ||
    url.includes('192.168.') ||
    url.includes('/cmd') ||
    url.includes('/status') ||
    url.includes('localhost')
  ) {
    return; // Service Worker'ı atlat, doğrudan ağa git
  }

  // Dış CDN isteklerini (Google Fonts vb.) sessizce reddet — uygulama sistem fontlarını kullanır
  if (
    url.includes('fonts.googleapis.com') ||
    url.includes('fonts.gstatic.com') ||
    url.includes('cdn.')
  ) {
    event.respondWith(
      new Response('', { status: 200, headers: { 'Content-Type': 'text/css' } })
    );
    return;
  }

  // HTML navigasyon istekleri: Daima önbellekteki index.html'i sun
  if (
    event.request.mode === 'navigate' ||
    (event.request.headers.get('accept') && event.request.headers.get('accept').includes('text/html'))
  ) {
    event.respondWith(
      caches.match('./index.html').then((cached) => {
        return cached || fetch(event.request).catch(() => caches.match('./index.html'));
      })
    );
    return;
  }

  // Diğer tüm kaynaklar: Önce önbellek, yoksa ağ, o da yoksa sessizce geç
  event.respondWith(
    caches.match(event.request).then((cached) => {
      if (cached) return cached;
      return fetch(event.request).then((response) => {
        // Başarılı yanıtları önbelleğe ekle
        if (response && response.status === 200 && response.type === 'basic') {
          const responseClone = response.clone();
          caches.open(CACHE_NAME).then((cache) => cache.put(event.request, responseClone));
        }
        return response;
      }).catch(() => {
        // Ağ da yoksa boş yanıt döndür (uygulama çökmez)
        return new Response('', { status: 200 });
      });
    })
  );
});
