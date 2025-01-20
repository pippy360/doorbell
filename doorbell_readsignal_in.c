volatile unsigned long pulseCount = 0;
volatile bool countingActive = false;
volatile unsigned long lastChangeTime = 0;
const int PULSE_PIN = A3;  // Input A
const unsigned long START_THRESHOLD = 350;  // Time in ms to detect start signal
const unsigned long END_THRESHOLD = 210;    // Time in ms to detect end signal

void setup() {
  Serial.begin(9600);
  
  // Configure A3 as input with pullup
  pinMode(PULSE_PIN, INPUT_PULLUP);
  
  // Enable pin change interrupt for A3 (PCINT11)
  PCICR |= (1 << PCIE1);    // Enable PCINT1 interrupt
  PCMSK1 |= (1 << PCINT11); // Enable mask for PCINT11
  
  Serial.println("Pulse counter started on Input A (A3)");
}

bool waitingOnInit = true;
bool countingPulses = false;
bool coolDownAfterPulses = false;

void loop() {

  // Check if we've had a long HIGH period indicating end of sequence
  if (countingActive) {
    Serial.println("The counting has been triggered ");    
  }

  if (countingActive && (millis() - lastChangeTime > END_THRESHOLD)) {
    noInterrupts();
    unsigned long finalCount = pulseCount;
    countingActive = false;
    pulseCount = 0;
    interrupts();
    
    Serial.print("Sequence complete! Number of pulses: ");
    Serial.println(finalCount);
  }
}

// Pin Change Interrupt handler for PCINT8..14
ISR(PCINT1_vect) {
  unsigned long currentTime = millis();
  bool pinState = digitalRead(PULSE_PIN);
  
  if (!countingActive) {
    // Look for start condition (long LOW)
    if (!pinState) {
      lastChangeTime = currentTime;
    } else if (currentTime - lastChangeTime >= START_THRESHOLD) {
      countingActive = true;
      pulseCount = 0;
    }
  } else {
    // Count falling edges (HIGH to LOW transitions)
    if (!pinState) {
      pulseCount++;
    }
  }
  
  lastChangeTime = currentTime;
}
