const int BUTTON = 5;

void setup(){
    Serial.begin(115200); //initialize serial!
    //Set up an Active Low Switch!
    pinMode(BUTTON,INPUT_PULLUP); //sets IO pin 5 as an input which defaults to a 3.3V signal when unconnected and 0V when the switch is pushed
}

void loop(){
    int value = digitalRead(BUTTON);
    if (value){
        //executed if pin is HIGH (voltage of 3.3V)
       //code here for printing "unpushed" 
       Serial.println("unpushed");
    }else{ 
        //executed if pin is LOW (voltage of 0V)
       //code here to print "pushed" 
       Serial.println("pushed");
    }
    //if your code crashes in serial monitor:
    //uncomment line below:
    delay(3);
}
