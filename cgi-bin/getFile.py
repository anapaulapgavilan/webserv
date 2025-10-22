#!/usr/bin/env python3
import os
import cgi
from paths import IMG_DIR


# Obtener el parámetro 'img' de la query string
form = cgi.FieldStorage()
img_name = form.getvalue("img", "")
safe_img = os.path.basename(img_name)
img_path = os.path.join(IMG_DIR, safe_img)

if not safe_img or not os.path.isfile(img_path):
    print("<div><h2>Imagen no encontrada</h2></div>")
else:
    print(f"""
    <div class="row tm-mb-90">            
        <div class="col-xl-8 col-lg-7 col-md-6 col-sm-12">
            <img src="file/{safe_img}" alt="{safe_img}" class="img-fluid">
        </div>
        <div class="col-xl-4 col-lg-5 col-md-6 col-sm-12">
            <div class="tm-bg-gray tm-video-details">
                <p class="mb-4">
                    Imagen: {safe_img}
                </p>
                <div class="text-center mb-5">
                    <a href="file/{safe_img}" class="btn btn-primary tm-btn-big" download>Download</a>
                </div>
                <div class="text-center mb-5">
                    <a href="#" id="delete-btn" class="btn btn-danger tm-btn-big" role="button">Delete</a>
                </div>
                <script>
                document.getElementById('delete-btn').onclick = function(e) {{
                    e.preventDefault();
                    fetch(window.location.href, {{
                        method: 'DELETE'
                    }}).then(response => {{
                        if (response.ok) {{
                            window.location.href = '/';
                        }} else {{
                            alert('Error al borrar la imagen');
                        }}
                    }});
                }};
                </script>
            </div>
        </div>
    </div>
    """)