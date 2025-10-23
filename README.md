# webserv

Es un servidor HTTP en C++. Carga un archivo de configuración inspirado en la sintaxis de Nginx (ver [`config/default.config`](config/default.config)) para crear múltiples hosts virtuales, servir recursos estáticos desde `www/`, y delegar ciertas extensiones de archivos a manejadores CGI como [`./cgi_tester`](cgi_tester).

## Compilación y ejecución
```bash
make./webserv config/default.config
```

Deja el servidor ejecutándose en ese terminal (o inícialo en segundo plano con `./webserv config/default.config &`) mientras abres otra consola para comandos de cliente. La configuración por defecto inicia tres servidores ligados a `127.0.0.1` en los puertos 8001, 8002 y 8003. Cada bloque `location` controla los métodos permitidos, la ejecución opcional de CGI, las raíces alias, la indexación automática y las redirecciones.

### ¿Qué es Siege?
Siege es un programa de línea de comandos que envía muchas peticiones HTTP a un servidor web. Piensa en él como un "martillo" que golpea tu servidor una y otra vez para comprobar si responde rápido, si entrega las páginas correctas y si aguanta cuando hay muchos usuarios a la vez.

### ¿Para qué lo usamos en este proyecto?
`webserv` es un servidor HTTP escrito en C++ que se comporta de forma parecida a Nginx. Lee la configuración de [`config/default.config`] y sirve archivos desde `www/`, ejecuta CGI y acepta cargas por POST en rutas concretas. Al usar Siege sobre tu propio servidor puedes detectar:

- Respuestas lentas cuando hay varias conexiones.
- Errores como `500` o `404` que aparezcan sólo con alta carga.
- Problemas en rutas especiales (CGI, subidas de archivos, redirecciones) que quizás no notes con pruebas manuales.

### ¿Cómo lo instalo?
1. Asegúrate de que `webserv` está compilado (`make`).
2. Ejecuta el script del repositorio:
   ```bash
   ./install_siege.sh
   ```
   Este script descarga el código fuente de Siege, lo compila y deja el binario en `~/.local/bin`. No necesitas ser administrador.

### ¿Cómo lo uso paso a paso?
1. **Arranca el servidor** en una terminal y déjalo corriendo:
   ```bash
   ./webserv config/default.config
   ```
2. **Abre otra terminal** (o pestaña) para los comandos de Siege.

3. **Lanza una prueba sencilla** contra la ruta principal (puerto 8001):
   ```bash
   siege -c10 -t30s http://127.0.0.1:8001/
   ```
   - `-c10` = 10 usuarios simulados.
   - `-t30s` = corre durante 30 segundos.

4. **Prueba la ruta de subida de archivos** (POST):
   ```bash
   echo "nombre=demo&dato=123" > /tmp/formulario.txt
   siege -c5 -r5 "http://127.0.0.1:8001/upload POST </tmp/formulario.txt"
   ```
   Aquí repetimos la petición 5 veces por cada usuario para comprobar que los archivos se guardan sin errores.

5. **Combina varias rutas** usando un archivo de URLs para probar alias, autoindex y redirecciones (puertos 8002 y 8003):
   ```bash
   cat > /tmp/webserv.urls <<'URLS'
   http://127.0.0.1:8002/contact.html
   http://127.0.0.1:8002/webinfo
   http://127.0.0.1:8002/webdev/imgs/
   http://127.0.0.1:8002/the-b2b-imgs
   http://127.0.0.1:8003/contact.html
   URLS
   siege -c8 -i -f /tmp/webserv.urls
   ```
   La opción `-i` hace que Siege recorra las URLs de forma aleatoria, parecido a usuarios navegando libremente.

### ¿Cómo sé si todo funcionó?
Cuando Siege termina muestra un resumen. Las métricas importantes son:

- **Availability (Disponibilidad):** debería estar cerca de 100%. Si es 0% y ves `socket: unable to connect ... Connection refused`, significa que el servidor no estaba corriendo o el puerto es incorrecto.
- **Response time (Tiempo de respuesta):** cuanto menor, mejor. Valores altos indican que el servidor tarda demasiado.
- **Transaction rate y Throughput:** cuántas peticiones por segundo atiende y cuántos MB se transfieren. Útil para comparar cambios en el código.

Si algo falla:

1. Comprueba que `webserv` sigue activo (la terminal del servidor no debe mostrar errores).
2. Usa `curl http://127.0.0.1:8001/` para confirmar que el puerto responde.
3. Repite la prueba ajustando `-c` (usuarios) o `-t`/`-r` (duración) para ver si el problema aparece con más o menos carga.

Con estas pruebas periódicas podrás detectar cuellos de botella o regresiones en tu servidor escrito en C/C++ antes de avanzar al siguiente ejercicio.
