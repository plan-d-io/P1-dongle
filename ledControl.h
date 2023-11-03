static const int ledState[8][6][3] PROGMEM = {
    {   //Continous blue
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x40 }
    },
    {   //Blue - blue - pause
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    },
    {   //Slow blink blue
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    },
    {   //Fast blink blue
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x40 },
        { 0x00, 0x00, 0x00 }
    },
    {   //Continous green
        { 0xff, 0x40, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0xff, 0x40, 0x00 }
    },
    {   //Green - green - pause
        { 0xff, 0x40, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    },
    {   //Slow blink green
        { 0xff, 0x40, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 }
    },
    {   //Fast blink green
        { 0xff, 0x40, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0x00, 0x00, 0x00 },
        { 0xff, 0x40, 0x00 },
        { 0x00, 0x00, 0x00 }
    },
};

elapsedMillis ledTime;
int ledi, unitState;
uint8_t DisBuff[2 + 5 * 5 * 3];

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata)
{
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++)
    {
        DisBuff[2 + i * 3 + 0] = Rdata;
        DisBuff[2 + i * 3 + 1] = Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

void blinkLed() {
/*
 * -1: booting or updating
 * 0: AP mode, meter detected, no errors (steady blue)
 * 1: AP mode, could not connect to wifi (blue-blue-pause)
 * 2: AP mode, meter not detected (slow blink blue)
 * 3: AP mode, critical error (fast blink blue)
 * 4: STA mode, meter detected, no errors (steady green)
 * 5: STA mode, could not reach remote host or upgrade repository (green-green-pause)
 * 6: STA mode, meter not detected (slow blink green)
 * 7: STA mode, critical error (fast blink green)
 */
  if(ledTime > 300){
    setBuff(ledState[unitState][ledi][0], ledState[unitState][ledi][1], ledState[unitState][ledi][2]);
    M5.dis.displaybuff(DisBuff);
    ledi++;
    if(ledi >= 6) ledi = 0;
    ledTime = 0;
 }
}
