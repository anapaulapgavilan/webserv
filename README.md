
# WebServ: Servidor HTTP

## What the project does

- **1. Capa de configuración**  
  Cuando el servidor se lanza, abre un *socket* y comienza a escuchar en un puerto especificado (ej. `8001`). Es como si una tienda levantara la persiana y anunciara: «estoy lista para atender clientes». Si un navegador se conecta, el servidor acepta la conexión y se establece un canal de comunicación.

- **2. Capa de ejecución**  
  Una vez conectados, los clientes envían peticiones HTTP que llegan como un flujo de bytes. 


## How to get started with the project
- Clone the project
- Select or create a new config file to use.
  - `default.conf` is used by default.
- Execute like this:
```bash
make
./webserv [config_file.conf]
```
- You will see a message with the webserver addresses that are listening
- Use `curl` or a web browser to send requests to the web being served.



## Errores comunes y cómo se manejan

| Caso                                            | Componente   | Acción                                   | Código HTTP resultante |
| ----------------------------------------------- | ------------ | ---------------------------------------- | ---------------------- |
| Puerto ocupado al *bind()*                      | ServerSetUp  | Lanza excepción → captura en `main()`    | – (cierra programa)    |
| `select()` devuelve `EBADF` o `EINTR`           | ServerManager| Re‑construye `_recv_fd_pool` / re‑intenta| 500 si se detecta roto |
| Ruta estática no encontrada                     | HttpResponse | Sirve `error_pages[404]`  o mensaje gen. | 404 Not Found          |
| Límite de cuerpo superado (`client_max_body_size`)| HttpRequest  | Detiene lectura, marca error            | 413 Payload Too Large  |

