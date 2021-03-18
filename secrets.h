/* Questo file contiene le credenziali utilizzate per accedere al servizio adafruit e 
 * per gestire il bot telegram (oltre al pin della sim, che per semplicità è consigliato disattivare)
*/

// Vedere https://github.com/fablab-imperia/PesaturaArnie#Adafruit-MQTT-broker
#define ADAFRUIT_USERNAME  "my_username"                        // Credenziali broker MQTT Adafruit
#define AIO_KEY "0123456789abcdef0123456789abcdef"


// Vedere https://github.com/fablab-imperia/PesaturaArnie#Telegram
#define CHAT_ID 123456789                                       // Numero identificativo univoco delle chat/gruppi telegram
#define TOKEN  "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11"      // Token del bot Telegram utilizzato


// Vedere https://github.com/fablab-imperia/PesaturaArnie#SIM
// Lasciare vuote e disattivare il blocco PIN inserendo la SIM in un telefono cellulare (per evitare problemi con lo sblocco)
const char user[] = "";
const char pass[] = "";
const char pin_card[] = "";









// The check below is just to ensure the user has changed the default values above and avoid time conuming debugging
// If for an extremely rare coincidence your chat_id is actually 123456789 just comment out the following 3 lines ;-)
#if (CHAT_ID == 123456789)
  #error "Some values in secrets.h are set to default value; read the documentation at https://github.com/fablab-imperia/PesaturaArnie#Adafruit-MQTT-broker and change it";
#endif
