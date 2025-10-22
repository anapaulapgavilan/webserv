#!/bin/bash
set -e

echo "[INFO] Iniciando servidor (logs en server.log)..."
./webserv > logs/server.log 2>&1 &
SERVER_PID=$!

sleep 3

echo "[INFO] Iniciando tester interactivo..."
./tester http://localhost:8001
echo "[INFO] Tester terminado. Cerrando servidor..."

sleep 3

kill $SERVER_PID
wait $SERVER_PID

echo "[INFO] Tester completado. Revisa logs en: server.log"
