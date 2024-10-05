#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <WebServer.h>

// Usar para la versión de 12 bits (ADS1015)
Adafruit_ADS1015 ads;

// Configuraciones de la red Wi-Fi
const char* ssid = "iPhone de Daniel";         // Nombre de la red Wi-Fi
const char* password = "atom2005";             // Contraseña de la red Wi-Fi

// Crear una instancia del servidor web en el puerto 80
WebServer server(80);

// Variables globales para almacenar las lecturas
float volts0 = 0.0, volts1 = 0.0;

void handleRoot() {
  // Página HTML básica con un gráfico que se actualiza automáticamente
  String html = "<html><head>";
  html += "<title>Lectura en Tiempo Real del ADS1015</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "<script>let volt0Data = []; let volt1Data = []; let timeData = []; let chart; function createChart() {";
  html += "const ctx = document.getElementById('myChart').getContext('2d');";
  html += "chart = new Chart(ctx, { type: 'line', data: { labels: timeData, datasets: [{ label: 'Canal 0', data: volt0Data, borderColor: 'rgba(75, 192, 192, 1)', fill: false }, { label: 'Canal 1', data: volt1Data, borderColor: 'rgba(192, 75, 192, 1)', fill: false }] }, options: { animation: false, responsive: true, scales: { x: { type: 'linear', position: 'bottom' }, y: { beginAtZero: true, max: 5 } } } });";
  html += "setInterval(refreshData, 2);";  // Actualiza los datos cada 100 ms
  html += "};";
  html += "function refreshData() {";
  html += "var xhttp = new XMLHttpRequest();";
  html += "xhttp.onreadystatechange = function() {";
  html += "if (this.readyState == 4 && this.status == 200) {";
  html += "var data = this.responseText.split(',');";
  html += "let volt0 = parseFloat(data[0]);";
  html += "let volt1 = parseFloat(data[1]);";
  html += "let time = (Date.now() - startTime) / 1000;";  // Usar el tiempo transcurrido
  html += "volt0Data.push(volt0); volt1Data.push(volt1); timeData.push(time);";
  html += "if (volt0Data.length > 1000) { volt0Data.shift(); volt1Data.shift(); timeData.shift(); }";  // Limitar a 100 datos
  html += "chart.update();";  // Actualizar el gráfico
  html += "}};"; 
  html += "xhttp.open('GET', '/data', true);";
  html += "xhttp.send();";
  html += "};";
  html += "window.onload = function() { startTime = Date.now(); createChart(); };";  // Crear gráfico al cargar la página
  html += "</script></head><body>";
  html += "<h1>Lectura en Tiempo Real del ADS1015</h1>";
  html += "<canvas id='myChart' width='400' height='200'></canvas>"; // Asegúrate que el canvas tiene un tamaño
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleData() {
  // Leer valores del ADC
  int16_t adc0 = ads.readADC_SingleEnded(0);
  int16_t adc1 = ads.readADC_SingleEnded(1);

  // Convertir a voltios
  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);

  // Enviar solo los valores en formato sencillo
  String data = String(volts0, 2) + "," + String(volts1, 2);

  
  server.send(200, "text/plain", data);
}

void setup(void) {
  Serial.begin(115200);

  // Inicializar ADS1015
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  // Configurar la tasa de muestreo a 3.3 kHz (3300 SPS)
  ads.setDataRate(RATE_ADS1015_3300SPS);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Conectado a la red Wi-Fi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configurar el servidor
  server.on("/", handleRoot);   // Ruta principal "/"
  server.on("/data", handleData);  // Ruta para enviar datos
  server.begin();               // Iniciar el servidor
  Serial.println("Servidor HTTP iniciado");
}

void loop(void) {
  server.handleClient();  // Manejar solicitudes de clientes
}
