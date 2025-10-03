int pirPin = 2;     // PIR OUT pin
int ledPin = 13;    // Built-in LED

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("PIR sensor warming up...");
}

void loop() {
  int val = digitalRead(pirPin);
  
  if (val == HIGH) {
    digitalWrite(ledPin, LOW);
    Serial.println("Motion detected!");
    delay(20000);               // Keep ON for 20 seconds
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, HIGH);
    Serial.println("No motion");
  }
  
  delay(500); // small delay for readability
}
