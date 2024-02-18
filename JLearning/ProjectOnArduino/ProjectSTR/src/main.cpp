#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>

#define DHTPIN 2
#define DHTTYPE DHT11 // DHT 11
#define motorA1 4
#define motorA2 5
#define motorB1 12
#define motorB2 13

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

const char* ssid = "IphMat";
const char* password = "110623Al";

float lastTemperature = 0.0;
float lastHumidity = 0.0;
bool motorsActivated = false;

String SendHTML()
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Control de Motores</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button:active {background-color: #2980b9;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Control de Motores</h1>\n";
  ptr += "<p><a class=\"button\" href=\"/motorAYBabrir\">Motor A Y B Abrir</a></p>\n";
  ptr += "<p><a class=\"button\" href=\"/motorAYBcerrar\">Motor A Y B Cerrar</a></p>\n";
  ptr += "<p>Temperatura: " + String(lastTemperature) + "°C</p>\n";
  ptr += "<p>Humedad: " + String(lastHumidity) + " %</p>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void handle_stopMotors()
{
  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, LOW);
  digitalWrite(motorB1, LOW);
  digitalWrite(motorB2, LOW);
  Serial.println("Motores detenidos");
}
void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML());
}

void handle_motorAYBabrir()
{
  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, HIGH);
  digitalWrite(motorB1, HIGH);
  digitalWrite(motorB2, LOW);
  Serial.println("Motor A Y B abrir");
  server.send(200, "text/html", SendHTML());

  delay(1500);         // Espera 2 segundos para detener los motores
  handle_stopMotors(); // Detiene los motores después de 2 segundos
}

void handle_motorAYBcerrar()
{
  digitalWrite(motorA1, HIGH);
  digitalWrite(motorA2, LOW);
  digitalWrite(motorB1, LOW);
  digitalWrite(motorB2, HIGH);
  Serial.println("Motor A Y B cerrar");
  server.send(200, "text/html", SendHTML());

  delay(1500);         // Espera 2 segundos para detener los motores
  handle_stopMotors(); // Detiene los motores después de 2 segundos
}

void handle_NotFound()
{
  server.send(404, "text/plain", "La pagina no existe");
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(motorB1, OUTPUT);
  pinMode(motorB2, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handle_OnConnect);
  server.on("/motorAYBabrir", HTTP_GET, handle_motorAYBabrir);
  server.on("/motorAYBcerrar", HTTP_GET, handle_motorAYBcerrar);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop()
{
  server.handleClient();

  // Antes de leer Temp, hum
  // Esperar 1 segundos entre cada lectura
  delay(1000);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  lastTemperature = t;
  lastHumidity = h;

  // Imprimir información en el monitor serie
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" °C, Humedad: ");
  Serial.print(h);
  Serial.println(" %");

  if (t >= 28 && !motorsActivated)
  {
    handle_motorAYBabrir();
    motorsActivated = true;
  }
  else if (t <= 24 && !motorsActivated)
  {
    handle_motorAYBcerrar();
    motorsActivated = true;
  }
}
