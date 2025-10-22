# Servidor HTTP Webserv

# WebServ: Servidor HTTP

## Propósito del proyecto
Webserv es un servidor HTTP/1.1 implementado íntegramente en C++98. El programa lee un archivo de configuración inspirado en la sintaxis de NGINX, abre los sockets declarados y atiende solicitudes estáticas, listados de directorios, ejecuciones CGI y cargas de archivos dentro de un bucle de eventos no bloqueante basado en `select(2)`.

- **1. Capa de configuración**  
  Cuando el servidor se lanza, abre un *socket* y comienza a escuchar en un puerto especificado (ej. `8001`). Es como si una tienda levantara la persiana y anunciara: «estoy lista para atender clientes». Si un navegador se conecta, el servidor acepta la conexión y se establece un canal de comunicación.

- **2. Capa de ejecución**  
  Una vez conectados, los clientes envían peticiones HTTP que llegan como un flujo de bytes. 

Ejecuta con:

make
./webserv [ruta/al/config]

Si no se especifica argumento, se utiliza `config/default.config`. Cada bloque `server` puede escuchar en un puerto diferente dentro del mismo proceso, lo que permite servir múltiples sitios o reglas de enrutamiento de manera simultánea.

## Cómo compilar y ejecutar
1. Asegúrate de tener disponible un compilador C++98 (GCC o Clang funcionan correctamente).
2. Compila el proyecto:
   ```bash
   make
   ```
3. Inicia el servidor con el archivo de configuración deseado:
   ```bash
   ./webserv config/default.config
   ```
4. Revisa el log de arranque para confirmar qué puertos se han abierto e interactúa con ellos mediante un navegador o herramientas como `curl`:
   ```bash
   curl http://127.0.0.1:8001/
   ```

## Estructura del repositorio
- `config/` – Archivos de configuración de ejemplo con varios bloques de servidor, reglas CGI y límites de subida.
- `www/` – Recursos estáticos que se sirven por defecto.
- `cgi-bin/` y `cgi_tester` – Scripts y binarios CGI utilizados por la configuración de ejemplo.
- `include/` – Cabeceras con las interfaces públicas de cada módulo.
- `src/` – Implementaciones detalladas en la siguiente sección.
- `tester/`, `ubuntu_tester/`, etc. – Herramientas auxiliares de prueba.

## Guía de archivos fuente
| Archivo | Descripción |
| ------- | ----------- |
| `src/main.cpp` | Punto de entrada: procesa argumentos, carga la configuración mediante `ReadConfig`, construye el `ServerManager` e inicia el bucle principal. |
| `src/ReadConfig.cpp` | Tokeniza el archivo de configuración y crea instancias de `ServerUnit` con las directivas leídas. |
| `src/ConfigFile.cpp` | Funciones auxiliares para comprobar existencia, tipo y permisos de rutas durante la validación de la configuración. |
| `src/Location.cpp` | Implementa la clase `Location`, encargada de almacenar métodos permitidos, roots, alias, reglas de subida y asignaciones CGI por ruta. |
| `src/ServerUnit.cpp` | Representa un servidor virtual; valida directivas, normaliza rutas y crea sockets de escucha en modo no bloqueante con `SO_REUSEADDR`. |
| `src/ServerManager.cpp` | Núcleo del bucle de eventos: gestiona sockets de escucha, acepta clientes, multiplexa lectura/escritura con `select`, asocia peticiones con su `ServerUnit` y genera respuestas. |
| `src/Request.cpp` | Analiza la petición HTTP, extrae método, ruta y cabeceras, controla límites de cuerpo y detecta transferencias chunked. |
| `src/HttpResponse.cpp` | Construye las respuestas para GET/POST/DELETE, resuelve archivos, genera autoindex, maneja subidas y ejecuta CGI cuando corresponde. |
| `src/Cgi.cpp` | Capa de integración con CGI: prepara el entorno, lanza el script con `fork/execve`, transmite el cuerpo y captura la salida para integrarla en la respuesta HTTP. |
| `src/utils.cpp` y `include/utils.hpp` | Utilidades de cadenas y rutas (comparaciones *case-insensitive*, trims, normalización) compartidas entre módulos. |
| `src/paths.cpp` | Funciones para limpiar y canonizar URIs y rutas de sistema de archivos. |
| `src/statusCode.cpp` | Mapea códigos HTTP a sus mensajes descriptivos usados en las páginas de error. |
| `src/logging.cpp` | Proporciona utilidades de logging con color y marcas de tiempo opcionales (`logInfo`, `logError`, `logDebug`). |

## Manejo de errores
El servidor evita finalizar abruptamente en tiempo de ejecución: los errores de configuración lanzan `ErrorException` (capturados en `main.cpp`) y los problemas durante la atención de solicitudes se devuelven como códigos HTTP 5xx, manteniendo el proceso activo. Ante fallos de socket, se cierran los descriptores implicados y se eliminan de los conjuntos monitorizados antes de continuar.

## Extender la configuración
1. Duplica un bloque de `server` en `config/default.config` y ajusta `listen`, `server_name`, `root` e `index` según el nuevo sitio.
2. Para reglas específicas por ruta, añade bloques `location` definiendo métodos permitidos, `root`/`alias`, redirecciones `return`, `autoindex`, directorios de subida (`upload_store`) y asociaciones `cgi`.
3. Reinicia el servidor tras guardar la configuración para aplicar los cambios.

Consulta la configuración por defecto y esta guía de archivos cuando necesites localizar la lógica correspondiente a un comportamiento concreto.