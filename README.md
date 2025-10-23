# webserv

Es un servidor HTTP en C++. Carga un archivo de configuración inspirado en la sintaxis de Nginx (ver [`config/default.config`](config/default.config)) para crear múltiples hosts virtuales, servir recursos estáticos desde `www/`, y delegar ciertas extensiones de archivos a manejadores CGI como [`./cgi_tester`](cgi_tester).

## Compilación y ejecución

## 1. Launch the installation of siege with homebrew.

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

## 2. Ask explanations about the basics of an HTTP server.

Piensa en HTTP como una conversación muy ordenada entre un *cliente* (por ejemplo, tu navegador o una herramienta como `curl`) y un *servidor* (tu programa `webserv`). Esa conversación sigue unas reglas fijas:

1. El cliente abre una conexión TCP al puerto donde escucha el servidor (en este proyecto, 8001–8003).
2. El cliente envía una **petición HTTP**. Siempre arranca con una línea como `GET /index.html HTTP/1.1`, donde `GET` es el método (qué acción quiere hacer), `/index.html` es el recurso y `HTTP/1.1` la versión del protocolo.
3. Después vienen cabeceras (`headers`) en formato `Nombre: Valor` que aportan información extra: `Host`, `Content-Length`, `Content-Type`, etc. Una línea en blanco indica que termina la parte de cabeceras.
4. Opcionalmente, la petición puede incluir un **cuerpo** (body), por ejemplo cuando haces un `POST` con datos de un formulario o subes un archivo.
5. El servidor lee la petición, decide cómo responder (buscar un archivo en `www/`, ejecutar un CGI, devolver un error) y envía una **respuesta HTTP** con esta estructura:
   - Línea de estado: `HTTP/1.1 200 OK` (código y mensaje).
   - Cabeceras (por ejemplo `Content-Type: text/html`).
   - Línea en blanco.
   - Cuerpo con el contenido solicitado (HTML, JSON, imagen, etc.).
6. Dependiendo de la configuración, la conexión se mantiene para más peticiones (keep-alive) o se cierra.

Algunos conceptos clave:

- **Métodos**: `GET` (lee un recurso), `POST` (envía datos), `DELETE` (borra), `PUT` (sustituye/crea). `webserv` debe validar qué métodos permite cada `location`.
- **Códigos de estado**: `200` (OK), `404` (no encontrado), `500` (error interno), `301` (redirección). Son fundamentales para que el cliente entienda qué ocurrió.
- **Cabeceras**: aportan metadatos. Por ejemplo, sin `Content-Length` el receptor no sabe cuándo termina el cuerpo.
- **Estática vs. dinámica**: servir archivos desde disco es “estático”. Cuando delegas una extensión a `cgi_tester`, estás ejecutando un programa externo que genera la respuesta “dinámica”.

El rol principal de tu servidor es seguir estas reglas estrictamente: leer bytes de la conexión, parsearlos según HTTP, preparar una respuesta correcta y escribirla de vuelta. Herramientas como Siege solo simulan muchos clientes para comprobar que incluso bajo presión sigues cumpliendo ese contrato.

## 3. Ask what function the group used for I/O Multiplexing. 

Cuando un servidor HTTP atiende a varios clientes a la vez necesita vigilar muchos sockets simultáneamente. Hacer un `recv()` o `send()` bloqueante por socket sería ineficiente, porque el proceso se quedaría "dormido" esperando a que lleguen bytes en un único descriptor y los demás clientes quedarían ignorados. Para evitarlo, este proyecto utiliza la llamada al sistema **`select()`**, una API clásica de Unix que permite revisar varios descriptores de archivo en una sola operación.

1. **Inclusión de la cabecera necesaria.** En el archivo común [`include/WebServ.hpp`](include/WebServ.hpp) se importa `<sys/select.h>`, lo que habilita las macros y estructuras (`fd_set`, `FD_SET`, `FD_ISSET`, etc.) que necesita `select`. También se definen constantes como `FD_SETSIZE`, que limita el número máximo de descriptores vigilables por este mecanismo.
2. **Conjuntos de lectura y escritura.** La clase `ServerManager` mantiene dos estructuras `fd_set`: `_read_fds` para sockets que deben vigilarse por lectura y `_write_fds` para los que tienen datos pendientes de envío. Estas estructuras se inicializan con `FD_ZERO` y se alimentan con `FD_SET` cada vez que se abre un socket nuevo o se programa una respuesta.
3. **Bucle principal con `select`.** Dentro de [`ServerManager::init`](src/ServerManager.cpp) se crea un bucle infinito mientras el servidor esté en ejecución. En cada iteración se copian los conjuntos permanentes (`_read_fds`, `_write_fds`) a copias temporales y se invoca `select(_max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL)`. Esa llamada única consulta **al mismo tiempo** los descriptores marcados para lectura y para escritura; no hay dos `select` separados ni un orden preferente. El primer argumento indica cuál es el descriptor más alto vigilado, y los parámetros `NULL` finales significan que no se vigilan errores ni se usa temporizador: el proceso queda bloqueado hasta que algún descriptor cambia de estado.
4. **Reacción a los sockets listos.** `select` devuelve cuántos descriptores están listos. El código recorre de `0` a `_max_fd` y utiliza `FD_ISSET` sobre las copias temporales para decidir qué hacer:
   - Si el descriptor listo pertenece a un socket de escucha (está en `_servers_map`), se llama a `_handle_new_connection` para aceptar al cliente con `accept()` y añadir su descriptor a los conjuntos.
   - Si el descriptor listo pertenece a un cliente y está marcado para lectura, `_handle_read` extrae la petición HTTP.
   - Si el descriptor está en el conjunto de escritura, `_handle_write` envía los bytes pendientes de la respuesta.
5. **Actualización dinámica.** Cuando un cliente cierra la conexión o la respuesta terminó, el servidor retira su descriptor con `FD_CLR` y libera la memoria asociada. De esta forma, el próximo `select` solo vigila los sockets activos.

Para alguien que empieza con C++ es útil pensar en `select` como un "vigilante" que nos avisa qué sockets merecen atención antes de gastar tiempo en operaciones de E/S. Gracias a esta estrategia, `webserv` puede mantener decenas de conexiones concurrentes en un único hilo sin necesidad de crear procesos o hilos adicionales por cada cliente.

## 4. Ask for an explanation of how does select() (or equivalent) work.

Imagina que el sistema operativo lleva una libreta con casillas numeradas, una por cada descriptor de archivo (0 para `stdin`, 1 para `stdout`, etc.). El tipo `fd_set` no es más que un **arreglo de bits** que marca qué casillas nos interesan. Las macros `FD_SET(fd, &read_set)` y `FD_CLR` encienden o apagan esos bits; `FD_ISSET` pregunta si una casilla concreta está activada después de volver de `select`.

Cuando llamamos a `select(nfds, &read_set, &write_set, NULL, timeout)` suceden varias cosas:

1. **Copias de trabajo.** El kernel duplica los conjuntos que le pasamos (por eso en el bucle usamos copias temporales). Las versiones en espacio de usuario quedan intactas hasta que se sustituyen manualmente.
2. **Bloqueo eficiente.** El hilo queda dormido dentro de la llamada, pero el kernel sigue vigilando todos los descriptores hasta que alguno cambia de estado (listo para leer, escribir o con error). Si especificamos un `timeout`, la llamada termina también al agotarse el tiempo. Aquí usamos `NULL`, así que esperamos indefinidamente.
3. **Marcas de resultado.** Cuando `select` despierta, sobrescribe los `fd_set` que pasamos con un nuevo patrón de bits: solo permanecen activos los descriptores que están listos. El valor de retorno indica cuántos son.
4. **Recorrido del usuario.** Ya en nuestro código, iteramos por cada descriptor y consultamos `FD_ISSET` para decidir la acción adecuada (aceptar, leer, escribir). Los descriptores no listos simplemente se omiten en esa vuelta del bucle.

Este mecanismo también explica dos limitaciones clásicas:

- `FD_SETSIZE` (definido normalmente en 1024) fija el máximo de descriptores vigilables en un solo `fd_set`. Si el servidor necesita miles de conexiones, conviene migrar a `poll`, `epoll` o `kqueue`.
- Al reescribir los conjuntos, es obligatorio **volver a añadir** cualquier descriptor que queramos seguir observando en la siguiente iteración (en `webserv` lo hacemos manteniendo los conjuntos "maestros" `_read_fds` y `_write_fds`).

Entender que `select` opera con máscaras de bits y que el kernel rellena esas máscaras por nosotros ayuda a visualizar por qué se requieren las copias, por qué el bucle revisa todos los ficheros hasta `_max_fd`, y cómo la llamada evita que el servidor malgaste CPU girando activamente sobre cada socket.

## 5. Ask if they use only one select() (or equivalent) and how they've managed the server to accept and the client to read/write.

Sí. El servidor mantiene un bucle principal en `ServerManager::init` y dentro de ese bucle hay **una única llamada a `select()` por iteración**. No existen otras funciones de multiplexación ni llamadas anidadas: esa invocación bloquea el hilo hasta que al menos un descriptor cambia de estado. Tras despertar, el código recorre una vez la lista de descriptores activos (`0` hasta `_max_fd`) y decide qué hacer con cada uno según aparezca en el conjunto de lectura o en el de escritura.

¿Cómo se aceptan los clientes y se gestionan sus lecturas/escrituras?

- **Aceptar conexiones nuevas.** Los descriptores de escucha (uno por `server` en la configuración) se guardan en `_servers_map`. Si `FD_ISSET` marca uno de esos descriptores tras la llamada a `select`, se invoca `_handle_new_connection`. Ese método ejecuta `accept()` para obtener el socket del cliente, lo pasa a modo no bloqueante, lo añade a `_read_fds` con `FD_SET` y lo registra en los mapas auxiliares (`_client_server_map`, `_read_requests`, `_write_buffer`, `_bytes_sent`). Así, la siguiente iteración del bucle podrá recibir datos de ese cliente.
- **Leer peticiones.** Cuando `select` indica que un descriptor de cliente está listo para lectura, `ServerManager` ejecuta `_handle_read`. Este método usa `recv()` en bloques de `BUFFER_SIZE`, acumula la petición en `ClientRequest::buffer`, intenta parsear cabeceras y comprueba si ya llegó todo el cuerpo. Si detecta que la respuesta está lista, mueve el descriptor del conjunto de lectura al de escritura (`FD_CLR` + `FD_SET`) para que la próxima iteración lo atienda como escritor.
- **Escribir respuestas.** Si el descriptor aparece en el conjunto de escritura, `_handle_write` envía los bytes pendientes desde `_write_buffer[client_sock]`, actualiza `_bytes_sent` y decide si debe cerrar la conexión o volver a escuchar más peticiones (`keep-alive`). Dependiendo del resultado, retira o recoloca el descriptor en los conjuntos adecuados.

Este flujo deja claro que toda la multiplexación gira alrededor de un único `select()`, y que la lógica de aceptar, leer y escribir se controla cambiando el descriptor entre los conjuntos `fd_set` que se pasan a esa misma llamada en las iteraciones sucesivas.

### ¿Cómo funciona `select()` internamente?

Imagina que el sistema operativo lleva una libreta con casillas numeradas, una por cada descriptor de archivo (0 para `stdin`, 1 para `stdout`, etc.). El tipo `fd_set` no es más que un **arreglo de bits** que marca qué casillas nos interesan. Las macros `FD_SET(fd, &read_set)` y `FD_CLR` encienden o apagan esos bits; `FD_ISSET` pregunta si una casilla concreta está activada después de volver de `select`.

Cuando llamamos a `select(nfds, &read_set, &write_set, NULL, timeout)` suceden varias cosas:

1. **Copias de trabajo.** El kernel duplica los conjuntos que le pasamos (por eso en el bucle usamos copias temporales). Las versiones en espacio de usuario quedan intactas hasta que se sustituyen manualmente.
2. **Bloqueo eficiente.** El hilo queda dormido dentro de la llamada, pero el kernel sigue vigilando todos los descriptores hasta que alguno cambia de estado (listo para leer, escribir o con error). Si especificamos un `timeout`, la llamada termina también al agotarse el tiempo. Aquí usamos `NULL`, así que esperamos indefinidamente.
3. **Marcas de resultado.** Cuando `select` despierta, sobrescribe los `fd_set` que pasamos con un nuevo patrón de bits: solo permanecen activos los descriptores que están listos. El valor de retorno indica cuántos son.
4. **Recorrido del usuario.** Ya en nuestro código, iteramos por cada descriptor y consultamos `FD_ISSET` para decidir la acción adecuada (aceptar, leer, escribir). Los descriptores no listos simplemente se omiten en esa vuelta del bucle.

Este mecanismo también explica dos limitaciones clásicas:

- `FD_SETSIZE` (definido normalmente en 1024) fija el máximo de descriptores vigilables en un solo `fd_set`. Si el servidor necesita miles de conexiones, conviene migrar a `poll`, `epoll` o `kqueue`.
- Al reescribir los conjuntos, es obligatorio **volver a añadir** cualquier descriptor que queramos seguir observando en la siguiente iteración (en `webserv` lo hacemos manteniendo los conjuntos "maestros" `_read_fds` y `_write_fds`).

Entender que `select` opera con máscaras de bits y que el kernel rellena esas máscaras por nosotros ayuda a visualizar por qué se requieren las copias, por qué el bucle revisa todos los ficheros hasta `_max_fd`, y cómo la llamada evita que el servidor malgaste CPU girando activamente sobre cada socket.

## 6. The select() (or equivalent) should be in the main loop and should check file descriptors for read and write AT THE SAME TIME. If not, the grade is 0 and the evaluation process ends now.

El bucle principal llama a select(_max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL) una sola vez por iteración y que en esa misma llamada vigila simultáneamente los descriptores de lectura y de escritura, reforzando cómo el servidor atiende ambas direcciones de I/O en cada vuelta.

## 7. There should be only one read or one write per client per select() (or equivalent). Ask the group to show you the code from the select() (or equivalent) to the read and write of a client.

Si quieres ver exactamente cómo se aplica la regla "una lectura **o** una escritura por cliente y por `select`", aquí tienes el fragmento de [`ServerManager::init`](src/ServerManager.cpp) que se ejecuta justo después de la llamada:

```cpp
int activity = select(_max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL);
if (activity < 0) { /* ... */ }

for (int fd = 0; fd <= _max_fd; ++fd) {
    bool handled = false; // una operación por descriptor en esta iteración
    if (FD_ISSET(fd, &temp_read_fds)) {
        if (_servers_map.find(fd) != _servers_map.end()) {
            _handle_new_connection(fd);
        } else {
            _handle_read(fd);
        }
        handled = true;
    }
    if (!handled && FD_ISSET(fd, &temp_write_fds)) {
        _handle_write(fd);
    }
}
```

1. `select` marca los descriptores listos en las copias `temp_read_fds` y `temp_write_fds`.
2. El bucle recorre todos los posibles `fd`. Para cada uno se inicializa `handled` en `false`.
3. Si el descriptor estaba listo para lectura se atiende primero (`_handle_new_connection` o `_handle_read`), y entonces `handled` pasa a `true`.
4. La rama de escritura (`_handle_write`) solo se ejecuta cuando el descriptor **no** fue procesado antes en lectura. Así se cumple la restricción de “una sola llamada a `recv()` o `send()` por socket y por despertar de `select`”.

### ¿Cómo funciona `select()` internamente?

Imagina que el sistema operativo lleva una libreta con casillas numeradas, una por cada descriptor de archivo (0 para `stdin`, 1 para `stdout`, etc.). El tipo `fd_set` no es más que un **arreglo de bits** que marca qué casillas nos interesan. Las macros `FD_SET(fd, &read_set)` y `FD_CLR` encienden o apagan esos bits; `FD_ISSET` pregunta si una casilla concreta está activada después de volver de `select`.

Cuando llamamos a `select(nfds, &read_set, &write_set, NULL, timeout)` suceden varias cosas:

1. **Copias de trabajo.** El kernel duplica los conjuntos que le pasamos (por eso en el bucle usamos copias temporales). Las versiones en espacio de usuario quedan intactas hasta que se sustituyen manualmente.
2. **Bloqueo eficiente.** El hilo queda dormido dentro de la llamada, pero el kernel sigue vigilando todos los descriptores hasta que alguno cambia de estado (listo para leer, escribir o con error). Si especificamos un `timeout`, la llamada termina también al agotarse el tiempo. Aquí usamos `NULL`, así que esperamos indefinidamente.
3. **Marcas de resultado.** Cuando `select` despierta, sobrescribe los `fd_set` que pasamos con un nuevo patrón de bits: solo permanecen activos los descriptores que están listos. El valor de retorno indica cuántos son.
4. **Recorrido del usuario.** Ya en nuestro código, iteramos por cada descriptor y consultamos `FD_ISSET` para decidir la acción adecuada (aceptar, leer, escribir). Los descriptores no listos simplemente se omiten en esa vuelta del bucle.

Este mecanismo también explica dos limitaciones clásicas:

- `FD_SETSIZE` (definido normalmente en 1024) fija el máximo de descriptores vigilables en un solo `fd_set`. Si el servidor necesita miles de conexiones, conviene migrar a `poll`, `epoll` o `kqueue`.
- Al reescribir los conjuntos, es obligatorio **volver a añadir** cualquier descriptor que queramos seguir observando en la siguiente iteración (en `webserv` lo hacemos manteniendo los conjuntos "maestros" `_read_fds` y `_write_fds`).

Entender que `select` opera con máscaras de bits y que el kernel rellena esas máscaras por nosotros ayuda a visualizar por qué se requieren las copias, por qué el bucle revisa todos los ficheros hasta `_max_fd`, y cómo la llamada evita que el servidor malgaste CPU girando activamente sobre cada socket.
