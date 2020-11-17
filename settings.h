#define WIFI_NAME "" // Nome (SSID) da reder Wifi 
#define WIFI_PASSWORD "" // Senha do Wifi
#define BOTtoken ""  // O Token do Telegram Bot (obter pelo Botfather)
#define THINGSSPEAK_WRITE_API_KEY ""  // O API Key para logar os eventos da cameera no THINGSSPEAK
#define ALERT_CHAT_ID  "" // Chat-Id para onde serão enviados as mensagens de alerta
#define CAMERA_NAME "Camera 1" // // Define um nome para essa Camera (Caso você quer usar mais do que uma câmera)

 // Define se o ULP (Ultra Low Power Processor) será usado (on/off)
#define ULP_ON 1 
#define ULP_OFF 0 
#define ULP ULP_ON

// define o DEBUG LEVEL 
#define DEBUG_VERBOSE 6 
#define DEBUG_WARN 5 
#define DEBUG_INFO 4 
#define DEBUG_WARN 3 
#define DEBUG_ERROR 2 
#define DEBUG_CRITICAL 1 
#define DEBUG_OFF 0 
#define DEBUG_LEVEL DEBUG_ERROR

// PIR SENSOR
#define PIR_SENSOR_GPIO GPIO_NUM_12

// Define o tempo de intervalo da leitura do sensor (em segundos) 
// (Somente quando ULP não está sendo usado)
#define PIR_READ_INTERVALL 3

// Define o tempo de intervalo (em segundos) para verificar se há um comando para o bot (em segundos) 
#define CHECK_COMMAND_INTERVALL 300

// Define o tempo de intervalo (em segundos) para mandar uma nova mensagem enquanto o PIR detecta movimentos
#define ALERT_REMINDER_INTERVALL 60

// Define a janela de tempo (em segundos) em que um novo movimento detectado pelo sensor PIR será considerado 
// um evento que está ligado a mesma alerta.   
// (Isso pode ser relevante se o detector de PIR costuma de se desligar rapidamente demais. Nesse caso
//  uma detecção com multiplos eventos, so porque não houve um movimento por 1-2)
#define ALERT_UPKEEP_INTERVALL 10

// Fator de conversão de para millisegundos para segundos 
#define mS_TO_S_FACTOR 1000

// Fator de conversão de para microsegundos para segundos 
#define uS_TO_S_FACTOR 1000000
