#include <iq2_cpp.h>
#include <vector>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <vex_device.h>
#include <map>
#include <cstring>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <functional>
using namespace vex;

//Устройства:
brain Brain;
motor LeftMotor(PORT6);
motor RightMotor(PORT1,true);
motor TurnMotor(PORT2);
optical LeftOptical(PORT3);
optical RightOptical(PORT5);
optical LeftTwoOptical(PORT9);
optical RightTwoOptical(PORT4);
inertial BrainInertial = inertial();
touchled leftLed(PORT7);
touchled rightLed(PORT8);
//

//Константы:
const bool debugSounds = true;
const int DefaultLeftMotorSpeed = 20, DefaultRightMotorSpeed = 20, TURN_SPEED = 80;
const double minVelThreshold = 10.0;
int Brain_precision = 0, Console_precision = 0;
float myVariable;
enum lrsType {
    LEFT,
    RIGHT,
    STRAIGHT
};
//

//Кофициэнты:
double kP = 0.04, kI = 0.0, kD = 0.001, kGyro = 3, kWorkGyro = 15;
//

//Переменые
bool isPaused = false;
bool isTempPaused = false;
bool isEndSpeek = false;
bool PcIsConnected = false;
int speed = 50;
int timerLine = 0;
vex::color myColor(100,200,240);
bool showTurnLamps = true;
lrsType turning = STRAIGHT;
double timeRobot = 0;
//

/**
 * @brief Запуск таймера
 */
void startTimer()
{
    timeRobot = Brain.timer(timeUnits::msec);
}
void resetSpeek()
{
    isEndSpeek = false;
}
/**
 * @brief Ждать окончания таймера
 * @param msec Время ожидания в милисекундах
 */
void sleepForTime(int msec)
{
    while(Brain.timer(timeUnits::msec) - (double)timeRobot <= msec && !leftLed.pressing())
    {
        vex::wait(1, timeUnits::msec);
    }
}

void sleepForEndSpeek()
{
    if(PcIsConnected)
    {
        while(!isEndSpeek)
        {
            wait(20,msec);
        }
    }
    resetSpeek();
}
bool waitIsPaused()
{
    if(PcIsConnected)
    {
        while(isPaused && !isTempPaused)
        {
            wait(20,msec);
        }
        if(isTempPaused == true)
        {
            isTempPaused = false;
            return false;
        }
    }
    return true;
}
void setSpeedTwoMotor(int speedMotors)
{
    LeftMotor.setVelocity(speedMotors, percent);
    RightMotor.setVelocity(speedMotors, percent);
}

void resetMotorsSpeed()
{
    LeftMotor.setVelocity(DefaultLeftMotorSpeed, percent);
    RightMotor.setVelocity(DefaultRightMotorSpeed, percent);
}

void taskTuring()
{    
    rightLed.setBrightness(100);
    leftLed.setBrightness(100);
    Brain.Screen.setFont(fontType::mono12);
    while (true)
    {
        if(showTurnLamps)
        {
            Brain.Screen.clearLine(4);
            if (turning == STRAIGHT)
            {
                leftLed.on(myColor,100);
                rightLed.on(myColor,100);
                Brain.Screen.printAt(1, 40, "STRAIGHT");
            }
            else if (turning == LEFT && (std::abs(LeftMotor.velocity(percent)) < minVelThreshold || std::abs(RightMotor.velocity(percent)) < minVelThreshold))
            {
                rightLed.off();
                Brain.Screen.printAt(1, 40, "LEFT     ");
                leftLed.setColor(myColor);
                task::sleep(500);
                leftLed.off();
                Brain.Screen.clearLine(4);
                task::sleep(500);
                continue;
            }
            else if (turning == RIGHT && (std::abs(LeftMotor.velocity(percent)) < minVelThreshold || std::abs(RightMotor.velocity(percent)) < minVelThreshold))
            {
                leftLed.off();
                Brain.Screen.printAt(1, 40, "RIGHT     ");
                rightLed.setColor(myColor);
                task::sleep(500);
                rightLed.off();
                Brain.Screen.clearLine(4);
                task::sleep(500);
                continue;
            }
        }
        task::sleep(100);

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

/**
 * @brief Отрисовка смайлика
 * @param backgroundColor Фонововый цвет
 */
void drawCoolFace(color backgroundColor)
{
    for (int frame = 0; frame < 4; frame++) // 4 кадра движения очков
    {
        int glassesY = 40 - frame;
        int mouthY = (frame % 2 == 0) ? 65 : 67;
        Brain.Screen.clearScreen();

        // Голова (желтый круг)
        Brain.Screen.setPenColor(black);
        Brain.Screen.setFillColor(backgroundColor);
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
        Brain.Screen.setFillColor(backgroundColor);
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
        TurnMotor.spinToPosition(std::min(100,degree), rotationUnits::deg,TURN_SPEED,velocityUnits::pct, false);
        LeftMotor.setVelocity(DefaultLeftMotorSpeed + plusMinus, percent);
        if(degree >= 70)
        {
            RightMotor.setVelocity( -1, percent);
        }
        else
        {   
            RightMotor.setVelocity( 0, percent);
        }
        }
    else if(degree == 0)
    {
        TurnMotor.spinToPosition(0, rotationUnits::deg,TURN_SPEED,velocityUnits::pct, true);
        resetMotorsSpeed();
    }
    else{
        TurnMotor.spinToPosition(std::max(-100,degree), rotationUnits::deg,TURN_SPEED,velocityUnits::pct, false);
        RightMotor.setVelocity(DefaultRightMotorSpeed + plusMinus, percent);
        if(degree <= -70)
        {
            LeftMotor.setVelocity(-1, percent);
        }
        else 
        {
            LeftMotor.setVelocity(0, percent);
        }
    }
    
}
void followLinePID() {
    double error, lastError = 0, integral = 0, derivative, correction;
    while (true) {
        if(!isPaused || isTempPaused)
        {
            runMotors();
            // Чтение отражённого свет
            double leftValue =  LeftOptical.brightness() + LeftTwoOptical.brightness() * 1.5;
            double rightValue = RightOptical.brightness() + RightTwoOptical.brightness() * 1.5;
            // Вычисление ошибки (4 * (round(BrainInertial.rotation() / 90))
            error = rightValue - leftValue;
            // PID-расчёт
            integral += error;
            derivative = error - lastError;
            correction = kP * error + kI * integral + kD * derivative;
            // Обновляем прошлую ошибку
            lastError = error;

            if(std::abs(error) <= kWorkGyro && Brain.timer(timeUnits::msec) - timerLine >= 300)
            {
                Brain.Screen.drawRectangle(150,98,10,10,vex::color::green);
                correction -= (BrainInertial.rotation() - round(BrainInertial.rotation() / 90) * 90) * kGyro;
                setTurn((error - (BrainInertial.rotation() - round(BrainInertial.rotation() / 90) * 90) * kGyro) * 0.7, 5);
            }
            else
            {
                timerLine = Brain.timer(timeUnits::msec);
                Brain.Screen.drawRectangle(150,98,10,10,vex::color::red);
                setTurn(error, 5);
            }
            LeftMotor.setVelocity(DefaultLeftMotorSpeed + correction, percent);
            RightMotor.setVelocity(DefaultRightMotorSpeed - correction, percent);
            wait(65,msec);
        }
        else
        {
            stopMotors();
            wait(150,msec);
        }
    }
}

void redColor()
{
    showTurnLamps = true;
    turning = LEFT;
    startTimer();
    int startRotation = BrainInertial.rotation();
    setTurn(-4,4);
    RightMotor.spinFor(300,rotationUnits::deg,false);
    LeftMotor.spinFor(300,rotationUnits::deg, true);
    setTurn(-10,4);
    turning = LEFT;
    stopMotors();
    wait(200,msec);
    runMotors();
    if(RightOptical.brightness() <= 85)
    {
        while(RightOptical.brightness() <= 85 && BrainInertial.rotation(degrees) - startRotation >= -88)
        {
        
        }
    }
    else 
    {   while(RightOptical.brightness() >= 85 && BrainInertial.rotation(degrees) - startRotation >= -88)
        {

        }
    }
    stopMotors();
    turning = LEFT;
    setTurn(-65);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation >= -88) {
    }
    stopMotors();
    setTurn(20,8);
    turning = STRAIGHT;
    sleepForTime(3000);
    wait(300,msec);
}
void blueColor()
{
    stopMotors();
    resetMotorsSpeed();
    LeftMotor.spinFor(200,degrees,false);
    RightMotor.spinFor(200,degrees,true);
    setTurn(-50,2);
    turning = STRAIGHT;
    wait(1500,msec);
    LeftMotor.setStopping(brakeType::coast);
    LeftMotor.stop();
    RightMotor.spin(reverse);
    while (BrainInertial.rotation(degrees) < -98) {
    // Выводим текущие градусы на экран Brain
    Brain.Screen.clearLine(1);
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Rotation: %.2f degrees", BrainInertial.rotation(degrees));

    // Ожидание перед следующей итерацией
    vex::task::sleep(10);
  }
    stopMotors();
    setTurn(5);
    resetMotorsSpeed();
    LeftMotor.spinFor(-450,degrees,false);
    RightMotor.spinFor(-450,degrees,true);
    printf("Sleep\n");
    wait(10,sec);
    startTimer();
    printf("FromGarage\n");
    LeftMotor.spinFor(430,degrees,false);
    RightMotor.spinFor(430,degrees,true);
    wait(1,sec);
    runMotors();
    setTurn(-50,2);
    turning = LEFT;
    wait(500,msec);
    while (BrainInertial.rotation(degrees) > -180) {
        vex::task::sleep(1);
    }
    if(debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    stopMotors();
    LeftMotor.stop();
    RightMotor.stop();
    sleepForTime(7000);
    turning = STRAIGHT;
}
void parallelPark() {
    int startRotation = BrainInertial.rotation();
    stopMotors();
    wait(500,msec);
    setTurn(-40);
    wait(0.5, sec);
    runMotors(reverse);
    while (BrainInertial.rotation(degrees) - startRotation < 57) {}
    stopMotors();
    wait(300,msec);
    
    setTurn(0);
    wait(300,msec);
    LeftMotor.spinFor(-75,degrees,false);
    RightMotor.spinFor(-75,degrees,true);

    wait(0.5, sec);
    setTurn(55);
    wait(0.5, sec);
    runMotors(reverse);
    while (BrainInertial.rotation(degrees) - startRotation > 0) {}
    stopMotors();
    wait(300,msec);
    wait(0.5, sec);

    setTurn(-15);
    turning = STRAIGHT;
    wait(0.5, sec);
    resetMotorsSpeed();
    if(debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    wait(5, sec);

    setTurn(55);
    turning = RIGHT;
    wait(0.5, sec);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation < 43) {}
    stopMotors();
    wait(300,msec);

        
    setTurn(0);
    LeftMotor.spinFor(290,degrees,false);
    RightMotor.spinFor(290,degrees,true);

    wait(0.5, sec);
    setTurn(-40);
    turning = LEFT;
    wait(0.5, sec);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation > 0) {}
    stopMotors(); 
    wait(300,msec);
    if(debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    stopMotors();
    setTurn(-15);
    turning = STRAIGHT;
}
void start()
{
    printf("Start();\n");
    sleepForEndSpeek();
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

vex::colorType getColor()
{
    int hue1 = LeftTwoOptical.hue(); 
    int hue2 = RightTwoOptical.hue();
    int avgHue = (hue1 + hue2) / 2;

    double brightness1 = LeftTwoOptical.brightness();
    double brightness2 = RightTwoOptical.brightness();
    double avgBrightness = (brightness1 + brightness2) / 2;
    Brain.Screen.printAt(10, 50, "Hue: %d Bright: %.2f", avgHue, avgBrightness);
    if(avgBrightness < 70 && avgBrightness > 20)
    {
        if ((LeftTwoOptical.color() == vex::colorType::red && RightTwoOptical.color() == vex::colorType::red) || (avgHue >= 160 && avgHue < 190))
            return colorType::red;
        else if (LeftTwoOptical.color() == vex::colorType::green || RightTwoOptical.color() == vex::colorType::green || (avgHue >= 90 && avgHue <= 160 && avgBrightness > 0.2))
            return colorType::green;
        else if (avgHue >= 190 && avgHue <= 220)
            return colorType::blue;
        else
            return colorType::none;
    }
    else{
        return colorType::none;
    }
}

//Команды
void rgbCommand(const std::vector<std::string>& args) {
    Brain.playSound(vex::soundType::siren2);
    myColor = vex::color(std::stoi(args[0]),std::stoi(args[1]),std::stoi(args[2]));
}
void ConnectTrue(const std::vector<std::string>& args) {
    PcIsConnected = true;
}

void GetState(const std::vector<std::string>& args) {
    int Vel = (((LeftMotor.velocity(velocityUnits::dps) + RightMotor.velocity(velocityUnits::dps)) / 2) / 360.0) * 200.0;
    printf("State(%d,%d,%d);\n", Vel, (int)BrainInertial.rotation(degrees), Brain.Battery.capacity());
}
void BrakeProgram(const std::vector<std::string>& args)
{
    Brain.programStop();
}
void Pausep(const std::vector<std::string>& args)
{
    isPaused = true;
}
void Resumep(const std::vector<std::string>& args)
{
    isPaused = false;
}
void EndMedia(const std::vector<std::string>& args)
{
    isEndSpeek = true;
}
void NextStep(const std::vector<std::string>& args)
{
    isPaused = true;
    isTempPaused = true;
}
std::map<std::string, std::function<void(std::vector<std::string>)>> commands = {
    {"rgb", rgbCommand},
    {"connectTrue", ConnectTrue},
    {"GetState", GetState},
    {"BrakeProgram", BrakeProgram},
    {"Pause", Pausep},
    {"Resume", Resumep},
    {"NextStep", NextStep},
    {"EndMedia", EndMedia}
};

int monitoringTask() {
    while (true) {
        char buff[101];
        scanf("%100s%*c%*c", buff);

        char* parenOpen = std::strchr(buff, '(');
        char* parenClose = std::strchr(buff, ')');
        char* semicolon = std::strchr(buff, ';');

        if (!parenOpen || !parenClose || !semicolon || parenClose > semicolon) {
            continue;
        }

        std::string command(buff, parenOpen - buff);

        auto it = commands.find(command);
        if (it == commands.end()) {
            continue;
        }

        std::vector<std::string> args;
        char* argStart = parenOpen + 1;
        char* argEnd = parenClose;
        char* current = argStart;
        char* tokenStart = current;

        while (current <= argEnd) {
            if (*current == ',' || current == argEnd) {
                args.emplace_back(tokenStart, current - tokenStart);
                tokenStart = current + 1;
            }
            ++current;
        }

        it->second(args);
    }

    return 0;
}


int main() {
    TurnMotor.setPosition(0,rotationUnits::deg);
    TurnMotor.setStopping(brakeType::coast);
    TurnMotor.setMaxTorque(100, percentUnits::pct);
    LeftMotor.setMaxTorque(100, percentUnits::pct);
    RightMotor.setMaxTorque(100, percentUnits::pct);

    LeftOptical.setLightPower(100);
    LeftOptical.setLight(vex::ledState::on);
    RightOptical.setLightPower(100);
    RightOptical.setLight(vex::ledState::on);
    LeftTwoOptical.setLightPower(100);
    LeftTwoOptical.setLight(vex::ledState::on);
    RightTwoOptical.setLightPower(100);
    RightTwoOptical.setLight(vex::ledState::on);

    Brain.Screen.setFont(fontType::mono12);
    vex::task beep(beepTask);

    if(Brain.Battery.capacity() < 50)
    {
        Brain.playSound(soundType::powerDown);
        Brain.Screen.print("The battery is dead");
        wait(5,sec);
        Brain.programStop();
    }
    startTimer();
    BrainInertial.calibrate();

    printf("CheckState();\n");
    vex::task monitor(monitoringTask);

    while (BrainInertial.isCalibrating()) {
        Brain.Screen.print("Calibrating/Connecting...");
        vex::task::sleep(100);
        Brain.Screen.clearLine(1);
    }

    startTimer();
    while (PcIsConnected && Brain.timer(timeUnits::msec) - (double)timeRobot <= 5000 && !leftLed.pressing())
    {
        vex::wait(10, timeUnits::msec);
    }
    start();

    thread tur = thread(taskTuring);
    resetMotorsSpeed();
    runMotors();

    thread lineFollowerThread = thread(followLinePID);
    waitIsPaused();
    printf("RobotGo();\n");

    while (true) {
        vex::task::sleep(5);
        vex::color screenColor = LeftTwoOptical.color();
        Brain.Screen.drawRectangle(50,50,50,50, screenColor);

        if(LeftTwoOptical.color() == colorType::red && RightTwoOptical.color() == colorType::red)
        {
            printf("TurnLeft();\n");
            lineFollowerThread.interrupt();
            showTurnLamps = true;
            stopMotors();
            redColor();
            if(debugSounds)
            {
                Brain.playSound(soundType::tada);
            }
            sleepForEndSpeek();

            resetMotorsSpeed();
            lineFollowerThread = thread(followLinePID);
            runMotors();

            if(waitIsPaused())
            {
                printf("RobotGo();\n");
            }
        }
        else if(LeftTwoOptical.hue() <= 238 && LeftTwoOptical.hue() >= 140)
        {
            printf("Garage();\n");
            lineFollowerThread.interrupt();
            showTurnLamps = true;
            if(debugSounds)
            {
                Brain.playSound(soundType::tada);
            }
            stopMotors();
            blueColor();
            sleepForEndSpeek();

            resetMotorsSpeed();
            runMotors();
            lineFollowerThread = thread(followLinePID);
            if(waitIsPaused())
            {
                printf("RobotGo();\n");
            }
        }
        else if(LeftTwoOptical.hue() >= 36 && LeftTwoOptical.hue() <= 94)
        {
            printf("ParallelPark();\n");
            lineFollowerThread.interrupt();
            showTurnLamps = true;
            if(debugSounds)
            {
                Brain.playSound(soundType::tada);
            }
            stopMotors();
            tur.interrupt();
            parallelPark();
            sleepForEndSpeek();

            resetMotorsSpeed();
            runMotors();
            lineFollowerThread = thread(followLinePID);
            if(waitIsPaused())
            {
                printf("RobotGo();\n");
            }
        }
        else if (BrainInertial.pitch(degrees) > 10) 
        {
            lineFollowerThread.interrupt();
            showTurnLamps = true;
            stopMotors();
            if(debugSounds)
            {
                Brain.playSound(soundType::wrongWay);
            }
            printf("Mountain();\n");
            resetSpeek();
            vex::task::sleep(1000);

            resetMotorsSpeed();
            runMotors();
            lineFollowerThread = thread(followLinePID);

            while (BrainInertial.pitch(degrees) > -5) {
                Brain.Screen.clearScreen();
                Brain.Screen.setCursor(1, 1);
                Brain.Screen.print("Pitch: %.2f degrees", BrainInertial.pitch(degrees));
                vex::task::sleep(100);
            }
            lineFollowerThread.interrupt();
            showTurnLamps = true;
            stopMotors();

            Brain.Screen.setCursor(1, 1);
            Brain.Screen.clearScreen();
            Brain.Screen.print("Pitch normalized!");

            sleepForEndSpeek();
            vex::task::sleep(1000);

            resetMotorsSpeed();
            runMotors();
            lineFollowerThread = thread(followLinePID);
            if(waitIsPaused())
            {
                printf("RobotGo();\n");
            }
        }
        else if(BrainInertial.rotation(degrees) <= -355)
        {
            wait(2,sec);
            lineFollowerThread.interrupt();
            stopMotors();
            printf("End();\n");
            while (true)
            {
                drawCoolFace(color(std::rand() % 256,std::rand() % 256,std::rand() % 256));
            }
        }
    }
    
    return 0;
}