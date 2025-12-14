#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ONLY these pins are needed for DFPlayer
#define DFPLAYER_RX_PIN 16  // ESP32 TX → DFPlayer RX
#define DFPLAYER_TX_PIN 17  // ESP32 RX → DFPlayer TX

SoftwareSerial dfplayerSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini myDFPlayer;

void checkSDCard() {
  Serial.println("\nChecking SD card in DFPlayer...");
  
  int fileCount = myDFPlayer.readFileCounts();
  
  if (fileCount > 0) {
    Serial.print("SD card found with ");
    Serial.print(fileCount);
    Serial.println(" audio files");
    
    // List first few files
    Serial.println("\nAvailable commands:");
    Serial.println("p = Play/Pause");
    Serial.println("s = Stop");
    Serial.println("+ = Volume up");
    Serial.println("- = Volume down");
    Serial.println("1 = Play track 1");
    Serial.println("2 = Play track 2");
    Serial.println("... etc");
  } else {
    Serial.println("No SD card or no files found!");
    Serial.println("Format SD card as FAT32");
    Serial.println("Name files: 001.mp3, 002.mp3, etc.");
  }
}

void setup() {
  Serial.begin(115200);
  dfplayerSerial.begin(9600);
  
  Serial.println("Initializing DFPlayer Mini...");
  Serial.println("(SD card is inside DFPlayer, not connected to ESP32)");
  
  delay(2000); // Give DFPlayer time to initialize
  
  if (!myDFPlayer.begin(dfplayerSerial)) {
    Serial.println("Unable to begin DFPlayer!");
    Serial.println("1. Check connections: RX->16, TX->17");
    Serial.println("2. Check power (add capacitors!)");
    Serial.println("3. Check SD card format/contents");
    while(true);
  }
  
  Serial.println("DFPlayer Mini online!");
  
  // Set initial settings
  myDFPlayer.volume(20);  // 0-30
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  
  // Check if SD card is readable
  checkSDCard();
}

void loop() {
  // Simple command handler via Serial Monitor
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case 'p':
        myDFPlayer.start();
        Serial.println("Play/Pause");
        break;
      case 's':
        myDFPlayer.stop();
        Serial.println("Stop");
        break;
      case '+':
        myDFPlayer.volumeUp();
        Serial.println("Volume +");
        break;
      case '-':
        myDFPlayer.volumeDown();
        Serial.println("Volume -");
        break;
      case '1':
        myDFPlayer.play(1);
        Serial.println("Playing track 1");
        break;
      case '2':
        myDFPlayer.play(2);
        Serial.println("Playing track 2");
        break;
      case '3':
        myDFPlayer.play(3);
        Serial.println("Playing track 3");
        break;
      case '?':
        Serial.print("Total tracks: ");
        Serial.println(myDFPlayer.readFileCounts());
        Serial.print("Current track: ");
        Serial.println(myDFPlayer.readCurrentFileNumber());
        break;
    }
  }
  
  // Optional: Auto-play demo
  static unsigned long lastPlay = 0;
  if (millis() - lastPlay > 10000) { // Every 10 seconds
    lastPlay = millis();
    myDFPlayer.next();
    Serial.println("Auto-play: Next track");
  }
}