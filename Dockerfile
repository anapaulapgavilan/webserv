FROM debian:bookworm

# Soporte para arquitectura x86 en Mac ARM
# Docker Desktop lo maneja automáticamente con emulación (QEMU)

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    curl \
    python3 \
    python3-pip \
    libc6 \
    libstdc++6 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Crea directorio del proyecto
WORKDIR /app

# Copia el código fuente y tester
COPY cgi-bin/ cgi-bin/
COPY config/ config/
COPY include/ include/
COPY src/ src/
COPY www/ www/
COPY YoupiBanane/ YoupiBanane/
COPY Makefile .
COPY ub_cgi_tester cgi_tester
COPY ub_tester tester
COPY entrypoint.sh .

# Compila el servidor
RUN make

# Permisos para ejecutar el tester
RUN chmod +x tester cgi_tester entrypoint.sh

# Exponer puerto para el servidor web (ajústalo según tu código)
EXPOSE 8001

# Iniciar el servidor en background y luego ejecutar el tester
CMD ["./entrypoint.sh"]

# To build and run the Docker container:

# docker build --platform=linux/amd64 -t c_webserv .

# docker run --rm -it \
# --platform=linux/amd64 \
#   -p 8001:8001 \
#   -v "$(pwd)/logs:/app/logs" \
#   c_webserv

# docker run --rm -it --platform=linux/amd64 -p 8001:8001 -v "$(pwd)/logs:/app/logs" c_webserv

