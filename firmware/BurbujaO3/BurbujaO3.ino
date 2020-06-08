//definiciones

//pantalla lcd

#include <Wire.h> // librerias
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // crear objeto de pantalla lcd (pines i2c, columnas, filas)

int cursors = 0; // indice del cursor 
unsigned long lastt = 0; // 
unsigned long sr = 100;

//RELe

#define pinrele 12
int staterele = 0;

//sensores

#define ozint A1 // sensor al interior de la burbuja
#define ozext A0 // sensor externo

int valozint = 0;
int valozext = 0;
int targetoz = 0;
int valordealarma = 1000; // sobre este valor el programa se detiene y suena la alarma, hay que resetear.
int maxo3 = 15; // valor maximo para setear la concentracion en la burbuja.

//tiempo

unsigned long target = 0;
unsigned long start = 0;
int maxt = 30; //tiempo maximo en minutos para setear el sistema.

//led rgb

#define rgbr 9
#define rgbg 10
#define rgbb 11

int r = 0;
int g = 0;
int b = 0;

//encoder

#define encA 7
#define encB 6
#define encC 5

int boton = 1;
int lastboton = 0;

int counter = 0; 
int aState;
int aLastState;  
int cont = 0;

boolean intro = false;

//buzzer

#define buzz 4

//musiquitas

#include "pitches.h"

int melody[] = {
  NOTE_C5, NOTE_C5
};

int noteDurations[] = {
  4, 8
};

void hello(){ // sonidos de encendido
    for (int thisNote = 0; thisNote < 2; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(buzz, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(buzz);
  }
}

//estados

int state = 0;
unsigned long starttime = 0;
unsigned long militarget = 0; // tiempo fijado en milisegundos
unsigned long elapsed = 0; // tiempo transcurrido en milisegundos
unsigned long temporizador = 0; //cuentatras
unsigned long minutos = 0;
unsigned long segundos = 0;

void states(){
  
switch (state) {
  
  case 0:
    g=1;
    if(target>0 && intro==false){
    r=1;
    g=0;
    b=0;
    state=1;
    staterele=1;
    tone(buzz, NOTE_C5, 500);
    delay(600);
    noTone(buzz);
    }
    break;
    
  case 1:
    if(targetoz<=valozint){
    r=0;
    g=0;
    b=1;
    state=2;
    starttime=millis();
    militarget=target*1000*60;
    tone(buzz, NOTE_C5, 500);
    delay(600);
    noTone(buzz);
    }
    break;
    
  case 2:
    elapsed=millis()-starttime;
    temporizador=militarget-elapsed;
    if(temporizador>200000000){
    r=0;
    g=1;
    b=0;
    state=0;
    target=0;
    staterele=0;
    lcd.setCursor(8,1);
    lcd.print("    ");
    tone(buzz, NOTE_C5, 500);
    delay(600);
    noTone(buzz);
    }
    break;
  default:

    break;
}

}

void setup() {
  
    Serial.begin(9600);
    states();
    setuppin();
    setuplcd();
    aLastState = digitalRead(encA);   
    lastt=millis();
    
}

void loop() {

  states();
  menu();
  warning();
  encoder();
  bip();
  led();
  digitalWrite(pinrele,staterele);
  if(millis()-lastt>=sr){
    updatesensors();
    updateled();
   lastt=millis();
  }


}

void updatesensors(){ // lectura de sensores, aca hay que poner las ecuaciones 

  valozint = analogRead(ozint);
  valozint = valozint/10;
  valozext = analogRead(ozext);

}

void updateled(){ // actualizar pantalla

  lcd.noCursor();
  lcd.setCursor(7,0);
  lcd.print(targetoz);
  if(targetoz<10){
    lcd.setCursor(8,0);
    lcd.print(" ");
  }
  lcd.setCursor(7,1);
  lcd.print(target);
  if(target<10){
    lcd.setCursor(8,1);
    lcd.print(" ");
  }
  if(state==2){
    segundos=temporizador/1000;
    minutos=segundos/60;
    segundos=segundos%60;
    lcd.setCursor(7,1);
    if(minutos<10){
      lcd.print("0");
    }
    lcd.print(minutos);
    lcd.print(":");
    if (segundos<10){
      lcd.print("0");
    }
    lcd.print(segundos);
  }
  
  lcd.setCursor(14 ,0);
  lcd.print(valozint);
  if(valozint<10){
    lcd.setCursor(15,0);
    lcd.print(" ");
  }
  lcd.setCursor(7 , cursors);
  lcd.cursor();

}

void setuppin(){
  
  pinMode(buzz,OUTPUT);
  pinMode(rgbr,OUTPUT);
  pinMode(rgbg,OUTPUT);
  pinMode(rgbb,OUTPUT);
  pinMode(ozint,INPUT);
  pinMode(ozext,INPUT);
  pinMode(encA,INPUT);
  pinMode(encB,INPUT);
  pinMode(encC,INPUT); 
  pinMode(pinrele,OUTPUT);
   
}

void setuplcd(){
  
  lcd.init();
  lcd.backlight();
  lcd.print("Burbuja O3");
  lcd.setCursor(0,1);
  lcd.print("Version 1.0");
  delay(2000);
  hello();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set O3 ");
  lcd.print(targetoz);
  lcd.setCursor(10, 0);
  lcd.print("ppm ");
  lcd.print(valozint);
  lcd.setCursor(0, 1);
  lcd.print("Tiempo ");
  lcd.print(target);
  delay(100);
  
}

void led(){ // activar leds
  
  digitalWrite(rgbr,r);
  digitalWrite(rgbg,g);
  digitalWrite(rgbb,b);

}

void bip(){ // presionar la perilla

  boton=digitalRead(encC);
  if(boton==0 && lastboton==1){
    tone(buzz,NOTE_C4,4);
    delay(4*1.3);
    noTone(buzz);
    intro=!intro;
    if(intro==true){
    if(cursors==0){
    counter=targetoz;
    }
    else{
    counter=target;
    }
    }
  }
  lastboton=boton;
}

void encoder(){       // lectura del giro del encoder
  
  aState = digitalRead(encA); 

       if (aState != aLastState){  

         cont=cont+1;       
         if(cont==2){
           
           if(intro==false){
           cursors=!cursors;
           }
           cont=0;  
         if (digitalRead(encB) != aState) { 
           counter --;
         } else {
           counter ++;
         }

       } 
       aLastState = aState; 
       }
         
}

void menu(){ // navegar por el menu
  
    if(intro==true){
      lcd.blink();
    if(cursors==0){
      targetoz=counter;
      targetoz=constrain(targetoz,0,maxo3);
      counter=constrain(counter,0,maxo3);
    }
    if(cursors==1){
      target=counter;
      target=constrain(target,0,maxt);
      counter=constrain(counter,0,maxt);
    }
  }
  if(intro==false){
    lcd.noBlink();
  }
  
}

void warning(){ // alarma para ozono exterior muy alto
  if(valozext>=valordealarma){ 
    while(true){  
    lcd.clear(); // parpadeo de pantalla
    tone(buzz,NOTE_C5,4); // sonido
    delay(4*1.3);
    noTone(buzz);
    digitalWrite(rgbr,1); // luz roja
    delay(100);
    lcd.print("ALARMA O3");
    digitalWrite(rgbr,0);
    delay(100);
    }
  }
}
