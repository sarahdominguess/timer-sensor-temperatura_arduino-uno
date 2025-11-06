#include <Keypad.h> // Biblioteca do teclado numérico
#include <LiquidCrystal.h> // Biblioteca do LCD
#include <Wire.h> // Biblioteca necessária para o funcionamento do RTC
#include "RTClib.h" // Biblioteca do relógio RTC
#include "DHT.h" // Biblioteca do sensor de temperatura DHT11

// Configurando o pino e o tipo do sensor de temperatura
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configurando LCD
LiquidCrystal lcd(13, 12, 4, 5, 6, 7);


// Configurando teclado numérico
const byte numRows= 4; //numero de linhas do teclados
const byte numCols= 4; //numero de colunas do teclado

//mapeando as definicoes das teclas pressionadas de acordo com as colunas e linhas do teclado
char keymap[4][4]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

//determinando os pinos de conexao para o arduino
byte rowPins[4] = {A4, A3, A2, A1}; //linhas de 0 a 3
byte colPins[4] = {A0, 9, 10, 11}; // colunas de 0 a 3

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows,numCols);

// Iniciando variáveis para leitura daquilo que for pressionado no teclado
int a = 0, b = 0, c = 0;
int var = 0;

// Configurando RTC
RTC_DS1307 rtc;

// Iniciando variáveis para contar o tempo e iniciar a passagem de tempo
DateTime inicio;
bool timerAtivo = false;
int tempoEmSegundos = 0;


void setup()
{
  Serial.begin(9600); // Monitor Serial
  lcd.begin(16, 2); // LCD
  dht.begin(); // Sensor de temperatura DHT11
  pinMode (2, OUTPUT); // Led verde
  pinMode (3, OUTPUT); // Buzzer

  if (!rtc.begin()) { // Relógio RTC
    Serial.println("RTC não encontrado");
    while (1);
  }

  Serial.println("Timer com RTC e monitoramento de temperatura pronto para iniciar.");
}

void loop()
{
  DateTime agora = rtc.now(); // Definindo variável com o tempo real
  char keypressed = myKeypad.getKey();
  
  if (!timerAtivo) { // Se timerAtivo for igual a 'false'
    lcd.setCursor(0, 0);
    lcd.print("Digite minutos:"); // Imprime mensagem no LCD para usuário

    if (keypressed != NO_KEY) { // Se alguma tecla for pressionada

      keypressed -= 48; // Realizando conversão das teclas
      
      var++;
      switch(var) {
        case 1: // Primeira tecla pressionada
        a = keypressed;
        lcd.setCursor(0, 1);
        lcd.print(a); // Imprime no LCD a tecla pressionada
        break;
      
        case 2: // Segunda tecla pressionada
        b = keypressed;
        if (b > 16 || b < 0) { // Se for pressionado qualquer tecla diferente de um número
          tempoEmSegundos = a * 60; // Convertendo valor para segundos
          inicio = agora; // Definindo quando o timer foi ativado
          timerAtivo = true; // Iniciando o timer
          lcd.clear(); // Limpa o que já foi escrito no LCD
        } else { // Se algum número for pressionado
          lcd.setCursor(1, 1);
          lcd.print(b); // Imprime no LCD a tecla pressionada
        }
        break;
        
        case 3: // Terceira tecla pressionada
        c = keypressed;
        if (c > 16 || c < 0) { // Se for pressionado qualquer tecla diferente de um número
          tempoEmSegundos = ((a * 10) + b) * 60; // Convertendo o primeiro valor digitado em dezenas, e o valor total para segundos
          inicio = agora; // Definindo quando o timer foi ativado
          timerAtivo = true; // Iniciando o timer
          lcd.clear(); // Limpa o que já foi escrito no LCD
        } else { // Se algum número for pressionado
          lcd.setCursor(2, 1);
          lcd.print(c); // Imprime no LCD a tecla pressionada
        }
        break;
        
        case 4: // Quarta tecla pressionada
        tempoEmSegundos = ((a * 100) + (b * 10) + c) * 60; // Convertendo o primeiro valor digitado em centenas, o segundo em dezenas, e o valor total para segundos
        inicio = agora; // Definindo quando o timer foi ativado
        timerAtivo = true; // Iniciando o timer
        lcd.clear(); // Limpa o que já foi escrito no LCD
      }
    }
    
  } else { // Se timerAtivo for igual a true (caso o timer tenha sido iniciado)
    TimeSpan tempoDecorrido = agora - inicio; // Armazenando em uma variável o valor em segundos que já se passou
    int contagemRegressiva = tempoEmSegundos - tempoDecorrido.totalseconds(); // Armazenando em uma variável o valor em segundos que ainda faltam para o fim

    if (contagemRegressiva > 0) { // Se a quantidade de segundos que faltam para o fim for maior que zero
      int minutos = contagemRegressiva / 60; // Convertendo o tempo que falta para minutos
      int segundos = contagemRegressiva % 60; // Convertendo o tempo que falta para segundos
      
      // Imprimindo no LCD e no monitor Serial a contagem regressiva para o fim do tempo escolhido
      Serial.print("Contagem regresiva: ");
      lcd.setCursor(5, 0);
      if (minutos < 10) {
        Serial.print("0");
        lcd.print("0");
      }
      Serial.print(minutos);
      lcd.print(minutos);
      Serial.print(":");
      lcd.print(":");
      if (segundos < 10) {
        Serial.print("0");
        lcd.print("0");
      }
      Serial.println(segundos);
      lcd.print(segundos);
      
      // Imprimindo no LCD e no monitor Serial a temperatura
      float temperatura = dht.readTemperature();
      if (isnan(temperatura)) {
        Serial.println("Falha ao ler o sensor de temperatura");
      } else {
        Serial.print("Temperatura: ");
        Serial.print(temperatura);
        Serial.println(" °C");
        
        lcd.setCursor(3, 1);
        lcd.print(temperatura);
        lcd.print(" *C"); // Uso de * ao invés de ° pois o LCD não consegue imprimir esse símbolo
      }


    } else { // Se a quantidade de segundos que faltam para o fim for igual ou menor que zero
      Serial.println("Fim do tempo");
      lcd.clear(); // Limpa o que já foi escrito no LCD
      lcd.setCursor(2, 0);
      lcd.print("Fim do tempo"); // Imprime mensagem no LCD para usuário
      
      digitalWrite(2, 1); // Acende led
      // Apita buzzer
      tone(3, 698, 1000);
      delay(100);
      noTone(3);
      delay(100);
      
      if (keypressed != NO_KEY) { // Se alguma tecla for pressionada
        digitalWrite(2, 0); // Deslida led
        noTone(3); // Desliga buzzer
        timerAtivo = false; // Desativa timer

        // Redefine variáveis para leitura daquilo que for pressionado no teclado
        a = 0, b = 0, c = 0;
        var = 0;
        lcd.clear();  // Limpa o que já foi escrito no LCD
      }
    }
  }     
  delay(10);
}