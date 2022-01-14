const int AUDIO_IN = A0;
uint16_t value_in[1000] = {0};
uint8_t value_out[1000] = {0};
int sum;

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i=0;i<1000;i++) value_in[i] = 16*analogRead(AUDIO_IN);
    
  // Run mulaw_encode 1000 times
  long start = micros();    // Start (time is an elapsedMicros counter...make sure you know what that is)
  for (int i=0; i<1000; i++) {
    value_out[i] = mulaw_encode(value_in[i]);    
  }
  long duration = micros() - start; // Stop
   
  Serial.printf("processing time: %d\r\n", duration);
   
  // Print out sum, so compiler doesn't optimize the benchmark
  for (int i=0; i<1000; i++) {
    sum += value_out[i]; 
  }
  Serial.println(sum);
   
  delay(1000);
}
int8_t mulaw_encode(int16_t sample) {
   const uint16_t MULAW_MAX = 0x1FFF;
   const uint16_t MULAW_BIAS = 33;
   uint16_t mask = 0x1000;
   uint8_t sign = 0;
   uint8_t position = 12;
   uint8_t lsb = 0;
   if (sample < 0)
   {
      sample = -sample;
      sign = 0x80;
   }
   sample += MULAW_BIAS;
   if (sample > MULAW_MAX)
   {
      sample = MULAW_MAX;
   }
   for (; ((sample & mask) != mask && position >= 5); mask >>= 1, position--)
        ;
   lsb = (sample >> (position - 4)) & 0x0f;
   return (~(sign | ((position - 5) << 4) | lsb));
}
