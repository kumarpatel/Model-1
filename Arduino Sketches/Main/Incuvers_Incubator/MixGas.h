#ifdef INCLUDE_MIXGAS 
class MixGasSystem {
  private:
    int pinAssignment;
    
    long setMSecondsOn;
    long setMSecondsOff;
    long nextStatusChangeTimestamp;
    long tickTime;

    boolean enabled;
    boolean currentlyOn;
    boolean useInternalTiming;
    
  public:
    void SetupMixGas(int pin, boolean enabled) {
      #ifdef DEBUG_MIXGAS
        Serial.println(F("MixGas::Setup"));
        Serial.println(pin);
        Serial.println(enabled);
      #endif
      
      // Setup MixGas 
      this->useInternalTiming = enabled;
      this->pinAssignment = pin;
      pinMode(this->pinAssignment, OUTPUT);  
      this->MakeSafeState();
      this->currentlyOn = false;
    }

    void UpdateMixGasDeltas(long on, long off) {
      #ifdef DEBUG_MIXGAS
        Serial.println(F("MixGas::UpdateDeltas"));
        Serial.println(on);
        Serial.println(off);
      #endif
      this->setMSecondsOn = on;
      this->setMSecondsOff = off;
      this->nextStatusChangeTimestamp = 0;
    }

    void UpdateMixGasCycle(boolean currentlyOn, long nextCycleDelta) {
      this->nextStatusChangeTimestamp = millis() + nextCycleDelta;
      this->currentlyOn = currentlyOn;
      if (currentlyOn) {
        #ifdef DEBUG_MIXGAS
          Serial.println(F("MixGas::Turning On"));
        #endif
        digitalWrite(this->pinAssignment, HIGH);
      } else {
        #ifdef DEBUG_MIXGAS
          Serial.println(F("MixGas::Turning Off"));
        #endif
        digitalWrite(this->pinAssignment, LOW);
      }
    }

    void MakeSafeState() {
      #ifdef DEBUG_MIXGAS
        Serial.println(F("MixGas::SafeState"));
      #endif
      digitalWrite(this->pinAssignment, LOW);     // Set LOW (mixgas off)
      // this->currentlyOn = false; // don't set this so we can resume old operation when we are done.
    }

    void UpdateMode(int mode) {
      this->nextStatusChangeTimestamp = 0;
      this->currentlyOn = false;
      if (mode == 1) {
        this->enabled = true;
        this->useInternalTiming = true;
      } else if (mode == 2) {
        this->enabled = true;
        this->useInternalTiming = false;
      } else {
        MakeSafeState();
        this->enabled = false;
        this->useInternalTiming = false;
      }
    }
    void DoTick() {
      this->tickTime = millis();

      if (this->useInternalTiming && this->nextStatusChangeTimestamp < this->tickTime) {
        if (this->currentlyOn) {
          this->nextStatusChangeTimestamp = this->tickTime + this->setMSecondsOff;
          digitalWrite(this->pinAssignment, LOW);
          this->currentlyOn = false;
        } else {
          this->nextStatusChangeTimestamp = this->tickTime + this->setMSecondsOn;
          digitalWrite(this->pinAssignment, HIGH);
          this->currentlyOn = true;
        } 
      } else if (this->currentlyOn) {
        digitalWrite(this->pinAssignment, HIGH); // this will allow us to resume after a suspended state ie setup mode.
      }
    }

    char GetSerialAPIndicator() {
      return GetIndicator(this->currentlyOn, false, false, true);
    }

    String GetOldUIDisplay() {
      //0123456789012345
      //LED: Off 10h18m
      
      String line = F("LED: "); // 5 chars
      if (this->currentlyOn) {
        line = String(line + F("On "));  // 8 chars
      } else {
        line = String(line + F("Off ")); // 9 chars
      }

      line = String(line + ConvertMillisToScaledReadable(this->nextStatusChangeTimestamp - this->tickTime, 7, false) + F("    "));

      return line;
    }

    char GetNewUIIndicator() {
      return GetIndicator(this->currentlyOn, false, false, true);
    }

    String GetNewUIReading() {
      return ConvertMillisToScaledReadable(this->nextStatusChangeTimestamp, 4, false);
    }
    
};
#else
class MixGasSystem {
  private:
  public:
    void SetupMixGasing(int pin, boolean enabled) {
    }

    void UpdateMixGasDeltas(long on, long off) {
    }

    void UpdateMixGasCycle(boolean currentlyOn, long nextCycleDelta) {
    }

    void MakeSafeState() {
    }
  
    void DoTick() {
    }

    void UpdateMode(int mode) {
    }

    char GetSerialAPIndicator() {
      return 'x';
    }

    String GetOldUIDisplay() {
      return F("LED: not incl.");
    }

    char GetNewUIIndicator() {
      return 'x';
    }

    String GetNewUIReading() {
      return F("n/i");
    }
};
#endif
