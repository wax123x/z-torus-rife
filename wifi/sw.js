const CACHE_NAME = 'zt-rife-v8';
const ASSETS_TO_CACHE = [
  './',
  './index.html',
  './manifest.json',
  './sw.js',
  './ztorus_logo.webp',
  './icon-192.webp',
  './icon-512.webp',
  './buzzer_beep.wav',
  './buzzer_finish_chime.wav',
  './buzzer_uyanis_denge.wav',
  './data/frequency_db.json'
];

self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      console.log('[SW] Pre-caching offline app shell');
      return cache.addAll(ASSETS_TO_CACHE);
    })
  );
  self.skipWaiting();
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keys) => {
      return Promise.all(
        keys.map((key) => {
          if (key !== CACHE_NAME) {
            console.log('[SW] Deleting old cache:', key);
            return caches.delete(key);
          }
        })
      );
    })
  );
  self.clients.claim();
});

self.addEventListener('fetch', (event) => {
  if (event.request.method !== 'GET') return;
  
  const url = event.request.url;
  // WebSocket ve ESP32 API isteklerini Service Worker önbelleğine takma
  if (url.startsWith('ws:') || url.startsWith('wss:') || url.includes('/cmd') || url.includes('/status')) {
    return;
  }

  // Uygulama açılış / navigasyon istekleri (HTML): Her zaman çevrimdışı önbellekten index.html sun!
  if (event.request.mode === 'navigate' || (event.request.headers.get('accept') && event.request.headers.get('accept').includes('text/html'))) {
    event.respondWith(
      caches.match('./index.html').then((cachedHtml) => {
        if (cachedHtml) {
          return cachedHtml;
        }
        return fetch(event.request).catch(() => caches.match('./index.html'));
      })
    );
    return;
  }

  // Statik dosyalar (JSON, WebP, WAV) için önce cache, bulamazsa network
  event.respondWith(
    caches.match(event.request).then((cachedResponse) => {
      if (cachedResponse) {
        return cachedResponse;
      }
      return fetch(event.request).catch(() => {
        if (event.request.headers.get('accept') && event.request.headers.get('accept').includes('text/html')) {
          return caches.match('./index.html');
        }
      });
    })
  );
});
