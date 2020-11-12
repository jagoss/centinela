#include "mbed.h"

using namespace std::chrono;

DigitalOut voltageRegultator(PB_12);
DigitalOut trigger(PB_5);
AnalogIn raindrop(PC_2);
DigitalIn isRaining(PA_8);
InterruptIn echo(PC_7);
LowPowerTimer timer;
LowPowerTimeout timeOut;
float distInm = 0;
unsigned long timeUs = 0;
// double beginTime = 0;
// double endTime = 0;
char *msg;
const char *ALERT_MSG = "RAINING";
UnbufferedSerial pc(USBTX, USBRX, 115200);
Thread eventThread;
EventQueue queue;

FileHandle *mbed::mbed_override_console(int fd)
{
    return &pc;
}

void printStart(float startTime) {
    printf("Start Time: %f\n", startTime);
}

void printEnd() {
    printf("Termina Timer!\n");
}

void build_msg_mqtt(char *msg, float distance, unsigned int epoch)
{
    sprintf(msg, "{'distance': %f, 'epoch': %u}", distance, epoch);
}

void printTimeDist(unsigned long timeUs, float dist) {
    printf("--------------------\n");
    // printf("Tiempo Inicio: %fus\n", beginTime);
    // printf("Tiempo Fin: %fus\n", endTime);
    printf("Tiempo total: %lu us\n", timeUs);
    printf("Distancia: %fm\n", dist);
}

void sendData(float dist)
{
    time_t seconds = time(NULL);
    build_msg_mqtt(msg, dist, (unsigned int) seconds);
    // sendMessage(msg);
}

void startTimer() {
    timer.stop();
    timer.reset();
    timer.start();
}

void sendAlert() {
    // send ALERT_MSG via mqtt
}

void start()
{
    trigger.write(1);
    ThisThread::sleep_for(0.01);
    trigger.write(0);
    if (isRaining.read() == 1) {
        sendAlert();
    }
}

void processMeassures()
{
    timer.stop();
    // endTime = timer.elapsed_time();
    timeUs = duration_cast<milliseconds>(timer.elapsed_time()).count();
    timer.reset();
    distInm = (timeUs*0.034)/200;
    queue.call(&printTimeDist, timeUs, distInm);
    queue.call(&sendData, distInm);
    timeOut.attach(&start, 5s);
}

int main()
{
    set_time(1256729737);
    voltageRegultator.write(1);
    printf("LECTURA DE MEDIDAS\n------------------\n");
    trigger.write(0);
    ThisThread::sleep_for(0.002);
    eventThread.start(callback(&queue, &EventQueue::dispatch_forever));
    printf("Event Thread creado!");
    echo.rise(&startTimer);
    echo.fall(&processMeassures);
    timeOut.attach(&start, 5s);
}
