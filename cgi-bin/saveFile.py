#!/usr/bin/env python3
import os
import sys
from email import policy
from email.parser import BytesParser
from paths import IMG_DIR


def parse_multipart_form_data():
    """Parse multipart form data from stdin"""
    try:
        # Get content type and boundary from environment
        content_type = os.environ.get('CONTENT_TYPE', '')
        content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
        
        print(f"<p>DEBUG: CONTENT_TYPE={content_type}, CONTENT_LENGTH={content_length}</p>")
        sys.stderr.write(f"DEBUG: CONTENT_TYPE={content_type}, CONTENT_LENGTH={content_length}\n")
        
        if not content_type.startswith('multipart/form-data'):
            print("<p>DEBUG: Not multipart/form-data</p>")
            return {}
            
        # Read the raw data
        raw_data = sys.stdin.buffer.read(content_length)
        print(f"<p>DEBUG: Read {len(raw_data)} bytes from stdin</p>")
        sys.stderr.write(f"DEBUG: Read {len(raw_data)} bytes from stdin\n")
        
        # Create an email message to parse the multipart data
        msg_str = f"Content-Type: {content_type}\r\n\r\n"
        msg_str = msg_str.encode() + raw_data
        
        msg = BytesParser(policy=policy.default).parsebytes(msg_str)
        
        form_data = {}
        
        for part in msg.walk():
            if part.get_content_disposition() == 'form-data':
                name = part.get_param('name', header='content-disposition')
                filename = part.get_param('filename', header='content-disposition')
                
                print(f"<p>DEBUG: Found part name={name}, filename={filename}</p>")
                sys.stderr.write(f"DEBUG: Found part name={name}, filename={filename}\n")
                
                if name:
                    if filename:  # File upload
                        form_data[name] = {
                            'filename': filename,
                            'content': part.get_content(),
                            'content_type': part.get_content_type()
                        }
                    else:  # Regular form field
                        form_data[name] = part.get_content()
        
        return form_data
        
    except Exception as e:
        print(f"<p>Error parsing form data: {e}</p>")
        sys.stderr.write(f"Error parsing form data: {e}\n")
        return {}

# Parse the form data
form = parse_multipart_form_data()

if "file" in form and isinstance(form["file"], dict):
    fileitem = form['file']
    filename = fileitem.get('filename')
    print(f"<p>Filename recibido: {filename}</p>")

    if filename:
        safe_filename = os.path.basename(filename)
        # Ruta relativa a la carpeta del script
        filepath = os.path.join(IMG_DIR, safe_filename)
        filepath = os.path.abspath(filepath)

        try:
            content = fileitem['content']
            if isinstance(content, str):
                # Para binarios, latin1 es seguro porque mapea bytes 1:1
                content = content.encode('latin1')
            with open(filepath, 'wb') as f:
                f.write(content)
            print(f"<p>Archivo '{safe_filename}' subido con éxito.</p>")
        except Exception as e:
            print(f"<p>Error al guardar el archivo: {e}</p>")
    else:
        print("<p>No se ha enviado ningún archivo2.</p>")
else:
    print("<p>No se ha enviado ningún archivo.</p>")
