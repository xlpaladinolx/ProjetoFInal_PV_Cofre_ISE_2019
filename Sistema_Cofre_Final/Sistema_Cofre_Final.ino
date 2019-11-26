#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Keypad.h>
#include <ctype.h>
#include <SPI.h>
#include <RFID.h>
#include <Stepper.h>
#include <LiquidCrystal.h>

// Dados Wi-Fi-------------------------------------
#define FIREBASE_HOST "cofredb.firebaseio.com"
#define FIREBASE_AUTH "NwS4ClcQoz2sIo0HbQZjc2kvQJdyhiicyJCfuJuw"
#define WIFI_SSID "BETH"
#define WIFI_PASSWORD "toby0410"
//--------------------------------------------------
// RFID---------------------------------------------
#define SS_PIN 21
#define RST_PIN 22
RFID rfid(SS_PIN, RST_PIN);
int taglida;
//--------------------------------------------------
// LCD----------------------------------------------
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(17, 5, 36, 39, 34, 35);
//--------------------------------------------------
// MOTOR--------------------------------------------
const int stepsPerRevolution = 64; // change this to fit the number of steps per revolution
Stepper myStepper(stepsPerRevolution, 15, 4, 2, 16);
//--------------------------------------------------
// JSON-RFID----------------------------------------
String jsonStr;
//--------------------------------------------------
// Teclado------------------------------------------
const byte qtdLinhas = 4; //QUANTIDADE DE LINHAS DO TECLADO
const byte qtdColunas = 4; //QUANTIDADE DE COLUNAS DO TECLADO
char matriz_teclas[qtdLinhas][qtdColunas] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte PinosqtdLinhas[qtdLinhas] = {13, 12, 14, 27}; //PINOS UTILIZADOS PELAS LINHAS
byte PinosqtdColunas[qtdColunas] = {26, 25, 33, 32}; //PINOS UTILIZADOS PELAS COLUNAS
Keypad meuteclado = Keypad( makeKeymap(matriz_teclas), PinosqtdLinhas, PinosqtdColunas, qtdLinhas, qtdColunas);
//--------------------------------------------------
// Banco de Dados-----------------------------------
FirebaseData firebaseData;
String path = "/";
String loginDigi = "";
int senhaBD;
String senhaDigi;
//--------------------------------------------------
void consultaBD()
{
  int i = 0;
  if (Firebase.getInt(firebaseData, path + loginDigi))
  {
    printResult(firebaseData);
  }
  else
  {
    lcd.println("Login não Existe");
    loginDigi  = "";
    ESP.restart();
  }
  Serial.println("Digite a Senha:");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digite a Senha:");
  while ( i != 1)
  {
    char tecla_pressionada = meuteclado.getKey();
    if (tecla_pressionada) { //SE ALGUMA TECLA FOR PRESSIONADA, FAZ
      if (tecla_pressionada == '#')
      {
        Serial.println("");
        Serial.println(senhaBD);
        Serial.println(senhaDigi.toInt());
        if (senhaBD == senhaDigi.toInt())
        {
          Serial.println("Acesso Liberado");
          i = 1;
          abrirCofre();
        } else {
          Serial.println("Acesso Negado");
          loginDigi  = "";
          ESP.restart();
        }
      } else {
        Serial.print(tecla_pressionada);
        senhaDigi += tecla_pressionada;
      }
    }

  }
}

void consultaRFID()
{
  if (Firebase.getInt(firebaseData, path + loginDigi))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bem Vindo Convidado!");
    Serial.println("Acesso Liberado");
    abrirCofre();
  }
  else
  {
    Serial.println("Acesso Negado");
    loginDigi  = "";
    ESP.restart();
  }
}

void printResult(FirebaseData &data)
{
  senhaBD = data.intData();
}

void abrirCofre()
{
  loginDigi  = "";
  senhaDigi  = "";
  myStepper.step(400);
  int tempoatual = millis();
  int auxi = 0;
  while (millis() - tempoatual < 15000) {
    char tecla_pressionada = meuteclado.getKey();
    if (tecla_pressionada) { //SE ALGUMA TECLA FOR PRESSIONADA, FAZ
      if (tecla_pressionada == '*')
      {
        auxi = 1;
        fecharCofre();
        break;
      }
    }
  }
  if (auxi = 0)
  {
    fecharCofre();
  }
}

void fecharCofre()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fechando Cofre");
  myStepper.step(-400);
  ESP.restart();
}

void setup()
{
  Serial.begin(115200);
  // Conexão WiFi-----------------------------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //------------------------------------------
  // LCD--------------------------------------
  lcd.begin(16, 2);
  //------------------------------------------
  // MOTOR------------------------------------
  myStepper.setSpeed(200);
  //------------------------------------------
  // RFID-------------------------------------
  SPI.begin(); // Init SPI bus
  //rfid.PCD_Init(); // Init MFRC522
  rfid.init();
  //------------------------------------------
  // Firebase---------------------------------
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  //------------------------------------------

}
void loop()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digite o Login:");
  char tecla_pressionada = meuteclado.getKey(); //VERIFICA SE ALGUMA DAS TECLAS FOI PRESSIONADA

  if (tecla_pressionada) { //SE ALGUMA TECLA FOR PRESSIONADA, FAZ
    if (tecla_pressionada == '#')
    {
      Serial.println("");
      Serial.println(loginDigi);
      consultaBD();
    }
    Serial.print(tecla_pressionada);
    loginDigi += tecla_pressionada;
  }
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      loginDigi += (String)rfid.serNum[0];
      loginDigi += (String)rfid.serNum[1];
      loginDigi += (String)rfid.serNum[2];
      loginDigi += (String)rfid.serNum[3];
      loginDigi += (String)rfid.serNum[4];
      Serial.println(loginDigi);
      delay(2000);
      consultaRFID();

    }
  }
}
