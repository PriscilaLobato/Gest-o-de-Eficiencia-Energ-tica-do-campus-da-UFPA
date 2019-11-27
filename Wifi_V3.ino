/*
 *
Projeto de Gestão de Energia e Eficincia Energética do Campus da UFPA
Coordenadora: Profa. Carminda Célia Moura de Moura Carvalho
Orientador: Prof. Me. Antonio Roniel Marques de Sousa
Frente 5: Automação
Grupo de Trabalho Frente 5 : 
                              - Denis Monteiro - Graduação
                              - Elen Priscila de Souza Lobato - Graduação
                              - Jonathan Muñoz Tabora - Mestrado

Baseado no Código: https://github.com/fabianodc/IoT/blob/master/relemqtt/relemqtt.ino
Autores deste Código: Priscila Lobato
Descrição: Código de programação para o NodeMCU ESP8266 ou ESP32 para realizar o controle de relés via WiFi
Versão:3

*/

#include <FS.h>                 //Esta precisa ser a primeira referência, ou nada dará certo e sua vida será arruinada. kkk
#include <DNSServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>        //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>
#include <EEPROM.h>


#if defined(ESP8266)
#include <ESP8266WiFi.h>  //ESP8266 Core WiFi Library         
#else
#include <WiFi.h>      //ESP32 Core WiFi Library    
#endif
 
#if defined(ESP8266)
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#else
#include <WebServer.h> //Local WebServer used to serve the configuration portal ( https://github.com/zhouhan0126/WebServer-esp32 )
#endif


#define DEBUG                   //Se descomentar esta linha vai habilitar a 'impressão' na porta serial

//Coloque os valores padrões das CloudMQTT. Tais valores poderão ser substituídos posteriormente em caso alteração.
#define servidor_mqtt             "tailor.cloudmqtt.com"        //URL do servidor da CloudMQTT 
#define servidor_mqtt_porta       "17711"                       //Porta do servidor da CloudMQTT
#define servidor_mqtt_usuario     "khkcilvq"                    //Usuário do servidor da CloudMQTT
#define servidor_mqtt_senha       "vnraLRQzakwH"                //Senha do servidor da CloudMQTT
#define mqtt_topico_sub           "ceamazon/lcade"              //Tópico para subscrever ou assinar do servidor da CloudMQTT


//Declaração do pino que serão utilizados
#define pino1                      5                   //Pino que executara a acao dado no topico "esp8266/pincmd" e terá seu status informado no tópico "esp8266/pinstatus"
#define pino2                      4
#define pino3                      0 
#define pino4                      2
#define pino5                      14
#define pino6                      12 
#define pino7                      13 
#define pino8                      15





// Memória alocada para armazenar o status deste pino na EEPROM
#define memoria_alocada            127                //Define o quanto sera alocado na EEPROM (valores entre 4 e 4096 bytes)

WiFiClient espClient;                                 //Instância do WiFiClient
PubSubClient client(espClient);                       //Passando a instância do WiFiClient para a instância do PubSubClient

//Variáveis que armazenam o status dos pinos foram gravados anteriormente na EEPROM
uint8_t statusAnt   =              0;                  
uint8_t statusAnt2   =             0;
uint8_t statusAnt3   =             0;
uint8_t statusAnt4   =             0;
uint8_t statusAnt5   =             0;
uint8_t statusAnt6   =             0;
uint8_t statusAnt7   =             0;
uint8_t statusAnt8   =             0;

//Flag para salvar os dados
bool precisaSalvar  =             false;

//Função para imprimir na porta serial da IDE do Arduino
void imprimirSerial(bool linha, String mensagem){
  #ifdef DEBUG
    if(linha){
      Serial.println(mensagem);
    }else{
      Serial.print(mensagem);
    }
  #endif
}

//Função de retorno para notificar sobre a necessidade de salvar as configurações
void precisaSalvarCallback() {
  imprimirSerial(true, "As configuracoes tem que ser salvas.");
  precisaSalvar = true;
}

//Função que reconecta ao servidor da CloudMQTT
void reconectar() {
  //Repete até conectar
  while (!client.connected()) {
    imprimirSerial(false, "Tentando conectar ao servidor da CloudMQTT...");
    
    //Tentativa de conectar. Se o MQTT precisa de autenticação, será chamada a função com autenticação, caso contrário, chama a sem autenticação. 
    bool conectado = strlen(servidor_mqtt_usuario) > 0 ?
                     client.connect("ESPCEAMAZONLCADE", servidor_mqtt_usuario, servidor_mqtt_senha) :
                     client.connect("ESPCEAMAZONLCADE");

    if(conectado) {
      imprimirSerial(true, "Conectado!");
      //Subscreve para monitorar os comandos recebidos
      client.subscribe(mqtt_topico_sub, 1); //QoS 1
    } else {
      imprimirSerial(false, "Falhou ao tentar conectar. Codigo: ");
      imprimirSerial(false, String(client.state()).c_str());
      imprimirSerial(true, " tentando novamente em 5 segundos");
      //Aguarda 5 segundos para tentar novamente
      delay(5000);
    }
  }
}

//Função que verifica qual foi o último status do pino antes do ESP ser desligado
void lerStatusAnteriorPino(){
  EEPROM.begin(memoria_alocada);  //Aloca o espaco definido na memoria
  statusAnt = EEPROM.read(0);     //Lê o valor armazenado na EEPROM e passa para a variável "statusAnt"
  statusAnt2 = EEPROM.read(1); 
  statusAnt3 = EEPROM.read(2); 
  statusAnt4 = EEPROM.read(3);
  statusAnt5 = EEPROM.read(4);
  statusAnt6 = EEPROM.read(5);
  statusAnt7 = EEPROM.read(6); 
  statusAnt8 = EEPROM.read(7);
  
  if(statusAnt > 1){
    statusAnt = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(0,statusAnt);
  }
  digitalWrite(pino1, statusAnt);
  EEPROM.end();

 if(statusAnt2 > 1){
    statusAnt2 = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(1,statusAnt2);
  }
  digitalWrite(pino2, statusAnt2);
  EEPROM.end();


 if(statusAnt3 > 1){
    statusAnt3 = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(2,statusAnt3);
  }
  digitalWrite(pino3, statusAnt3);
  EEPROM.end();


 if(statusAnt4 > 1){
    statusAnt4 = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(3,statusAnt4);
  }
  digitalWrite(pino4, statusAnt4);
  EEPROM.end();

 if(statusAnt5 > 1){
    statusAnt5= 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(4,statusAnt5);
  }
  digitalWrite(pino5, statusAnt5);
  EEPROM.end();


 if(statusAnt6 > 1){
    statusAnt6 = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(5,statusAnt);
  }
  digitalWrite(pino6, statusAnt6);
  EEPROM.end();


 if(statusAnt7 > 1){
    statusAnt7 = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(6,statusAnt7);
 }
  digitalWrite(pino7, statusAnt7);
  EEPROM.end();

 if(statusAnt8 > 1){
    statusAnt8 = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(7,statusAnt8);
  }
  digitalWrite(pino8, statusAnt8);
  EEPROM.end();



}



//Função que grava status do pino na EEPROM
void gravarStatusPino(uint8_t statusPino1){
  EEPROM.begin(memoria_alocada);
  EEPROM.write(0, statusPino1);
   EEPROM.end();
}


void gravarStatusPino2(uint8_t statusPino2){
 EEPROM.begin(memoria_alocada);
  EEPROM.write(1, statusPino2);
   EEPROM.end();
}

void gravarStatusPino3(uint8_t statusPino3){
 EEPROM.begin(memoria_alocada);
  EEPROM.write(2, statusPino3);
   EEPROM.end();
}

void gravarStatusPino4(uint8_t statusPino4){
 EEPROM.begin(memoria_alocada);
  EEPROM.write(3, statusPino4);
   EEPROM.end();
}

void gravarStatusPino5(uint8_t statusPino5){
 EEPROM.begin(memoria_alocada);
  EEPROM.write(4, statusPino5);
   EEPROM.end();
}

void gravarStatusPino6(uint8_t statusPino6){
 EEPROM.begin(memoria_alocada);
  EEPROM.write(5, statusPino6);
   EEPROM.end();
}
void gravarStatusPino7(uint8_t statusPino7){
 EEPROM.begin(memoria_alocada);
  EEPROM.write(6, statusPino7);
   EEPROM.end();
}

void gravarStatusPino8(uint8_t statusPino8){
 EEPROM.begin(memoria_alocada);
  EEPROM.write(7, statusPino8);
   EEPROM.end();
}

//Função que será chamada ao receber mensagem do servidor da CloudMQTT
void retorno(char* topico, byte* mensagem, unsigned int tamanho) {
  
  //Convertendo a mensagem recebida para string
  mensagem[tamanho] = '\0';
  String strMensagem = String((char*)mensagem);
  strMensagem.toLowerCase();
  //float f = s.toFloat();
  
  imprimirSerial(false, "Mensagem recebida! Topico: ");
  imprimirSerial(false, topico);
  imprimirSerial(false, ". Tamanho: ");
  imprimirSerial(false, String(tamanho).c_str());
  imprimirSerial(false, ". Mensagem: ");
  imprimirSerial(true, strMensagem);


  if(strMensagem == "liga1"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
    digitalWrite(pino1, HIGH);            
    client.publish(mqtt_topico_sub, "ligado1");
    gravarStatusPino(HIGH);
  
  
  }
  
  else if(strMensagem == "desliga1"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino1, LOW);
   client.publish(mqtt_topico_sub, "desligado1");
    gravarStatusPino(LOW);
  }

  if(strMensagem == "liga2"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
     digitalWrite(pino2, HIGH);
   client.publish(mqtt_topico_sub, "ligado2");
    gravarStatusPino2(HIGH);
  }
  
  else if(strMensagem == "desliga2"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino2, LOW);
    client.publish(mqtt_topico_sub, "desligado2");
    gravarStatusPino2(LOW);
  }
  
  
   if(strMensagem == "liga3"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
     digitalWrite(pino3, HIGH);            
    client.publish(mqtt_topico_sub, "ligado3");
    gravarStatusPino3(HIGH);
  }
  
  else if(strMensagem == "desliga3"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino3, LOW); 
   client.publish(mqtt_topico_sub, "desligado3");
    gravarStatusPino3(LOW);
  }
  
   if(strMensagem == "liga4"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
     digitalWrite(pino4, HIGH);
    client.publish(mqtt_topico_sub, "ligado4");
    gravarStatusPino4(HIGH);
  }
  
  else if(strMensagem == "desliga4"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino4, LOW);
    client.publish(mqtt_topico_sub, "desligado4");
    gravarStatusPino4(LOW);
  }
  
  
  if(strMensagem == "liga5"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
     digitalWrite(pino5, HIGH);            
    client.publish(mqtt_topico_sub, "ligado5");
    gravarStatusPino5(HIGH);
  }
  
  
  else if(strMensagem == "desliga5"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino5, LOW); 
   client.publish(mqtt_topico_sub, "desligado5");
    gravarStatusPino5(LOW);
  }
  
  
if(strMensagem == "liga6"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
     digitalWrite(pino6, HIGH);
   client.publish(mqtt_topico_sub, "ligado6");
    gravarStatusPino6(HIGH);
  }
  
  else if(strMensagem == "desliga6"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino6, LOW);
    client.publish(mqtt_topico_sub, "desligado6");
    gravarStatusPino6(LOW);
  }
  
 
if(strMensagem == "liga7"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
     digitalWrite(pino7, HIGH);            
    client.publish(mqtt_topico_sub, "ligado7");
    gravarStatusPino7(HIGH);
  }
  
  else if(strMensagem == "desliga7"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino7, LOW); 
   client.publish(mqtt_topico_sub, "desligado7");
    gravarStatusPino7(LOW);
  }
  
 
if(strMensagem == "liga8"){
    imprimirSerial(true, "Colocando o pino em stado ALTO...");
  
     digitalWrite(pino8, HIGH);            
   client.publish(mqtt_topico_sub, "ligado8");
    gravarStatusPino8(HIGH);
  }else if(strMensagem == "desliga8"){
    imprimirSerial(true, "Colocando o pino em stado BAIXO...");
   
             
     digitalWrite(pino8, LOW); 
   client.publish(mqtt_topico_sub, "desligado8");
    gravarStatusPino8(LOW);
  }
  
    
  
// Imprimi no monitor o status de todos os pinos após processar o comando
  
  imprimirSerial(false, "Status do pino depois de processar o comando: ");
  imprimirSerial(true, String(digitalRead(pino1)).c_str());
  imprimirSerial(true, String(digitalRead(pino2)).c_str());
  imprimirSerial(true, String(digitalRead(pino3)).c_str());
  imprimirSerial(true, String(digitalRead(pino4)).c_str());
  imprimirSerial(true, String(digitalRead(pino5)).c_str());
  imprimirSerial(true, String(digitalRead(pino6)).c_str());
  imprimirSerial(true, String(digitalRead(pino7)).c_str());
  imprimirSerial(true, String(digitalRead(pino8)).c_str());
  
}




//Função inicial (será executado SOMENTE quando ligar o ESP)
void setup() {
  
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  imprimirSerial(true, "...");

//Fazendo os pinos serem saída, pois eles irão "controlar" algo. E colocando todos os pinos em estado lógico baixo "LOW"
  pinMode(pino1, OUTPUT);
  digitalWrite(pino1, LOW);    
   
 pinMode(pino2, OUTPUT);
  digitalWrite(pino2, LOW);    
   
  pinMode(pino3, OUTPUT);
  digitalWrite(pino3, LOW);      
   
  pinMode(pino4, OUTPUT);
  digitalWrite(pino4, LOW);    
   
  pinMode(pino5, OUTPUT);
  digitalWrite(pino5, LOW);    
   
 pinMode(pino6, OUTPUT);
  digitalWrite(pino6, LOW);    
   
  pinMode(pino7, OUTPUT);
  digitalWrite(pino7, LOW);      
   
  pinMode(pino8, OUTPUT);
  digitalWrite(pino8, LOW);    
  
  
  
  //Formatando a memória interna
  //Descomente a linha abaixo enquanto estiver testando e comente ou apague quando estiver pronto)
  //SPIFFS.format();

  //Iniciando o SPIFSS (SPI Flash File System)
  imprimirSerial(true, "Iniciando o SPIFSS (SPI Flash File System)");
  if (SPIFFS.begin()) {
    imprimirSerial(true, "Sistema de arquivos SPIFSS montado!");
    if (SPIFFS.exists("/config.json")) {
      //Arquivo de configuração existe e será lido.
      imprimirSerial(true, "Abrindo o arquivo de configuracao...");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        imprimirSerial(true, "Arquivo de configuracao aberto.");
        size_t size = configFile.size();
        
        //Alocando um buffer para armazenar o conteúdo do arquivo.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
            //Copiando as variáveis salvas previamente no aquivo json para a memória do ESP.
            imprimirSerial(true, "arquivo json analisado.");
            strcpy(servidor_mqtt, json["servidor_mqtt"]);
            strcpy(servidor_mqtt_porta, json["servidor_mqtt_porta"]);
            strcpy(servidor_mqtt_usuario, json["servidor_mqtt_usuario"]);
            strcpy(servidor_mqtt_senha, json["servidor_mqtt_senha"]);
            strcpy(mqtt_topico_sub, json["mqtt_topico_sub"]);

        } else {
          imprimirSerial(true, "Falha ao ler as configuracoes do arquivo json.");
        }
      }
    }
  } else {
    imprimirSerial(true, "Falha ao montar o sistema de arquivos SPIFSS.");
  }
  //Fim da leitura do sistema de arquivos SPIFSS

  //Parâmetros extras para configuração
  //Depois de conectar, parameter.getValue() vai pegar o valor configurado.
  //Os campos do WiFiManagerParameter são: id do parâmetro, nome, valor padrão, comprimento
  WiFiManagerParameter custom_mqtt_server("server", "Servidor MQTT", servidor_mqtt, 40);
  WiFiManagerParameter custom_mqtt_port("port", "Porta", servidor_mqtt_porta, 6);
  WiFiManagerParameter custom_mqtt_user("user", "Usuario", servidor_mqtt_usuario, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "Senha", servidor_mqtt_senha, 20);
  WiFiManagerParameter custom_mqtt_topic_sub("topic_sub", "Topico para subscrever", mqtt_topico_sub, 30);

  //Inicialização do WiFiManager. Uma vez iniciado não é necessário mantê-lo em memória.
  WiFiManager wifiManager;

  //Definindo a função que informará a necessidade de salvar as configurações
  wifiManager.setSaveConfigCallback(precisaSalvarCallback);
  
  //Adicionando os parâmetros para conectar ao servidor MQTT
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_mqtt_topic_sub);

  //Busca o ID e senha da rede wifi e tenta conectar.
  //Caso não consiga conectar ou não exista ID e senha,
  //cria um access point com o nome "AutoConnectAP" e a senha "senha123"
  //E entra em loop aguardando a configuração de uma rede WiFi válida.
  if (!wifiManager.autoConnect("AutoConnectAP", "senha123")) {
    imprimirSerial(true, "Falha ao conectar. Excedeu o tempo limite para conexao.");
    delay(3000);
    //Reinicia o ESP e tenta novamente ou entra em sono profundo (DeepSleep)
    ESP.reset();
    delay(5000);
  }

  //Se chegou até aqui é porque conectou na WiFi!
  imprimirSerial(true, "Conectado!! :)");

  //Lendo os parâmetros atualizados
  strcpy(servidor_mqtt, custom_mqtt_server.getValue());
  strcpy(servidor_mqtt_porta, custom_mqtt_port.getValue());
  strcpy(servidor_mqtt_usuario, custom_mqtt_user.getValue());
  strcpy(servidor_mqtt_senha, custom_mqtt_pass.getValue());
  strcpy(mqtt_topico_sub, custom_mqtt_topic_sub.getValue());

  //Salvando os parâmetros informados na tela web do WiFiManager
  if (precisaSalvar) {
    imprimirSerial(true, "Salvando as configuracoes");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["servidor_mqtt"] = servidor_mqtt;
    json["servidor_mqtt_porta"] = servidor_mqtt_porta;
    json["servidor_mqtt_usuario"] = servidor_mqtt_usuario;
    json["servidor_mqtt_senha"] = servidor_mqtt_senha;
    json["mqtt_topico_sub"] = mqtt_topico_sub;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      imprimirSerial(true, "Houve uma falha ao abrir o arquivo de configuracao para incluir/alterar as configuracoes.");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

  imprimirSerial(false, "IP: ");
  imprimirSerial(true, WiFi.localIP().toString());

  //Informando ao client do PubSub a url do servidor e a porta.
  int portaInt = atoi(servidor_mqtt_porta);
  client.setServer(servidor_mqtt, portaInt);
  client.setCallback(retorno);
  
  //Obtendo o status do pino antes do ESP ser desligado
  lerStatusAnteriorPino();
}

//Função de repetição (será executado INFINITAMENTE até o ESP ser desligado)
void loop() {
  if (!client.connected()) {
    reconectar();
  }
  client.loop();
}
