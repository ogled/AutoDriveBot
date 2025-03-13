#include "iq2_cpp.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <stdbool.h>
#include <vex_device.h>
using namespace vex;

brain Brain;
motor LeftMotor(PORT6);
motor RightMotor(PORT1,true);
motor TurnMotor(PORT2);
optical LeftOptical(PORT4);
optical RightOptical(PORT5);
inertial BrainInertial = inertial();
touchled leftLed(PORT7);
touchled rightLed(PORT8);
//Константы
constexpr int TURN_SPEED = 80;
constexpr double kP = 0.1;  // Пропорциональный коэффициент
constexpr double kI = 0.0;  // Интегральный коэффициент (если требуется)
constexpr double kD = 0;  // Дифференциальный коэффициент
constexpr bool debugSounds = true;
enum lrsType {
    LEFT,
    RIGHT,
    STRAIGHT
};
lrsType turning = STRAIGHT;
//Все остальное
int DefaultLeftMotorSpeed = 20;
int DefaultRightMotorSpeed = 20;

std::map<vex::motor*, int> motorMap{
    {&LeftMotor, DefaultLeftMotorSpeed},
    {&RightMotor, DefaultRightMotorSpeed}
};


int speed = 50;
double timeRobot = 0;

void startTimer()
{
    timeRobot = Brain.timer(timeUnits::msec);
}
void sleepForTime(int msec)
{
    while(Brain.timer(timeUnits::msec) - (double)timeRobot <= msec)
    {
        
    }
}

void smoothlySetSpeed()
{
    int currentSpeed1 = LeftMotor.velocity(percent);  // Текущая скорость мотора 1
    int currentSpeed2 = RightMotor.velocity(percent);  // Текущая скорость мотора 2

    // Плавное изменение скорости для мотор 1
    if (currentSpeed1 < speed) {
        // Увеличение скорости
        for (int speed1 = currentSpeed1; speed1 <= speed; speed1++) {
            LeftMotor.setVelocity(speed1, percent);
            LeftMotor.spin(forward);
            RightMotor.setVelocity(speed1, percent);  // Устанавливаем одинаковую скорость для мотор 2
            RightMotor.spin(forward);
            task::sleep(20);  // Задержка для плавного изменения
        }
    } else {
        // Уменьшение скорости
        for (int speed1 = currentSpeed1; speed1 >= speed; speed1--) {
            LeftMotor.setVelocity(speed1, percent);
            LeftMotor.spin(forward);
            RightMotor.setVelocity(speed1, percent);  // Устанавливаем одинаковую скорость для мотор 2
            RightMotor.spin(forward);
            task::sleep(20);  // Задержка для плавного изменения
        }
    }
}
void setSpeed(motor* mot, int speedMotor)
{
    if(motorMap.find(mot) != motorMap.end())
    {
        motorMap[mot] = speedMotor;
        mot->setVelocity(speedMotor, percentUnits::pct);
    }
}
int getSpeed(motor* mot)
{
    return motorMap.find(mot)->second;
}
void setSpeedTwoMotor(int speedMotors)
{
    setSpeed(&LeftMotor,speedMotors);
    setSpeed(&RightMotor,speedMotors);
}
void resetMotorsSpeed()
{
    setSpeed(&LeftMotor,DefaultLeftMotorSpeed);
    setSpeed(&RightMotor,DefaultRightMotorSpeed);
}
void taskTuring()
{
    rightLed.setBrightness(100);
    leftLed.setBrightness(100);
    Brain.Screen.setFont(fontType::mono12);
    while (true)
    {
        Brain.Screen.clearLine(4);
        if (turning == STRAIGHT)
        {
            leftLed.setColor(orange);
            rightLed.setColor(orange);
            Brain.Screen.printAt(1, 40, "STRAIGHT");
        }
        else if (turning == LEFT && (LeftMotor.velocity(percent) > -10 && RightMotor.velocity(percent) > -10))
        {
            rightLed.off();
            Brain.Screen.printAt(1, 40, "LEFT     ");
            leftLed.setColor(orange);
            task::sleep(500);
            leftLed.off();
            Brain.Screen.clearLine(4);
            task::sleep(500);
            continue;
        }
        else if (turning == RIGHT && (LeftMotor.velocity(percent) > -10 && RightMotor.velocity(percent) > -10))
        {
            leftLed.off();
            Brain.Screen.printAt(1, 40, "RIGHT     ");
            rightLed.setColor(orange);
            task::sleep(500);
            rightLed.off();
            Brain.Screen.clearLine(4);
            task::sleep(500);
            continue;
        }
        else if(turning == LEFT && (LeftMotor.velocity(percent) < -10 || RightMotor.velocity(percent) < -10))
        {
            leftLed.off();
            Brain.Screen.printAt(1, 40, "RIGHTL     ");
            rightLed.setColor(orange);
            task::sleep(500);
            rightLed.off();
            Brain.Screen.clearLine(4);
            task::sleep(500);
            continue;
        }
        else if (turning == RIGHT && (LeftMotor.velocity(percent) < 10 || RightMotor.velocity(percent) < 10))
        {
            rightLed.off();
            Brain.Screen.printAt(1, 40, "LEFTR     ");
            leftLed.setColor(orange);
            task::sleep(500);
            leftLed.off();
            Brain.Screen.clearLine(4);
            task::sleep(500);
            continue;
        }
        task::sleep(100);
    }
}
// Функция для рисования первого смайлика
void drawCoolFace1()
{
    for (int frame = 0; frame < 4; frame++) // 4 кадра движения очков
    {
        int glassesY = 40 - frame;
        int mouthY = (frame % 2 == 0) ? 65 : 67;
        Brain.Screen.clearScreen();

        // Голова (желтый круг)
        Brain.Screen.setPenColor(black);
        Brain.Screen.setFillColor(yellow);
        Brain.Screen.drawCircle(80, 50, 30);

        // Очки (движутся плавно вверх)
        Brain.Screen.setFillColor(black);
        Brain.Screen.drawRectangle(60, glassesY, 15, 10);
        Brain.Screen.drawRectangle(85, glassesY, 15, 10);
        Brain.Screen.drawLine(60, glassesY + 5, 100, glassesY + 5);

        // Улыбка
        Brain.Screen.setPenWidth(2);
        Brain.Screen.drawLine(65, mouthY, 95, mouthY);

        Brain.Screen.render();
        task::sleep(100);
    }

    for (int frame = 3; frame >= 0; frame--) // Очки опускаются вниз
    {
        int glassesY = 40 - frame;
        int mouthY = (frame % 2 == 0) ? 65 : 67;

        Brain.Screen.clearScreen();
        Brain.Screen.setPenColor(black);
        Brain.Screen.setFillColor(yellow);
        Brain.Screen.drawCircle(80, 50, 30);

        Brain.Screen.setFillColor(black);
        Brain.Screen.drawRectangle(60, glassesY, 15, 10);
        Brain.Screen.drawRectangle(85, glassesY, 15, 10);
        Brain.Screen.drawLine(60, glassesY + 5, 100, glassesY + 5);

        Brain.Screen.setPenWidth(2);
        Brain.Screen.drawLine(65, mouthY, 95, mouthY);

        Brain.Screen.render();
        task::sleep(100);
    }
}
// Функция для рисования второго смайлика
void drawCoolFace2()
{
    for (int frame = 0; frame < 4; frame++) // 4 кадра для анимации
    {
        int glassesY = 42 - frame;
        int mouthY = (frame % 2 == 0) ? 65 : 67;

        Brain.Screen.clearScreen();

        // Голова (розовый круг)
        Brain.Screen.setPenColor(black);
        Brain.Screen.setFillColor(red_orange);
        Brain.Screen.drawCircle(80, 50, 30);

        // Очки (движутся плавно вверх)
        Brain.Screen.setFillColor(black);
        Brain.Screen.drawRectangle(60, glassesY, 15, 10);
        Brain.Screen.drawRectangle(85, glassesY, 15, 10);
        Brain.Screen.drawLine(60, glassesY + 5, 100, glassesY + 5);

        // Улыбка
        Brain.Screen.setPenWidth(2);
        Brain.Screen.drawLine(65, mouthY, 95, mouthY);

        Brain.Screen.render();
        task::sleep(100);
    }

    for (int frame = 3; frame >= 0; frame--) // Очки опускаются вниз
    {
        int glassesY = 42 - frame;
        int mouthY = (frame % 2 == 0) ? 65 : 67;

        Brain.Screen.clearScreen();
        Brain.Screen.setPenColor(black);
        Brain.Screen.setFillColor(red_orange);
        Brain.Screen.drawCircle(80, 50, 30);

        Brain.Screen.setFillColor(black);
        Brain.Screen.drawRectangle(60, glassesY, 15, 10);
        Brain.Screen.drawRectangle(85, glassesY, 15, 10);
        Brain.Screen.drawLine(60, glassesY + 5, 100, glassesY + 5);

        Brain.Screen.setPenWidth(2);
        Brain.Screen.drawLine(65, mouthY, 95, mouthY);

        Brain.Screen.render();
        task::sleep(100);
    }
}
// Функция для рисования третьего смайлика
void drawCoolFace3()
{
    for (int frame = 0; frame < 4; frame++) // 4 кадра для анимации
    {
        int glassesY = 38 - frame;
        int mouthY = (frame % 2 == 0) ? 65 : 67;

        Brain.Screen.clearScreen();

        // Голова (синий круг)
        Brain.Screen.setPenColor(black);
        Brain.Screen.setFillColor(blue);
        Brain.Screen.drawCircle(80, 50, 30);

        // Очки (движутся плавно вверх)
        Brain.Screen.setFillColor(black);
        Brain.Screen.drawRectangle(60, glassesY, 15, 10);
        Brain.Screen.drawRectangle(85, glassesY, 15, 10);
        Brain.Screen.drawLine(60, glassesY + 5, 100, glassesY + 5);

        // Улыбка
        Brain.Screen.setPenWidth(2);
        Brain.Screen.drawLine(65, mouthY, 95, mouthY);

        Brain.Screen.render();
        task::sleep(100);
    }

    for (int frame = 3; frame >= 0; frame--) // Очки опускаются вниз
    {
        int glassesY = 38 - frame;
        int mouthY = (frame % 2 == 0) ? 65 : 67;

        Brain.Screen.clearScreen();
        Brain.Screen.setPenColor(black);
        Brain.Screen.setFillColor(blue);
        Brain.Screen.drawCircle(80, 50, 30);

        Brain.Screen.setFillColor(black);
        Brain.Screen.drawRectangle(60, glassesY, 15, 10);
        Brain.Screen.drawRectangle(85, glassesY, 15, 10);
        Brain.Screen.drawLine(60, glassesY + 5, 100, glassesY + 5);

        Brain.Screen.setPenWidth(2);
        Brain.Screen.drawLine(65, mouthY, 95, mouthY);

        Brain.Screen.render();
        task::sleep(100);
    }
}
void setTurn(int degree, int plusMinus = 10)
{
    if(degree > 0)
    {
        turning = RIGHT;
        TurnMotor.spinToPosition(std::min(55,degree), rotationUnits::deg,TURN_SPEED,velocityUnits::pct, false);
        setSpeed(&LeftMotor, DefaultLeftMotorSpeed + plusMinus);
        if(degree >= 70)
        {
            setSpeed(&RightMotor, -2);
        }
        else
        {   
            setSpeed(&RightMotor, 0);
        }
        }
    else if(degree == 0)
    {
        turning = STRAIGHT;
        TurnMotor.spinToPosition(0, rotationUnits::deg,TURN_SPEED,velocityUnits::pct, true);
        resetMotorsSpeed();
    }
    else{
        turning = LEFT;
        TurnMotor.spinToPosition(std::max(-55,degree), rotationUnits::deg,TURN_SPEED,velocityUnits::pct, false);
        setSpeed(&RightMotor, DefaultRightMotorSpeed + plusMinus);
        if(degree <= -70)
        {
            setSpeed(&LeftMotor, -2);
        }
        else 
        {
            setSpeed(&LeftMotor, 0);
        }
    }
    
}
void followLinePID() {
    double error, lastError = 0, integral = 0, derivative, correction;
    while (true) {
        // Чтение отражённого света
        double leftValue = LeftOptical.brightness();
        double rightValue = RightOptical.brightness();
        // Вычисление ошибки
        error = leftValue - rightValue;
        // PID-расчёт
        integral += error;
        derivative = error - lastError;
        correction = kP * error + kI * integral + kD * derivative;
        // Обновляем прошлую ошибку
        lastError = error;

        if (error > 10) { // Робот слишком сильно отклонился влево
                setTurn(20); // Повернуть вправо
            } else if (error < -10) { // Робот слишком сильно отклонился вправо
                setTurn(-24); // Повернуть влево
            } else {
                setTurn(0); // Если всё в порядке, продолжить движение прямо
            }
        setSpeed(&LeftMotor, DefaultLeftMotorSpeed + correction);
        setSpeed(&RightMotor, DefaultRightMotorSpeed - correction);
        wait(100,msec);
    }
}
void runMotors(vex::directionType typ = forward)
{
    LeftMotor.spin(typ);
    RightMotor.spin(typ);
}
void stopMotors()
{
    LeftMotor.stop();
    RightMotor.stop();
}

void redColor()
{
    startTimer();
    int startRotation = BrainInertial.rotation();
    RightMotor.spinFor(0,rotationUnits::deg,false);
    LeftMotor.spinFor(0,rotationUnits::deg, true);
    setTurn(-70,4);
    stopMotors();
    wait(1000,msec);
    runMotors();
    LeftMotor.stop(brakeType::hold);
    LeftMotor.stop();
    wait(3,sec);
    while (BrainInertial.rotation(degrees) - startRotation >= -85) {
        if(LeftOptical.color() == red)
        {
            
            startRotation = startRotation - 90;
            wait(1,sec);
        }
    }

    stopMotors();
    setTurn(20,8);
    sleepForTime(3000);
    wait(100,msec);
}
void blueColor()
{
    stopMotors();
    resetMotorsSpeed();
    LeftMotor.spinFor(150,degrees,false);
    stopMotors();
    RightMotor.spinFor(150,degrees,true);
    setTurn(70,2);
    wait(1500,msec);
    RightMotor.setStopping(brakeType::coast);
    RightMotor.stop();
    LeftMotor.spin(reverse);
    while (BrainInertial.rotation(degrees) > -180) {
    // Выводим текущие градусы на экран Brain
    Brain.Screen.clearLine(1);
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Rotation: %.2f degrees", BrainInertial.rotation(degrees));

    // Ожидание перед следующей итерацией
    vex::task::sleep(10);
  }
    stopMotors();
    setTurn(0);
    resetMotorsSpeed();
    LeftMotor.spinFor(-300,degrees,false);
    RightMotor.spinFor(-300,degrees,true);
    vexSerialWriteBuffer(1, (uint8_t*)"Sleep", 6);
    wait(10,sec);
    startTimer();
    vexSerialWriteBuffer(1, (uint8_t*)"FromGarage", 11);
    LeftMotor.spinFor(350,degrees,false);
    RightMotor.spinFor(350,degrees,true);
    wait(1,sec);
    runMotors();
    setTurn(70,7);
    wait(500,msec);
    while (RightOptical.brightness() > 90) {
        vex::task::sleep(1);
    }
    wait(1,sec);
    while (RightOptical.brightness() < 85 && (RightOptical.hue() >= 235 || RightOptical.hue() <= 175)) {
        vex::task::sleep(1);
    }

    while (RightOptical.brightness() > 90 && (RightOptical.hue() >= 235 || RightOptical.hue() <= 175)) {
        vex::task::sleep(1);
    }
    if(debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    stopMotors();
    sleepForTime(7000);
}
void parallelPark() {
    int startRotation = BrainInertial.rotation();
    stopMotors();
    wait(500,msec);
    setTurn(40);
    wait(0.5, sec);
    runMotors(reverse);
    while (BrainInertial.rotation(degrees) - startRotation > -43) {}
    stopMotors();
    wait(300,msec);
    
    setTurn(0);
    LeftMotor.spinFor(-350,degrees,false);
    RightMotor.spinFor(-350,degrees,true);

    wait(0.5, sec);
    setTurn(-40);
    wait(0.5, sec);
    runMotors(reverse);
    while (BrainInertial.rotation(degrees) - startRotation < 0) {}
    stopMotors();
    wait(300,msec);
    wait(0.5, sec);

    setTurn(15);
    turning = STRAIGHT;
    wait(0.5, sec);
    resetMotorsSpeed();
    if(debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    wait(5, sec);

    setTurn(-40);
    wait(0.5, sec);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation > -43) {}
    stopMotors();
    wait(300,msec);


    setTurn(40);
    wait(0.5, sec);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation < 0) {}
    stopMotors();
    wait(300,msec);
    if(debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    stopMotors();
    Brain.playSound(soundType::fillup);
    setTurn(-15);
    turning = STRAIGHT;
}
int beepTask() {
    while (true) {
        if (LeftMotor.velocity(percent) < -10 || RightMotor.velocity(percent) < -10) {
            Brain.playNote(2,4,200); // Воспроизведение тона частотой 440 Гц на 100 мс
            vex::task::sleep(200);
        }
        vex::task::sleep(100); // Проверка каждые 100 мс
    }
    return 0;
}


int Brain_precision = 0, Console_precision = 0;

bool running = true;

void watchDog() {
    int lastTime = Brain.Timer.system();
    while (running) {
        vex::this_thread::sleep_for(500);
        int currentTime = Brain.Timer.system();

        // Если программа "останавливается" (не обновляет таймер), можно выполнить действия
        if (currentTime == lastTime) {
            Brain.Screen.print("Program stopped!");
            Brain.playSound(soundType::doorClose);
            break;
        }
        lastTime = currentTime;
    }
}

int main() {
    
    vexGenericSerialEnable(1,115200);
    wait(100,msec);
    vex::thread monitor(watchDog);
    vexSerialWriteBuffer(1, (uint8_t*)"RobotCalibrating", 17);
    TurnMotor.setPosition(0,rotationUnits::deg);
    TurnMotor.setStopping(brakeType::coast);
    TurnMotor.setMaxTorque(100, percentUnits::pct);
    LeftMotor.setMaxTorque(100, percentUnits::pct);
    RightMotor.setMaxTorque(100, percentUnits::pct);
    LeftOptical.setLightPower(100);
    LeftOptical.setLight(vex::ledState::on);
    RightOptical.setLightPower(100);
    RightOptical.setLight(vex::ledState::on);
    Brain.Screen.setFont(fontType::mono12);
    vex::task beep(beepTask);

    if(Brain.Battery.capacity() < 40)
    {
        Brain.playSound(soundType::powerDown);
        Brain.Screen.print("The battery is dead");
        wait(5,sec);
        Brain.programStop();
    }
    startTimer();
    BrainInertial.calibrate();
    while (BrainInertial.isCalibrating()) {
        Brain.Screen.print("Calibrating...");
        vex::task::sleep(100);
        Brain.Screen.clearLine(1);
    }
    sleepForTime(9000);

    vexSerialWriteBuffer(1, (uint8_t*)"EndTask", 8);
    thread tur = thread(taskTuring);
    resetMotorsSpeed();
    runMotors();

    thread lineFollowerThread = thread(followLinePID);
    
    while (true) {
        vex::task::sleep(50);
        if(RightOptical.color() == colorType::red)
        {
            vexSerialWriteBuffer(1, (uint8_t*)"TurnLeft", 9);
            lineFollowerThread.interrupt();
            stopMotors();
            redColor();
            if(debugSounds)
            {
                Brain.playSound(soundType::tada);
            }
            resetMotorsSpeed();
            lineFollowerThread = thread(followLinePID);
            runMotors();
            vexSerialWriteBuffer(1, (uint8_t*)"EndTask", 8);
        }
        else if(RightOptical.hue() <= 235 && RightOptical.hue() >= 175)
        {
            vexSerialWriteBuffer(1, (uint8_t*)"ToGarage", 9);
            lineFollowerThread.interrupt();
            if(debugSounds)
            {
                Brain.playSound(soundType::tada);
            }
            stopMotors();
            blueColor();
            resetMotorsSpeed();
            runMotors();
            lineFollowerThread = thread(followLinePID);
            while(RightOptical.hue() >= 235 || RightOptical.hue() <= 175)
            {
                vex::task::sleep(10);
            }
            vexSerialWriteBuffer(1, (uint8_t*)"EndTask", 8);
            vex::task::sleep(600);
        }
        else if(RightOptical.color() == colorType::green)
        {
            vexSerialWriteBuffer(1, (uint8_t*)"ToParallelPark", 15);
            lineFollowerThread.interrupt();
            if(debugSounds)
            {
                Brain.playSound(soundType::tada);
            }
            stopMotors();
            parallelPark();
            tur.interrupt();

            vexSerialWriteBuffer(1, (uint8_t*)"TheEnd", 7);

            while (true)
            {
                int randomChoice = std::rand() % 3;
                if (randomChoice == 0)
                {
                    drawCoolFace1(); // Рисуем первый смайлик
                }
            else if (randomChoice == 1)
            {
                drawCoolFace2(); // Рисуем второй смайлик
            }
            else
            {
                drawCoolFace3(); // Рисуем третий смайлик
            }
            }
            vex::task::sleep(1000);
        }
        else if (BrainInertial.pitch(degrees) > 10 || BrainInertial.pitch(degrees) < -10) 
        {
            vexSerialWriteBuffer(1, (uint8_t*)"ToMountain", 11);
            lineFollowerThread.interrupt();
            stopMotors();
            if(debugSounds)
            {
                Brain.playSound(soundType::wrongWay);
            }
            vex::task::sleep(1000);

            resetMotorsSpeed();
            runMotors();
            lineFollowerThread = thread(followLinePID);

            while (BrainInertial.pitch(degrees) > 3 || BrainInertial.pitch(degrees) < -3) {
                Brain.Screen.clearScreen();
                Brain.Screen.setCursor(1, 1);
                Brain.Screen.print("Pitch: %.2f degrees", BrainInertial.pitch(degrees));
    
                vex::task::sleep(100);
            }
            Brain.Screen.setCursor(1, 1);
            Brain.Screen.clearScreen();
            Brain.Screen.print("Pitch normalized!");
            vexSerialWriteBuffer(1, (uint8_t*)"Task end\n", 10);
        }
    }
    
    return 0;
}
