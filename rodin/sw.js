const CACHE_NAME = 'zt-rodin-v2.5.1';
const ASSETS = [
  "./",
  "index.html",
  "z_vault_core.js",
  "crypto-js.min.js",
  "manifest.json",
  "translations.js",
  "favicon.png",
  "icons/icon-192.png",
  "icons/icon-512.png",
  "icons/ikon.jpg"
];

self.addEventListener("install", event => {
    event.waitUntil(
        caches.open(CACHE_NAME).then(cache => {
            return cache.addAll(ASSETS.map(url => new Request(url, {cache: 'reload'})))
            .catch(err => console.error('Cache addAll failed', err));
        })
    );
    self.skipWaiting();
});

self.addEventListener("activate", event => {
    event.waitUntil(
        caches.keys().then(keys => Promise.all(
            keys.filter(key => key !== CACHE_NAME && key.startsWith('zt-')).map(key => caches.delete(key))
        ))
    );
    self.clients.claim();
});

self.addEventListener("fetch", event => {
    if (event.request.method !== 'GET') return;
    const isHtml = event.request.mode === 'navigate' || event.request.headers.get('accept').includes('text/html');
    if (isHtml) {
        event.respondWith(
            fetch(event.request)
            .then(res => {
                return caches.open(CACHE_NAME).then(cache => {
                    cache.put(event.request, res.clone());
                    return res;
                });
            })
            .catch(() => caches.match(event.request, { ignoreSearch: true }))
        );
    } else {
        event.respondWith(
            caches.match(event.request, { ignoreSearch: true }).then(cached => {
                return cached || fetch(event.request).then(res => {
                    if (!event.request.url.includes('.mp4')) {
                        return caches.open(CACHE_NAME).then(cache => {
                            cache.put(event.request, res.clone());
                            return res;
                        });
                    }
                    return res;
                });
            }).catch(() => fetch(event.request))
        );
    }
});
