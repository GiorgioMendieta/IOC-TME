// --------------------------------------------------------------------------------------------------------------------
// Multi-tâches cooperatives : solution basique mais efficace :-)
// --------------------------------------------------------------------------------------------------------------------

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



// --------------------------------------------------------------------------------------------------------------------
// unsigned int waitFor(timer, period) 
// Timer pour taches périodiques 
// configuration :
//  - MAX_WAIT_FOR_TIMER : nombre maximum de timers utilisés
// arguments :
//  - timer  : numéro de timer entre 0 et MAX_WAIT_FOR_TIMER-1
//  - period : période souhaitée
// retour :
//  - nombre de périodes écoulées depuis le dernier appel
// --------------------------------------------------------------------------------------------------------------------
#define MAX_WAIT_FOR_TIMER 5
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches périodiques
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta = 1 + newTime;                   // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}

//--------- Boîte aux lettres

enum {EMPTY, FULL};

struct mailbox_s {
  int state;
  int val;
};

struct mailbox_s mb = {.state = EMPTY};
struct mailbox_s mb2 = {.state = EMPTY};


//--------- définition de la tache Lum

struct Lum_s {
  int timer;
  unsigned long period;
  int val;
  int pin;
};

void setup_Lum(struct Lum_s * ctx, int timer, unsigned long period){
  ctx->timer = timer;
  ctx->period = period;
  ctx->val = 0;
  ctx->pin = 36;
}

void loop_Lum(struct Lum_s * ctx) {
  if (!waitFor(ctx->timer, ctx->period)) return; 
  if (mb.state != EMPTY) return; // attend que la mailbox soit vide
  ctx->val = map(analogRead(ctx->pin), 4095, 0, 0, 100);
  mb.val = ctx->val;
  mb.state = FULL;
  mb2.val = ctx->val;
  mb2.state = FULL;
}

//--------- définition de la tache Led

struct Led_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;                                               // etat interne de la led
}; 

void setup_Led( struct Led_s * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void loop_Led( struct Led_s * ctx) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  if(mb2.state == FULL){
    ctx->period = map(mb2.val, 0, 100, 200000, 2000000);
    mb2.state = EMPTY;
  }
  digitalWrite(ctx->pin,ctx->etat);                       // ecriture
  ctx->etat = 1 - ctx->etat;                              // changement d'état
}

//--------- definition de la tache Mess

struct Mess_s {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                             // periode d'affichage
  char mess[20];
} ; 

void setup_Mess( struct Mess_s * ctx, int timer, unsigned long period, const char * mess) {
  ctx->timer = timer;
  ctx->period = period;
  strcpy(ctx->mess, mess);
  Serial.begin(9600);                                     // initialisation du débit de la liaison série
}

void loop_Mess(struct Mess_s *ctx) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
  Serial.println(ctx->mess);                              // affichage du message
}

//--------- definition de la tache Compteur

struct Oled_s {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned int cpt;                                       // compteur
  unsigned long period;                                   // période d'incrémentation du compteur
//  struct cursor_s{
//    int x;
//    int y;
//  } cursor;
  int val;
};

void setup_Oled(struct Oled_s *Oled, int timer, unsigned long period){
  
  Wire.begin(4, 15); // pins SDA , SCL
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64, pour notre ESP32 l'adresse est 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Oled->timer = timer;
  Oled->period = period;
  Oled->cpt = 0;
  Oled->val = 0;

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  //display.display();
  //delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  //display.setCursor(Oled->cursor.x, Oled->cursor.y);
  //display.println(itoa(Oled->Oled));
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(Oled->cpt, DEC);
  display.print(Oled->val, DEC);
  display.println("%");
  display.display();
}

void loop_Oled( struct Oled_s * ctx) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  ctx->cpt++;
  display.clearDisplay();
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(ctx->cpt, DEC);
  if(mb.state == FULL){
    ctx->val = mb.val;
    mb.state = EMPTY;
  }
  //display.setCursor(0,1);
  display.print(ctx->val, DEC);
  display.println("%");
  display.display();
}

//--------- Déclaration des tâches

struct Led_s Led1;
struct Mess_s Mess1;
struct Mess_s Mess2;
struct Oled_s Oled;
struct Lum_s Lum;


//--------- Setup et Loop

void setup() {
  setup_Led(&Led1, 0, 100000, LED_BUILTIN);                        // Led est exécutée toutes les 100ms 
  setup_Mess(&Mess1, 1, 1000000, "bonjour");              // Mess est exécutée toutes les secondes 
  setup_Mess(&Mess2, 2, 2000000, "salut");
  setup_Oled(&Oled, 3, 1000000);
  setup_Lum(&Lum, 4, 500000);
}

void loop() {
  loop_Led(&Led1);                                        
  loop_Mess(&Mess1);
  loop_Mess(&Mess2);
  loop_Oled(&Oled);
  loop_Lum(&Lum);
}
