#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55

int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint8_t ui8Adjust;
    ui8Adjust = 83;

    //SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); // CLOCK 40 MHZ
    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); // CLOCK 80 MHZ
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64); // SETA O CLOCK DO PWM POR: CLOCK_CPU/64

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); // HABILITA PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); // HABILITA PD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // HABILITA PF

    GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0); // DEFINIE PWM NO PINO PD0
    GPIOPinConfigure(GPIO_PD0_M1PWM0); // CONFIGURA PD0 COMO M1PWM0

    // INICIO HABILITA CHAVES SW1 E SW2
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // FIM HABILITA CHAVES SW1 E SW2

    ui32PWMClock = SysCtlClockGet() / 64; // PEGA CLOCK DEFINIDO PARA O PWM
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1; // PEGA O CLOCK DO PWM E DIVIDE PELO CLOCK COM QUE SE QUER TRABALHAR SUBTRAINDO 1 POR CONTAR ATÉ 0
    PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN); // CONFIGURA O CONTADOR COMO DECRESCENTE
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load); // SETA O CONTADOR

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000); // DIVIDE-SE O CONTADOR POR 1000 E FAZ A MULTIPLICACAO PARA AJUSTE PARA 1.51ms (MEIO DA POSICAO DO SERVO)
    PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true); // CONFIGURA PWM Module 1 COMO SAIDA
    PWMGenEnable(PWM1_BASE, PWM_GEN_0); // HABILITA GERACAO DE PWM

    while(1)
    {

        if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) // VERIFICA SE O PINO PF4 FOI PRESSIONADO
        {
            ui8Adjust--;
            if (ui8Adjust < 56)
            {
                ui8Adjust = 56; // TRAVA NO LIMITE DE 1mS
            }
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000); // SETA O DUTY CYCLE DO PWM
        }

        if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)  // VERIFICA SE O PINO PF0 FOI PRESSIONADO
        {
            ui8Adjust++;
            if (ui8Adjust > 111)
            {
                ui8Adjust = 111; // TRAVA NO LIMITE DE 2mS
            }
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000); // SETA O DUTY CYCLE DO PWM
        }

        SysCtlDelay(100000); // SETA A VELOCIDADE DA REPETICAO SOMENTE
    }

}
