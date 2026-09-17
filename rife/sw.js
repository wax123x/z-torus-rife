const CACHE_NAME = 'zt-rife-v2.5.1';
const ASSETS = [
  "./",
  "index.html",
  "z_vault_core.js",
  "home.css",
  "info.css",
  "info.html",
  "key_generator_manifest.json",
  "kurulum.html",
  "manevi_player.html",
  "manifest.json",
  "player.html",
  "reset.html",
  "translations.js",
  "x-hub.html",
  "z-ke.html",
  "images/hub_blockage_v1.webp",
  "images/icon-192.png",
  "images/icon-512.png",
  "images/icon-popup.webp",
  "images/menu_abundance.webp",
  "images/menu_allergies.webp",
  "images/menu_animal.webp",
  "images/menu_app_bg.webp",
  "images/menu_chakras.webp",
  "images/menu_detox.webp",
  "images/menu_general.webp",
  "images/menu_hair.webp",
  "images/menu_heart.webp",
  "images/menu_info_bg.webp",
  "images/menu_pain.webp",
  "images/menu_psych.webp",
  "images/menu_site_bg.gif",
  "images/menu_skin.webp",
  "images/menu_spine.webp",
  "images/menu_spiritual.webp",
  "images/menu_support_bg.webp",
  "images/menu_weight.webp",
  "images/o_logo_optimized.webp",
  "images/quantum_banner.webp",
  "images/yeni_logo_kare.jpg",
  "images/yeni_logo_kare.png"
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
                if (!event.request.url.startsWith('http')) return res;
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
                    if (!event.request.url.includes('.mp4') && event.request.url.startsWith('http')) {
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
