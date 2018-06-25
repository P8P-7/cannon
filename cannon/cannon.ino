#include <DynamixelSerial.h>
#include <Timer.h>

#define PIN_SERVO_DIRECTION 0x02
#define PIN_VALVE 0x05
#define PIN_COIL 0x06
#define PIN_VALVE_BUTTON 0x07
#define PIN_COIL_BUTTON 0x08
#define PIN_JOYSTICK_X A0
#define PIN_JOYSTICK_Y A1

#define SERVO_TURN 0x02
#define SERVO_TILT 0x01

#define JOYSTICK_DEADZONE_MIN 450
#define JOYSTICK_DEADZONE_MAX 562

#define DELAY_OPEN_CLOSE_VALVE 275
#define DELAY_CLOSE_VALVE_FIRE 200
#define DELAY_FIRE_STOP_FIRE 370
#define DELAY_RE_AUTO_FIRE 380

#define RIGHT 1

bool loading;
bool firing;
bool autoFiring;

Timer autoFireTimer;

void joystickToServo(double joystickValue, int servo) {
    if (joystickValue >= JOYSTICK_DEADZONE_MAX) {
        Dynamixel.turn(servo, RIGHT, map(joystickValue, 512, 1023, 0, 1023));
    } else if (joystickValue < JOYSTICK_DEADZONE_MIN) {
        Dynamixel.turn(servo, LEFT, map(joystickValue, 512, 0, 0, 1023));
    } else {
        Dynamixel.turn(servo, RIGHT, 0);
    }
}

void joystickToTurn(double joystickValue, int servo) {
    if (joystickValue >= JOYSTICK_DEADZONE_MAX) {
        Dynamixel.turn(servo, RIGHT, map(joystickValue, 512, 1023, 0, 511));
    } else if (joystickValue < JOYSTICK_DEADZONE_MIN) {
        Dynamixel.turn(servo, LEFT, map(joystickValue, 512, 0, 0, 511));
    } else {
        Dynamixel.turn(servo, RIGHT, 0);
    }
}

void load() {
    if (!loading) {
        digitalWrite(PIN_COIL, LOW);
        loading = true;
    }
}

void stopLoad() {
    if (loading) {
        digitalWrite(PIN_COIL, HIGH);
        loading = false;
    }
}

void fire() {
    if (!firing) {
        digitalWrite(PIN_VALVE, LOW);
        firing = true;
    }
}

void stopFire() {
    if (firing) {
        digitalWrite(PIN_VALVE, HIGH);
        firing = false;
    }
}

void autoFire() {
    load();
    autoFireTimer.after(DELAY_OPEN_CLOSE_VALVE, &stopLoad);
    autoFireTimer.after(DELAY_CLOSE_VALVE_FIRE, &fire);
    autoFireTimer.after(DELAY_FIRE_STOP_FIRE, &stopFire);
    autoFireTimer.after(DELAY_RE_AUTO_FIRE, &stopAutoFire);
}

void stopAutoFire() {
    autoFiring = false;
}

void setup() {
    pinMode(PIN_VALVE, OUTPUT);
    pinMode(PIN_COIL, OUTPUT);

    pinMode(PIN_JOYSTICK_X, INPUT);
    pinMode(PIN_JOYSTICK_Y, INPUT);
    pinMode(PIN_VALVE_BUTTON, INPUT);
    pinMode(PIN_COIL_BUTTON, INPUT);

    Dynamixel.begin(1000000, PIN_SERVO_DIRECTION);
    Dynamixel.setEndless(SERVO_TURN, ON);
    Dynamixel.setEndless(SERVO_TILT, ON);

    loading = false;
    firing = false;
    autoFiring = false;

    digitalWrite(PIN_COIL, HIGH);
}

void loop() {  
    joystickToTurn(analogRead(PIN_JOYSTICK_X), SERVO_TURN);
    joystickToServo(analogRead(PIN_JOYSTICK_Y), SERVO_TILT);

    if (autoFiring) {
        autoFireTimer.update();
        return;
    }

    if (digitalRead(PIN_VALVE_BUTTON) == HIGH && digitalRead(PIN_COIL_BUTTON)) {
        autoFiring = true;
        autoFire();
    }

    if (digitalRead(PIN_COIL_BUTTON) == HIGH) {
        load(); 
    } else {
        stopLoad();
    }

    if (digitalRead(PIN_VALVE_BUTTON) == HIGH) {
        stopFire();
    } else {
        fire();
    }
}

