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
#include <map>
#include <array>
using namespace vex;

// Устройства:
brain Brain;
motor LeftMotor(PORT6);
motor RightMotor(PORT1, true);
motor TurnMotor(PORT2);
optical LeftOptical(PORT3);    
optical RightOptical(PORT5);   
optical LeftTwoOptical(PORT9); 
optical RightTwoOptical(PORT4);
inertial BrainInertial = inertial();
touchled leftLed(PORT7);
touchled rightLed(PORT8);
//

// Константы:
const int maxTurnJoystic = 50;
const int maxBroadcasts = 6;
const float accelFactor = 0.02f;
const float brakeFactor = 0.8f;

const int PermanentAccelPercent = 10;
const bool debugSounds = true;
const int DefaultLeftMotorSpeed = 20, DefaultRightMotorSpeed = 20, TURN_SPEED = 80;
int Brain_precision = 0, Console_precision = 0;
float myVariable;
enum lrsType
{
    LEFT,
    RIGHT,
    STRAIGHT,
    NULLLRS
};
struct SensorState {
    std::vector<int> values;
};
//

// Кофициэнты:
double kP = 0.02, kI = 0.01, kGyro = 5, kWorkGyro = 15;
//

// Переменые
int curentBroadcast = 0;
double gyroOffset = 0;
static float currentSpeed = 0.0f;
bool isJoysticMode = false;
bool isPaused = false;
bool isTempPaused = false;
bool isEndSpeek = false;
bool PcIsConnected = false;
int speed = 50;
int timerLine = 0;
vex::color myColor(100, 200, 240);
bool showTurnLamps = true;
lrsType turning = STRAIGHT;
double timeRobot = 0;

bool IsDroveTurnLeft = false;
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
    while (Brain.timer(timeUnits::msec) - (double)timeRobot <= msec && !leftLed.pressing())
    {
        vex::wait(2, timeUnits::msec);
    }
}
void sleepForEndSpeek()
{
    if (PcIsConnected)
    {
        while (!isEndSpeek)
        {
            wait(20, msec);
        }
    }
    resetSpeek();
}
bool waitIsPaused()
{
    if (PcIsConnected)
    {
        while (isPaused && !isTempPaused)
        {
            wait(20, msec);
        }
        if (isTempPaused == true)
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
void setSpeedTwoMotor(double speedMotors)
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
    lrsType lastType = NULLLRS;
    while (true)
    {
        if (showTurnLamps)
        {
            if(turning != lastType)
            {
                Brain.Screen.clearLine(1);
                Brain.Screen.setCursor(1, 1);
                Brain.Screen.print("Headlight   : %d", turning);
                lastType = turning;
            }
            if (turning == STRAIGHT)
            {
                leftLed.on(myColor, 100);
                rightLed.on(myColor, 100);
            }
            else if (turning == LEFT)
            {
                rightLed.off();
                leftLed.setColor(myColor);
                task::sleep(500);
                leftLed.off();
                task::sleep(500);
                continue;
            }
            else if (turning == RIGHT)
            {
                leftLed.off();
                rightLed.setColor(myColor);
                task::sleep(500);
                rightLed.off();
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
    if (degree > 0)
    {
        TurnMotor.spinToPosition(std::min(100, degree), rotationUnits::deg, TURN_SPEED, velocityUnits::pct, false);
        LeftMotor.setVelocity(DefaultLeftMotorSpeed + plusMinus, percent);
        if (degree >= 70)
        {
            RightMotor.setVelocity(-1, percent);
        }
        else
        {
            RightMotor.setVelocity(0, percent);
        }
    }
    else if (degree == 0)
    {
        TurnMotor.spinToPosition(0, rotationUnits::deg, TURN_SPEED, velocityUnits::pct, true);
        resetMotorsSpeed();
    }
    else
    {
        TurnMotor.spinToPosition(std::max(-100, degree), rotationUnits::deg, TURN_SPEED, velocityUnits::pct, false);
        RightMotor.setVelocity(DefaultRightMotorSpeed + plusMinus, percent);
        if (degree <= -70)
        {
            LeftMotor.setVelocity(-1, percent);
        }
        else
        {
            LeftMotor.setVelocity(0, percent);
        }
    }
}
void followLinePID()
{
    double error, integral = 0, correction;
    while (true)
    {
        if (!isPaused || isTempPaused)
        {
            runMotors();

            double leftValue = LeftOptical.brightness() + LeftTwoOptical.brightness() * 1.5;
            double rightValue = RightOptical.brightness() + RightTwoOptical.brightness() * 1.5;
            
            error = rightValue - leftValue;
            
            integral += error;
            correction = kP * error + kI * integral;

            double rawGyroError = BrainInertial.rotation() - round(BrainInertial.rotation() / 90) * 90;
            double gyroError = rawGyroError - gyroOffset;

            if (std::abs(error) <= kWorkGyro && Brain.timer(timeUnits::msec) - timerLine >= 300)
            {
                Brain.Screen.drawRectangle(150, 98, 10, 10, vex::color::green);
                correction -= gyroError * kGyro;
                setTurn((error - gyroError * kGyro) * 0.7);
                gyroOffset = gyroOffset * 0.9 + rawGyroError * 0.1;
            }
            else if(std::abs(error) > kWorkGyro)
            {
                timerLine = Brain.timer(timeUnits::msec);
                Brain.Screen.drawRectangle(150, 98, 10, 10, vex::color::red);
                setTurn(error / 3);
            }
            else
            {
                setTurn(error / 3);
            }

            if(correction >= 0)
            {
                RightMotor.setVelocity(std::max(5.0, DefaultRightMotorSpeed - correction), percent);
            }
            else
            {
                LeftMotor.setVelocity(std::max(5.0, DefaultLeftMotorSpeed + correction), percent);
            }
            
            wait(50, msec);
        }
        else
        {
            stopMotors();
            wait(150, msec);
        }
    }
}

class ControllerState
{
public:
    ControllerState()
    {
        for (int i = 0; i < 3; i++)
            analogs[i] = 127;
        for (int i = 3; i < maxAnalogs; i++)
            analogs[i] = 0;
        for (int i = 0; i < maxButtons; i++)
            buttons[i] = false;
    }

    bool UpdateFromString(std::string cmd)
    {
        // Уберём пробелы/переводы строк
        trim(cmd);

        if (cmd.size() < 4)
            return false;
        if (cmd.rfind("T:", 0) != 0)
            return false;

        size_t commaPos = cmd.find(',');
        if (commaPos == std::string::npos)
            return false;

        std::string key = cmd.substr(2, commaPos - 2);
        std::string valStr = cmd.substr(commaPos + 1);

        if (key.size() < 2)
            return false;
        char type = key[0];
        int index = key[1] - '0';
        if (index < 0)
            return false;
        int value = std::stoi(valStr);

        if (type == 'a')
        {
            if (index > maxAnalogs)
                return false;
            value = clamp(value, 0, 255);

            analogs[index] = value;
        }
        else if (type == 'b')
        {
            if (index > maxButtons)
                return false;
            buttons[index] = (value != 0);

            if (index == 4 && value == 1)
            {
                curentBroadcast--;
                if (curentBroadcast < 0)
                {
                    curentBroadcast = 0;
                }
            }
            else if (index == 5 && value == 1)
            {
                curentBroadcast++;
                if (curentBroadcast > maxBroadcasts)
                {
                    curentBroadcast = maxBroadcasts;
                }
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    int GetAnalog(int code) const
    {
        if (code < 0 || code > maxAnalogs)
            return 0;
        return analogs[code];
    }

    bool GetButton(int code) const
    {
        if (code < 0 || code > maxButtons)
            return false;
        return buttons[code];
    }

private:
    static const int maxAnalogs = 5;
    static const int maxButtons = 5;
    int analogs[maxAnalogs];
    bool buttons[maxButtons];

    static int clamp(int v, int minV, int maxV)
    {
        return (v < minV) ? minV : (v > maxV) ? maxV
                                              : v;
    }

    static void trim(std::string &s)
    {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
            s.pop_back();
        while (!s.empty() && (s.front() == '\n' || s.front() == '\r' || s.front() == ' '))
            s.erase(s.begin());
    }
};
ControllerState *remote;

void redColor()
{
    showTurnLamps = true;

    turning = LEFT;
    startTimer();
    int startRotation = BrainInertial.rotation();
    setTurn(-4, 4);
    RightMotor.spinFor(300, rotationUnits::deg, false);
    LeftMotor.spinFor(300, rotationUnits::deg, true);
    setTurn(-10, 4);
    stopMotors();
    wait(200, msec);
    runMotors();
    if (RightOptical.brightness() <= 85)
    {
        while (RightOptical.brightness() <= 85 && BrainInertial.rotation(degrees) - startRotation >= -88)
        {
        }
    }
    else
    {
        while (RightOptical.brightness() >= 85 && BrainInertial.rotation(degrees) - startRotation >= -88)
        {
        }
    }
    stopMotors();
    setTurn(-65);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation >= -88 && LeftOptical.brightness() > 50)
    {
        wait(10,msec);
    }
    stopMotors();
    setTurn(20, 8);
    turning = STRAIGHT;
    sleepForTime(3000);
    wait(300, msec);
}
void blueColor()
{
    stopMotors();
    resetMotorsSpeed();
    LeftMotor.spinFor(200, degrees, false);
    RightMotor.spinFor(200, degrees, true);
    setTurn(-50, 2);
    turning = STRAIGHT;
    wait(1500, msec);
    LeftMotor.setStopping(brakeType::coast);
    LeftMotor.stop();
    RightMotor.spin(reverse);
    while (BrainInertial.rotation(degrees) < -98)
    {
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
    LeftMotor.spinFor(-450, degrees, false);
    RightMotor.spinFor(-450, degrees, true);
    printf("Sleep\n");
    wait(10, sec);
    startTimer();
    printf("FromGarage\n");
    LeftMotor.spinFor(430, degrees, false);
    RightMotor.spinFor(430, degrees, true);
    wait(1, sec);
    runMotors();
    setTurn(-50, 2);
    turning = LEFT;
    wait(500, msec);
    while (BrainInertial.rotation(degrees) > -180)
    {
        vex::task::sleep(1);
    }
    if (debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    stopMotors();
    LeftMotor.stop();
    RightMotor.stop();
    sleepForTime(7000);
    turning = STRAIGHT;
}
void parallelPark()
{
    int startRotation = BrainInertial.rotation();
    stopMotors();
    wait(500, msec);
    setTurn(-40);
    wait(0.5, sec);
    runMotors(reverse);
    while (BrainInertial.rotation(degrees) - startRotation < 57)
    {
        wait(5,msec);
    }
    stopMotors();
    wait(300, msec);

    setTurn(0);
    wait(300, msec);
    LeftMotor.spinFor(-75, degrees, false);
    RightMotor.spinFor(-75, degrees, true);

    wait(0.5, sec);
    setTurn(55);
    wait(0.5, sec);
    runMotors(reverse);
    while (BrainInertial.rotation(degrees) - startRotation > 0)
    {
        wait(5,msec);
    }
    stopMotors();
    wait(300, msec);
    wait(0.5, sec);

    setTurn(-15);
    turning = STRAIGHT;
    wait(0.5, sec);
    resetMotorsSpeed();
    if (debugSounds)
    {
        Brain.playSound(soundType::tada);
    }
    wait(5, sec);

    setTurn(55);
    turning = RIGHT;
    wait(0.5, sec);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation < 43)
    {
        wait(5,msec);
    }
    stopMotors();
    wait(300, msec);

    setTurn(0);
    LeftMotor.spinFor(290, degrees, false);
    RightMotor.spinFor(290, degrees, true);

    wait(0.5, sec);
    setTurn(-40);
    turning = LEFT;
    wait(0.5, sec);
    runMotors();
    while (BrainInertial.rotation(degrees) - startRotation > 0)
    {
        wait(5,msec);
    }
    stopMotors();
    wait(300, msec);
    if (debugSounds)
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
int beepTask()
{
    while (true)
    {
        if (LeftMotor.velocity(percent) < -10 || RightMotor.velocity(percent) < -10)
        {
            Brain.playNote(2, 4, 200); // Воспроизведение тона частотой 440 Гц на 100 мс
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
    if (avgBrightness < 70 && avgBrightness > 20)
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
    else
    {
        return colorType::none;
    }
}

// Команды
void rgbCommand(const std::vector<std::string> &args)
{
    Brain.playSound(vex::soundType::siren2);
    myColor = vex::color(std::stoi(args[0]), std::stoi(args[1]), std::stoi(args[2]));
}
void ConnectTrue(const std::vector<std::string> &args)
{
    PcIsConnected = true;
}

void BrakeProgram(const std::vector<std::string> &args)
{
    Brain.programStop();
}
void Pausep(const std::vector<std::string> &args)
{
    isPaused = true;
}
void Resumep(const std::vector<std::string> &args)
{
    isPaused = false;
}
void EndMedia(const std::vector<std::string> &args)
{
    isEndSpeek = true;
}
void NextStep(const std::vector<std::string> &args)
{
    isPaused = true;
    isTempPaused = true;
}
void StartJM(const std::vector<std::string> &args)
{
    isJoysticMode = true;
}
void StopJM(const std::vector<std::string> &args)
{
    isJoysticMode = true;
}
void MS(const std::vector<std::string> &args)
{
    printf("MS\n");
}
std::map<std::string, std::function<void(std::vector<std::string>)>> commands = {
    {"rgb", rgbCommand},
    {"connectTrue", ConnectTrue},
    {"BrakeProgram", BrakeProgram},
    {"Pause", Pausep},
    {"Resume", Resumep},
    {"NextStep", NextStep},
    {"EndMedia", EndMedia},
    {"MS", MS},
    {"StartJM", StartJM},
    {"StopJM", StopJM}};

int monitoringTask()
{
    while (true)
    {
        char buff[101];
        scanf("%100s%*c%*c", buff);

        char *parenOpen = std::strchr(buff, '(');
        char *parenClose = std::strchr(buff, ')');
        char *semicolon = std::strchr(buff, ';');

        if (parenOpen || parenClose || semicolon || parenClose < semicolon)
        {
            std::string command(buff, parenOpen - buff);

            auto it = commands.find(command);
            if (it == commands.end())
            {
                continue;
            }

            std::vector<std::string> args;
            char *argStart = parenOpen + 1;
            char *argEnd = parenClose;
            char *current = argStart;
            char *tokenStart = current;

            while (current <= argEnd)
            {
                if (*current == ',' || current == argEnd)
                {
                    args.emplace_back(tokenStart, current - tokenStart);
                    tokenStart = current + 1;
                }
                ++current;
            }

            it->second(args);
        }
        else
        {
            // Brain.Screen.setCursor(1, 1);
            // Brain.Screen.clearScreen();
            // Brain.Screen.print(std::string(buff).c_str());

            remote->UpdateFromString(std::string(buff));
        }
    }

    return 0;
}


SensorState prevState = { { 0, 0, 0, 0 } };

int TaskSendDataSensors()
{
    while (true)
    {
        SensorState current;
        current.values.resize(5);

        current.values[0] = (int)((((LeftMotor.velocity(velocityUnits::dps) + RightMotor.velocity(velocityUnits::dps)) / 2) / 360.0) * 200.0);
        current.values[1] = (int)BrainInertial.rotation(degrees);
        current.values[2] = (int)Brain.Battery.capacity();
        current.values[3] = curentBroadcast;
        current.values[4] = TurnMotor.position(rotationUnits::deg);

        std::vector<std::string> changes;

        for (size_t i = 0; i < current.values.size(); i++) {
            if (current.values[i] != prevState.values[i]) {
                changes.push_back("S:s" + std::to_string(i) + "," + std::to_string(current.values[i]) + "\n");
                prevState.values[i] = current.values[i];
            }
        }

        for (auto& change : changes) {
            printf("%s", change.c_str());
        }

        vex::task::sleep(100);
    }
    return 0;
}

void MotorControllerForJoystic(int boost, int braking)
{
    if (curentBroadcast == 0)
    { 
        LeftMotor.stop(brakeType::hold);
        RightMotor.stop(brakeType::hold);
    }
    else if (curentBroadcast == 2)
    {
        LeftMotor.stop(brakeType::brake);
        RightMotor.stop(brakeType::brake);
    }
    else
    {
        float boostPercent = boost / 2.56f;
        float brakingPercent = (braking / 2.56f) * 2;

        float targetSpeed = boostPercent - brakingPercent + PermanentAccelPercent;

        if (targetSpeed < 0)
            targetSpeed = 0;

        if (targetSpeed > currentSpeed)
        {
            currentSpeed += (targetSpeed - currentSpeed) * accelFactor;
        }
        else
        {
            currentSpeed += (targetSpeed - currentSpeed) * brakeFactor;
        }

        if(curentBroadcast == 1 || curentBroadcast == 3)
        {
            setSpeedTwoMotor(static_cast<double>(currentSpeed));
        }
        else if(curentBroadcast == 4)
        {
            setSpeedTwoMotor(static_cast<double>(currentSpeed));
        }
        else if(curentBroadcast == 5)
        {
            setSpeedTwoMotor(static_cast<double>(currentSpeed * 0.50));
        }
        else if(curentBroadcast == 6)
        {
            setSpeedTwoMotor(static_cast<double>(currentSpeed * 0.25));
        }

        if(curentBroadcast != 1)
        {
            runMotors();
        }
        else{
            runMotors(reverse);
        }

    }
}

void calibrate()
{
    remote = new ControllerState();

    TurnMotor.setPosition(0, rotationUnits::deg);
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
    Brain.Screen.setFont(fontType::mono15);
    BrainInertial.calibrate();
    printf("CheckState();\n");
    startTimer();

    Brain.Screen.clearScreen();
    Brain.Screen.setCursor(1, 1);

    Brain.Screen.print("Calibrating...");


    Brain.Screen.newLine();
    while (BrainInertial.isCalibrating())
    {
        vex::task::sleep(100);
    }

    while (PcIsConnected && Brain.timer(timeUnits::msec) - (double)timeRobot <= 5000 && !leftLed.pressing())
    {
        vex::wait(10, timeUnits::msec);
    }
    start();
    resetMotorsSpeed();
}

int main()
{
    vex::task beep(beepTask);
    vex::task SendData(TaskSendDataSensors);
    vex::task monitor(monitoringTask);
    calibrate();
    thread tur = thread(taskTuring);

    runMotors();

    thread lineFollowerThread = thread(followLinePID);
    waitIsPaused();
    printf("RobotGo();\n");

    while (true)
    {
        while (!isJoysticMode)
        {
            vex::color screenColor = LeftTwoOptical.color();
            Brain.Screen.drawRectangle(50, 50, 50, 50, screenColor);

            if (LeftTwoOptical.color() == colorType::red && RightTwoOptical.color() == colorType::red)
            {
                if(!IsDroveTurnLeft)
                {
                    IsDroveTurnLeft = true;
                    printf("TurnLeft();\n");
                }
                lineFollowerThread.interrupt();
                showTurnLamps = true;
                stopMotors();
                redColor();
                if (debugSounds)
                {
                    Brain.playSound(soundType::tada);
                }
                sleepForEndSpeek();

                resetMotorsSpeed();
                lineFollowerThread = thread(followLinePID);
                runMotors();

                if (waitIsPaused())
                {
                    printf("RobotGo();\n");
                }
            }
            else if (LeftTwoOptical.hue() <= 238 && LeftTwoOptical.hue() >= 140)
            {
                printf("Garage();\n");
                lineFollowerThread.interrupt();
                showTurnLamps = true;
                if (debugSounds)
                {
                    Brain.playSound(soundType::tada);
                }
                stopMotors();
                blueColor();
                sleepForEndSpeek();

                resetMotorsSpeed();
                runMotors();
                lineFollowerThread = thread(followLinePID);
                if (waitIsPaused())
                {
                    printf("RobotGo();\n");
                }
            }
            else if (LeftTwoOptical.hue() >= 36 && LeftTwoOptical.hue() <= 94)
            {
                printf("ParallelPark();\n");
                lineFollowerThread.interrupt();
                showTurnLamps = true;
                if (debugSounds)
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
                if (waitIsPaused())
                {
                    printf("RobotGo();\n");
                }
            }
            else if (BrainInertial.pitch(degrees) > 10)
            {
                lineFollowerThread.interrupt();
                showTurnLamps = true;
                stopMotors();
                if (debugSounds)
                {
                    Brain.playSound(soundType::wrongWay);
                }
                printf("Mountain();\n");
                resetSpeek();
                vex::task::sleep(1000);

                resetMotorsSpeed();
                runMotors();
                lineFollowerThread = thread(followLinePID);

                while (BrainInertial.pitch(degrees) > -5)
                {
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
                if (waitIsPaused())
                {
                    printf("RobotGo();\n");
                }
            }
            else if (BrainInertial.rotation(degrees) <= -355)
            {
                wait(2, sec);
                lineFollowerThread.interrupt();
                stopMotors();
                printf("End();\n");
                while (true)
                {
                    drawCoolFace(color(std::rand() % 256, std::rand() % 256, std::rand() % 256));
                }
            }
            wait(10, msec);
        }
        lineFollowerThread.interrupt();
        resetMotorsSpeed();
        stopMotors();
        setTurn(0);

        int lastSpinTurn = 0;

        while (isJoysticMode)
        {
            Brain.Screen.clearLine(1);
            Brain.Screen.setCursor(1, 1);
            Brain.Screen.print("Turn - %d %", maxTurnJoystic * ((remote->GetAnalog(0) - 127) / 127));
            if (lastSpinTurn != maxTurnJoystic * ((remote->GetAnalog(0) - 127) / 127))
            {
                TurnMotor.spinToPosition(maxTurnJoystic * ((remote->GetAnalog(0) - 127) / 127), rotationUnits::deg, TURN_SPEED / 3, velocityUnits::pct, false);
                lastSpinTurn = maxTurnJoystic * ((remote->GetAnalog(0) - 127) / 127);
            }

            Brain.Screen.clearLine(2);
            Brain.Screen.setCursor(2, 1);
            Brain.Screen.print("Start - %d %", remote->GetAnalog(5) / 2.56);

            MotorControllerForJoystic(remote->GetAnalog(5), remote->GetAnalog(4));
            wait(5, msec);
        }
    }

    return 0;
}