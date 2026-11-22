#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PSP_MODULE_INFO("NHRA_2026", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

static unsigned int __attribute__((aligned(16))) list[262144];

typedef enum {
    STATE_PRE_STAGE,
    STATE_COUNTDOWN,
    STATE_RACING,
    STATE_FINISHED
} GameState;

int main(int argc, char* argv[]) {
    pspDebugScreenInit();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    sceGuInit();
    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, (void*)0, 512);
    sceGuDispBuffer(480, 272, (void*)0x88000, 512);
    sceGuOffset(2048 - (480 / 2), 2048 - (272 / 2));
    sceGuViewport(2048, 2048, 480, 272);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuFinish();
    sceGuSync(0, 0);
    sceGuDisplay(GU_TRUE);

    GameState state = STATE_PRE_STAGE;
    float carPositionX = 50.0f;
    float carSpeed = 0.0f;
    float acceleration = 0.0f;
    float timer = 0.0f;
    int countdownTimer = 3;
    unsigned int frameCount = 0;

    SceCtrlData pad;

    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);

        switch (state) {
            case STATE_PRE_STAGE:
                carPositionX = 50.0f;
                carSpeed = 0.0f;
                timer = 0.0f;
                if (pad.Buttons & PSP_CTRL_CROSS) {
                    state = STATE_COUNTDOWN;
                    frameCount = 0;
                }
                break;

            case STATE_COUNTDOWN:
                frameCount++;
                if (frameCount >= 60) {
                    countdownTimer--;
                    frameCount = 0;
                    if (countdownTimer < 0) {
                        state = STATE_RACING;
                    }
                }
                break;

            case STATE_RACING:
                timer += 0.016f;
                
                if (pad.Buttons & PSP_CTRL_RTRIGGER) {
                    acceleration = 0.4f;
                } else {
                    acceleration = -0.1f;
                }

                if (pad.Buttons & PSP_CTRL_LTRIGGER) {
                    carSpeed -= 0.5f;
                }

                carSpeed += acceleration;
                if (carSpeed < 0.0f) carSpeed = 0.0f;
                if (carSpeed > 15.0f) carSpeed = 15.0f;

                carPositionX += carSpeed;

                if (carPositionX >= 420.0f) {
                    state = STATE_FINISHED;
                }
                break;

            case STATE_FINISHED:
                if (pad.Buttons & PSP_CTRL_START) {
                    state = STATE_PRE_STAGE;
                    countdownTimer = 3;
                }
                break;
        }

        sceGuStart(GU_DIRECT, list);
        sceGuClearColor(0xff222222);
        sceGuClear(GU_COLOR_BUFFER_BIT);

        pspDebugScreenSetXY(2, 1);
        pspDebugScreenPrintf("=== NHRA: RACE DEPARTMENT 2026 (V0.1) ===");

        if (state == STATE_PRE_STAGE) {
            pspDebugScreenSetXY(2, 4);
            pspDebugScreenPrintf("FUNNY CAR PRONTO NA LINHA DE ARRANCADA");
            pspDebugScreenSetXY(2, 6);
            pspDebugScreenPrintf("Pressione [X] para iniciar a sequencia do semaforo.");
        } else if (state == STATE_COUNTDOWN) {
            pspDebugScreenSetXY(2, 4);
            if (countdownTimer > 0)
                pspDebugScreenPrintf("SEMAFORO DE LARGADA: %d", countdownTimer);
            else
                pspDebugScreenPrintf("SEMAFORO: VERDE! JA!");
        } else if (state == STATE_RACING) {
            pspDebugScreenSetXY(2, 3);
            pspDebugScreenPrintf("CRONOMETRO: %.3f s", timer);
            pspDebugScreenSetXY(2, 4);
            pspDebugScreenPrintf("VELOCIDADE: %.1f MPH", carSpeed * 10.0f);
            pspDebugScreenSetXY(2, 6);
            pspDebugScreenPrintf("[R] Acelerador  |  [L] Freio");
        } else if (state == STATE_FINISHED) {
            pspDebugScreenSetXY(2, 4);
            pspDebugScreenPrintf("FIM DE CORRIDA! TEMPO FINAL: %.3f s", timer);
            pspDebugScreenSetXY(2, 6);
            pspDebugScreenPrintf("Pressione [START] para nova tentativa.");
        }

        pspDebugScreenSetXY((int)(carPositionX / 15), 12);
        pspDebugScreenPrintf("[[ FUNNY CAR ]]");

        sceGuFinish();
        sceGuSync(0, 0);
        sceGuSwapBuffers();
    }

    sceGuTerm();
    sceKernelExitGame();
    return 0;
}
