// Dirty code for Dr. John A. Burns' "grape detector" machine, designed for a science-art exhibit at Silent Barn 
// in Bushwick, NYC, March 23, 2013.

/*
 The grape detector machine is actually a model of some sort of RNA repair process that I am not going to describe here.
 In practice, grapes are loaded into a tube with water flowing through (courtesy of a custom pump), past a light detector, 
 and then drop out of a hole where there are two buckets spaced apart. If a grape passes the light detector, compressed air
 is supposed to knock the grape into the second bucket when it passes out the hole. Water should drop unimpeded into the hole.
 (The demo would not run long enough for bucket 1 to overflow with water.)

   [FUNNEL] <-- grapes go in here
    \   /
     | |
     |g|   light sensor             g = a grape
     | \_______|_________           1 = water bucket
     \________[ ]______  \          2 = grape bucket
            _______    | |  
           | air   |==<  
           | blower|
                      | 1 |  | 2 |
    __________________|___|__|___|

   (water pump and structure scaffolding not shown)    
    Scale: uh... the device was about 3 feet long.
           

On the night of the opening, John did not have working code for the project and still had technical issues to resolve with the actual physical system. I got there about half an hour before the exhibit was suppoesd to open.

The biggest problem was the spacing between the detector and the air blower. Since the compressed air blower had a limited capacity, but the demo needed to keep running, we wanted to minimize the waste of air used in pushing a grape into bucket 2 (and also keep 2 from taking in much water). 

 Compounding the problem:
   -- We did not know the speed of the current and had no way to manually time it. 
      (However, because of the pump, it could be counted on to be constant.)
   -- I had never programmed an Arduino before and had only about 20 minutes to write and test the code before guests started arriving.
   -- The blower needed to have a long enough pulse to get the grape over to bucket 2.

 Thus, the variables in the system:
    -- current speed (unknown)
    -- where a grape is (somewhat known, as a function of the current and awareness that a grape has passed the sensor)
    -- air blowing duration (not terribly important in terms of this documentation)
    -- time between detection and blowing (in practice, this became the most important variable)

Clearly, we needed a delay of unknown (but more or less fixed length) between the detection of the grape and the blast from the pump. However, supposing two grapes pass right after each other, within the delay timeframe. Upon detection, the first grape triggers a delay after which the pump runs. The second grape would then pass the detector and be ignored completely.

A sensible solution might have involved threading. Each detected grape could instantiate a independent, concurrent thread which would count down and then trigger the pump. The problem is, we didn't have an Arduino manual or even Internet access so there was no time to
look into how one does threading with Arduino.
 
So, I implemented a very simple queueing system using bit logic. Each digit in a 64-bit unsigned integer (a long long) represents either a grape or its absence with a 1 (grape) or 0 (no grape).  (I guess 0 looks more like a grape than a 1 but I didn't think of that.) When a grape is found it is pushed into the queue in the 1 place. After a "shift_delay" of 10 milliseconds (determined by trial and error) the whole integer is then shifted left (i.e. 0110 would become 1100). Any time a grape reaches the "detection_bit" (we used bit 62, second from the end), the air pump is reset to run for a certain number of cycles (the time in milliseconds would be "shift_delay" * "air_time").

Thus as currently shown, with air_time 20 and shift_delay 10, the blower would run for 200 ms, and the detection_bit of 62 means that the blower would activate 620 ms after the grape is detected. These specific figures were arrived at by trial and error... The important thing was that the system worked and we wound up with bucket 2 filling with grapes (and only a little bit of water) as expected.

In other words, under duress, I devised a quick and dirty way to implement a non-threaded boolean queue.

Note that the Arduino automatically runs "setup" and then repeats "loop" until interrupted.

- BBC

*/

int lightPin = A0; 
int blowThreshold = 670; // voltage detected from light detector

int grape_travel_time = 400; // milliseconds
int grapeDetector = -1; // value of read from analog pin
int blower_queue_index = 0; 
int count = 0;
int blower_timer = 0;
unsigned long long queue = 0;
int detection_bit = 62; // 0-63 the bit in the 64 bit queue that we're going to check for whether to blow; could be adjusted 
int air_time = 20; // how long the air blower blows once it starts blowing (per grape id'd)
int shift_delay = 10; // milliseconds per cycle
const int switchPin = 3;     

void insert_state(int state)
{
  queue = queue | state; // state is 1 or 0 
}

void advance_queue()
{
  queue <<= 1; // shift left one bit (discards leftmost item, rightmost item is 0)
}

int check_bit()
{
  // return false;
  return (((queue >> detection_bit) & 1));
}


void display_test(int grape)
{
  int i;
  for (i=63;i>0;i--)
  {
    if (((queue >> i) & 1) == 1)
      Serial.print("1");
    else
      Serial.print("0");    
  }
  Serial.print(" -- ");
  Serial.println(grape);
}

void setup() {
  Serial.begin(9600);
  pinMode(switchPin, OUTPUT);
}

void air_blower() {
   if (check_bit() == 1)
     blower_timer = air_time;
     digitalWrite(switchPin,LOW);
     
   if (blower_timer > 0)
   {
     digitalWrite(switchPin, HIGH);
     //Serial.println("WHOOSH!!"); 
     blower_timer -= 1;  
   }   
   else 
     if (blower_timer == 0)
     digitalWrite(switchPin,LOW);
}

void loop() {  
  grapeDetector = analogRead(lightPin);
  if (grapeDetector < blowThreshold) 
  {
    insert_state(1);    // put a 1 in the queue
  }
  air_blower();
  delay(shift_delay);
  //display_test(grapeDetector);
  advance_queue();
  count += 1;
}

