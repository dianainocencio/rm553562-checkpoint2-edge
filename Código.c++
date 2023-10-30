#include <Wire.h>
#include <LiquidCrystal.h>
#include <DHT.h>

// Definindo os pinos e o tipo do sensor DHT
#define DHTPIN 9
#define DHTTYPE DHT22
#define GREEN_LED 13
#define YELLOW_LED 12
#define RED_LED 11
#define BUZZER_PIN 10
#define LDR_PIN A0

// Constantes para o cálculo da luminosidade
const float GAMMA = 0.7;
const float RL10 = 50;


// Inicialização dos objetos para o sensor DHT e o LCD
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Variáveis para armazenar os valores dos sensores e o tempo da última atualização do display
unsigned long sensor_LDR = 0;
unsigned long sensor_Temperature = 0;
unsigned long sensor_Humidity = 0;
unsigned long lastDisplayUpdate = 0;

// Variáveis para o controle do buzzer
unsigned long buzzerStartTime = 0;
unsigned long buzzerOffStartTime = 0;
bool buzzerActive = false;
bool isBuzzerOff = false;

// Estado e modo de operação do buzzer
bool buzzerState = false;
bool buzzerContinuous = false;

// Valores lidos dos sensores
float lux = 0;
float temperature = 0;
float humidity = 0;

// Variável para controle de qual informação será mostrada no display
int displayUpdate = 0;

// Buffers para armazenar leituras dos sensores
int read_LDR[5] = {0};
int read_LDR_index = 0;
int total_LDR = 0;

int read_Temperature[5] = {0};
int read_Temperature_index = 0;
int total_Temperature = 0;

int read_Humidity[5] = {0};
int read_Humidity_index = 0;
int total_Humidity = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  readLuminosity();
  readTemperature();
  readHumidity();
  controlLeds();
  controlBuzzer();

  // Atualiza o display a cada 5 segundos
  if (millis() - lastDisplayUpdate >= 5000) {
    controlDisplay();
    lastDisplayUpdate = millis();
  }

  Serial.print("Luminosidade (lux): ");
  Serial.println(lux);
  Serial.print("Temperatura (°C): ");
  Serial.println(temperature);
  Serial.print("Umidade (%): ");
  Serial.println(humidity);
}

// Lê a luminosidade a cada 100 ms
void readLuminosity() {
  if((millis()- sensor_LDR)>= 100) {
    sensor_LDR = millis();
    // Cálculo da média móvel para a luminosidade
    total_LDR = total_LDR - read_LDR[read_LDR_index];
    read_LDR[read_LDR_index] = analogRead(LDR_PIN);
    total_LDR = total_LDR + read_LDR[read_LDR_index];
    read_LDR_index = read_LDR_index + 1;

    if (read_LDR_index >= 5) {
      read_LDR_index = 0;
    }
    
    // Cálculo da luminosidade em lux
    float light = total_LDR / 5;
    float voltage = light / 1024. * 5;
    float resistance = 2000 * voltage / (1 - voltage / 5);
    lux = pow(RL10  * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
    }
}

// Lê a temperatura a cada 100 ms
void readTemperature() {
  if ((millis() - sensor_Temperature) >= 100) {
    // Cálculo da média móvel para a temperatura
    sensor_Temperature = millis();
    total_Temperature = total_Temperature - read_Temperature[read_Temperature_index];
    read_Temperature[read_Temperature_index] = dht.readTemperature() * 10;
    total_Temperature = total_Temperature + read_Temperature[read_Temperature_index];
    read_Temperature_index = (read_Temperature_index + 1) % 5;
    temperature = (float)total_Temperature / 50.0;
  }
}

void readHumidity() {
  // Lê a umidade a cada 100 ms
  if ((millis() - sensor_Humidity) >= 100) {
    sensor_Humidity = millis();
    // Cálculo da média móvel para a umidade
    total_Humidity = total_Humidity - read_Humidity[read_Humidity_index];
    read_Humidity[read_Humidity_index] = dht.readHumidity() * 10;
    total_Humidity = total_Humidity + read_Humidity[read_Humidity_index];
    read_Humidity_index = (read_Humidity_index + 1) % 5;
    humidity = (float)total_Humidity / 50.0;
  }
}

void controlLeds() {
  // Controla os LEDs com base na luminosidade
  if (lux < 300) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
  } else if (lux >= 300 && lux < 600) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }
}

void controlDisplay() {
  lcd.clear();

  if (displayUpdate > 2){
    displayUpdate = 0;
  }
  // Mostra informações no display com base na variável displayUpdate
  // Luminosidade
  if (displayUpdate == 0) {
    if (lux < 200) {
      lcd.setCursor(0, 0);
      lcd.print("Ambiente Escuro");
    } else if (lux >= 200 && lux < 3000) {
      lcd.print("Ambiente Meia");
      lcd.setCursor(0, 1);
      lcd.print("Luz");
    } else {
      lcd.print("Ambiente Muito");
      lcd.setCursor(0, 1);
      lcd.print("Claro");
    }
  }

  // Temperatura
  if (displayUpdate == 1) {
  if (temperature >= 10 && temperature <= 15) {
    lcd.setCursor(0, 0);
    lcd.print("Temperatura OK");
    lcd.setCursor(0, 1);
    lcd.print("Temp. = ");
    lcd.print(temperature, 1);
    lcd.print("C");
  } else if (temperature > 15) {
    lcd.setCursor(0, 0);
    lcd.print("Temp. ALTA");
    lcd.setCursor(0, 1);
    lcd.print("Temp. = ");
    lcd.print(temperature, 1);
    lcd.print("C");
  } else if (temperature < 10) {
    lcd.setCursor(0, 0);
    lcd.print("Temp. BAIXA");
    lcd.setCursor(0, 1);
    lcd.print("Temp. = ");
    lcd.print(temperature, 1);
    lcd.print("C");
  }
  }

  // Umidade
  if (displayUpdate == 2) {
  if (humidity >= 50 && humidity <= 80) {
    lcd.setCursor(0, 0);
    lcd.print("Umidade OK");
    lcd.setCursor(0, 1);
    lcd.print("Umidade = ");
    lcd.print(humidity, 1);
    lcd.print("%");
  } else if (humidity > 80) {
    lcd.setCursor(0, 0);
    lcd.print("Umidade ALTA");
    lcd.setCursor(0, 1);
    lcd.print("Umidade = ");
    lcd.print(humidity, 1);
    lcd.print("%");
  } else if (humidity < 50) {
    lcd.setCursor(0, 0);
    lcd.print("Umidade BAIXA");
    lcd.setCursor(0, 1);
    lcd.print("Umidade = ");
    lcd.print(humidity, 1);
    lcd.print("%");
  }
}
displayUpdate = displayUpdate + 1;
}

void controlBuzzer() {
  // Controle do buzzer com base na temperatura e umidade
  static unsigned long buzzerStartTime = 0;
  static unsigned long buzzerOffStartTime = 0;
  static bool isBuzzerOn = false;

  if (temperature < 10 && humidity < 60) {
    if (isBuzzerOn) {
      if (millis() - buzzerStartTime >= 3000) {
        noTone(BUZZER_PIN);
        isBuzzerOn = false;
        buzzerOffStartTime = millis();
      }
    } else {
      if (millis() - buzzerOffStartTime >= 3000) {
        tone(BUZZER_PIN, 1000);
        isBuzzerOn = true;
        buzzerStartTime = millis();
      }
    }
  } else if (temperature > 15 && humidity > 80) {
    tone(BUZZER_PIN, 1000);
    isBuzzerOn = true; 
  } else {
    noTone(BUZZER_PIN);
    isBuzzerOn = false; 
  }
}
