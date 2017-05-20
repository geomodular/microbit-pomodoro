#include "MicroBit.h"

MicroBit uBit;

#define INTERVAL 60000 // [ms]

enum State {
    IDLE,
    RUNNING,
    DONE,
    PAUSE
};

static struct {
    enum State state;
    unsigned long t_last;
    unsigned long t_actual;
    int pixel; // actual pixel
} AppState;

struct Pixel {
    int x;
    int y;
};

struct Pixel getPixel(int index)
{
    return (struct Pixel) {
        .x = index / 5,
        .y = index % 5
    };
}

void beep()
{
    uBit.io.P0.setAnalogValue(511);
    uBit.io.P0.setAnalogPeriod(1);
    uBit.sleep(90);
    uBit.io.P0.setAnalogPeriod(0);
}

void beep3x()
{
    beep();
    uBit.sleep(1000);
    beep();
    uBit.sleep(1000);
    beep();
}

void onButton(MicroBitEvent e)
{
    if (e.value == MICROBIT_BUTTON_EVT_CLICK) {
        if (e.source == MICROBIT_ID_BUTTON_B) {
            if (AppState.state == IDLE) {
                beep();
                AppState.state = RUNNING;
            } else if (AppState.state == RUNNING) {
                beep();
                AppState.state = PAUSE;
            } else if (AppState.state == PAUSE) {
                beep();
                AppState.state = RUNNING;
            }
        } else if (e.source == MICROBIT_ID_BUTTON_A) {
            if (AppState.state == PAUSE || AppState.state == DONE) {
                beep();
                uBit.display.clear();
                AppState.t_last = 0;
                AppState.t_actual = 0;
                AppState.pixel = 0;
                AppState.state = IDLE;
            }
        }
    }
}

int main()
{
    uBit.init();
    uBit.display.setDisplayMode(DISPLAY_MODE_GREYSCALE);
    uBit.display.scroll("Pomodoro", 75);

    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);

    /* General timer */
    unsigned long t_last = uBit.systemTime();
    while(1) {
        unsigned long t_actual = uBit.systemTime();
        unsigned long t_dt = t_actual - t_last;
        t_last = t_actual;

        if (AppState.state == RUNNING) {
            AppState.t_actual += t_dt;

            /* New pixel */
            if (AppState.t_actual - AppState.t_last > INTERVAL) {
                struct Pixel p = getPixel(AppState.pixel);
                uBit.display.image.setPixelValue(p.y, p.x, 255);
                AppState.pixel++;
                AppState.t_last = AppState.t_actual;

                if (AppState.pixel == 25) {
                    AppState.state = DONE;
                    beep3x();
                }
            }
        }

        if (AppState.state != IDLE && AppState.state != DONE) {
            int brightness = 0;
            if (AppState.state != PAUSE)
                brightness = t_actual % 2000 > 1000 ? 0 : 32; // ~= 1 sec
            struct Pixel p = getPixel(AppState.pixel);
            uBit.display.image.setPixelValue(p.y, p.x, brightness);
        }

        if (AppState.state == DONE) {
            for (int i = 0; i < 25; i++) {
                struct Pixel p = getPixel(i);
                uBit.display.image.setPixelValue(p.y, p.x, t_actual % 1000 > 500 ? 0 : 255);
            }
        }

        uBit.sleep(10); // [ms]
    }
}

