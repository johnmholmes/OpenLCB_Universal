float ServoEasing::CubicEaseIn(float aFactorOfTimeCompletion) {
    return (aFactorOfTimeCompletion * QuadraticEaseIn(aFactorOfTimeCompletion));
}

float ServoEasing::EaseOutBounce(float aFactorOfTimeCompletion) {
    float tFactorOfMovementCompletion;
    if (aFactorOfTimeCompletion < 4 / 11.0) {
        tFactorOfMovementCompletion = (121 * aFactorOfTimeCompletion * aFactorOfTimeCompletion) / 16.0;
    } else if (aFactorOfTimeCompletion < 8 / 11.0) {
        tFactorOfMovementCompletion = (363 / 40.0 * aFactorOfTimeCompletion * aFactorOfTimeCompletion)
                - (99 / 10.0 * aFactorOfTimeCompletion) + 17 / 5.0;
    } else if (aFactorOfTimeCompletion < 9 / 10.0) {
        tFactorOfMovementCompletion = (4356 / 361.0 * aFactorOfTimeCompletion * aFactorOfTimeCompletion)
                - (35442 / 1805.0 * aFactorOfTimeCompletion) + 16061 / 1805.0;
    } else {
        tFactorOfMovementCompletion = (54 / 5.0 * aFactorOfTimeCompletion * aFactorOfTimeCompletion)
                - (513 / 25.0 * aFactorOfTimeCompletion) + 268 / 25.0;
    }
    return tFactorOfMovementCompletion;
}


mUserEaseInFunction(aFactorOfTimeCompletion, UserDataPointer)

#  if defined(ENABLE_EASE_USER)
void ServoEasing::registerUserEaseInFunction(float (*aUserEaseInFunction)(float aFactorOfTimeCompletion, void *aUserDataPointer),
        void *aUserDataPointer) {
    mUserEaseInFunction = aUserEaseInFunction;
    UserDataPointer = aUserDataPointer;
}
void ServoEasing::setUserDataPointer(void *aUserDataPointer) {
    UserDataPointer = aUserDataPointer;
}

uint8_t activeServo;
bool uint8_t activeServoSent[NUM_PDA_SERVO];
float (*aUserEaseInFunction)(float aFactorOfTimeCompletion, void *aUserDataPointer) {
  if(aFactorOfTimeCompletion>0.5) {
    if( target[activeServo] == NODECONFIG.read( EEADDR( servo[activeServo].easing)) ) {
      send(..);
      activeServoSent[activeServo] = true;
    } else {
      send(..);
      activeServoSent[activeServo] = true;
    }
  }
  uint8_t f = NODECONFIG.read( EEADDR( servo[activeServo].easing));
  switch (f) {
    case 0: return ServoEasing::EaseOutLinear(aFactorOfTimeCompletion);
    case 1: return ServoEasing::CubicEaseIn(aFactorOfTimeCompletion);
    case 2: return ServoEasing::EaseOutBounce(aFactorOfTimeCompletion);
    case 3: // pull with pause;
      if(aFactorOfTimeCompletion<0.40) return ServoEasing::EaseOutLinear(aFactorOfTimeCompletion*1.25); // 0-0.4 --> 0->0.5
      if(aFactorOfTimeCompletion<0.60) return ServoEasing::EaseOutLinear(0.5); // 0.4-0.5 --> 0.5
      return ServoEasing::EaseOutLinear((aFactorOfTimeCompletion-0.6)*1.25);  // 0.6-0.99 -> 0.5->0.99
  }
}