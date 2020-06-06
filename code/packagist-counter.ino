// This #include statement was automatically added by the Particle IDE.
#include <LedControl-MAX7219-MAX7221.h>

#include <math.h>

LedControl lc=LedControl(D6,D4,D5,1);

int count = 0;
int lastCount = 0;
int downloadsPerInterval = 1;
int updateIntervalInMinutes = 10;
int knownDownloadRatePerMinute = 15;
bool updating = false;

void setup() {
    
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* and clear the display */
  lc.clearDisplay(0);
  
  Particle.subscribe("hook-response/commonmark-count", gotPackagistCount, MY_DEVICES);
  
  lc.setChar(0, 0, '-', true);
  lc.setChar(0, 1, '-', false);
  lc.setChar(0, 2, '-', false);
  lc.setChar(0, 3, '-', false);
  lc.setChar(0, 4, '-', false);
  lc.setChar(0, 5, '-', false);
  lc.setChar(0, 6, '-', false);
  lc.setChar(0, 7, '-', false);
  
  downloadsPerInterval = knownDownloadRatePerMinute * updateIntervalInMinutes;
  
  updating = true;
  Particle.publish("commonmark-count");
}

void loop() {
    while (updating) {
        delay(250);
    }
    
    // Divide the current interval into the update frequency
    // Divide the update interval into 100 units, updating the count at each tick
    int incrementCounterEvery = (1000 * 60 * updateIntervalInMinutes) / max(1, downloadsPerInterval);
    Spark.publish("incrementCounterEvery", String(incrementCounterEvery) + " ms");
    for (int i = 0; i < downloadsPerInterval; i++) {
        delay(incrementCounterEvery);
        count++;
        displayCount();
    }
    
    // Set the last decimal point to indicate that we're fetching an update
    // To do this, we need to set the last digit as well
    lc.setDigit(0, 0, count % 10, true);
    
    updating = true;
    Particle.publish("commonmark-count");
}

void gotPackagistCount(const char *name, const char *data) {
    lastCount = count;
    count = strtol(data, NULL, 10);
    
    Spark.publish("lastCount", String(lastCount));
    
    if (lastCount <= 1 || lastCount == count ) {
        lastCount = count - (knownDownloadRatePerMinute * updateIntervalInMinutes);
        Spark.publish("fakingLastCount", String(count) + " - (" + String(knownDownloadRatePerMinute) + " * " + String(updateIntervalInMinutes) + ") = " + String(lastCount));
    }
    
    Spark.publish("previousDownloadRate", String(downloadsPerInterval));
    downloadsPerInterval = max(1, count - lastCount);
    Spark.publish("downloadsPerInterval", String(downloadsPerInterval) + " downloads");
    
    // Display the count
    displayCount();
    
    updating = false;
}

void displayCount() {
    int i = 0;
    int n = numPlaces(count);
    int tmp = count;
    do {
        int digit = tmp % 10;
        lc.setDigit(0, i, digit, i % 3 == 0 && i > 0);
        i++;
        tmp=(int)(tmp/10);
    } while (i < n);
    
    while(i < 8) {
        lc.setChar(0, i, ' ', false);
        i++;
    }
}

int numPlaces (int n) {
    if (n == 0) return 1;
    return floor (log10 (abs (n))) + 1;
}
