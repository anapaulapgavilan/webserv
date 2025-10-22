#!/usr/bin/env python3
import os
from paths import IMG_DIR

os.makedirs(IMG_DIR, exist_ok=True)

# return divs with images in IMG_DIR

try:
    files = sorted(os.listdir(IMG_DIR))
    img_files = [f for f in files if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.bmp', '.webp'))]
    for img in img_files:
        print(f"""
        <div class="col-xl-3 col-lg-4 col-md-6 col-sm-6 col-12 mb-5">
            <figure class="effect-ming tm-video-item">
                <img src="file/{img}" alt="{img}" class="img-fluid">
                <figcaption class="d-flex align-items-center justify-content-center">
                    <h2>{img}</h2>
                    <a href="photo-detail.html?img={img}">View more</a>
                </figcaption>
            </figure>
        </div>
        """)
except Exception as e:
    print(f"<p>Error leyendo imágenes: {e}</p>")
