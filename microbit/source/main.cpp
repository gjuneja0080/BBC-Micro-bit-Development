#include "includes.h"
#include "nrf_ecb.c"

bool isProxy = false;

// MicroBit Piano
int CHIP_ADDRESS = 0x0D << 1;
bool INITIALISED = 0;
int KEY_K0 = 0x100;
int KEY_K1 = 0x200;
int KEY_K2 = 0x400;
int KEY_K3 = 0x800;
int KEY_K4 = 0x1000;
int KEY_K5 = 0x2000;
int KEY_K6 = 0x4000;
int KEY_K7 = 0x8000;
int KEY_K8 = 0x01;
int KEY_K9 = 0x02;
int KEY_K10 = 0x04;
int KEY_K11 = 0x08;
int KEY_K12 = 0x10;
int KEY_K13 = 0x20;
int KEY_K14 = 0x40;
vector<int> keys = {KEY_K2, KEY_K3, KEY_K4, KEY_K5};

int keySensitivity = 1; //default is 8
int keyNoiseThreshold = 5;
int keyRegValue = 0x0000;

int main()
{
    uBit.init();
    uBit.display.setBrightness(255);
    uBit.radio.enable();
    uBit.radio.setFrequencyBand(7);

    uBit.serial.setRxBufferSize(180);
    uBit.serial.eventOn("\x0F");

    uBit.messageBus.listen(MICROBIT_ID_SERIAL, MICROBIT_SERIAL_EVT_DELIM_MATCH, handleSerialIn);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_HOLD, lock);
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, radioRecv);

    hello();

    initConfig();

    initMenu();

    // Create background task to constantly send heartbeats
    create_fiber(heartbeat);
    addBroadcast();

    // Wait loop to prevent program exiting
    while (1)
    {
        uBit.sleep(1000);
    }
}

void initConfig() {
    if (Config::fileExists()) {
        //uBit.display.scroll("exists");
        config = Config::readFromFile();
    } else {
        Config::writeDummyToFile();
        config = Config::readFromFile();
        // get from server
    }
}

void initMenu() {
    navigator.activate();
    auto *m = new Menu(&mainMenuHandler);
    m->addEntry(ManagedString("Contacts"));
    m->addEntry(ManagedString("Twitter"));
    m->addEntry(ManagedString("Weather"));
    m->addEntry(ManagedString("Sports"));
    m->addEntry(ManagedString("Piano"));
    navigator.transitTo(m);

    create_fiber(heartbeat);
    addBroadcast();

    while (1)
    {
        uBit.sleep(1000);
    }
}

void handleSerialIn(MicroBitEvent e)
{
    // Read packet until packet terminator symbol
    ManagedString packet = uBit.serial.readUntil("\x0F");
    // Packet type is the first character
    //uBit.display.scroll(packet, 55);
    char packetType = packet.charAt(0);

    if (packetType == 0x09)
    {
        // HELLO packet, this device is now a serial proxy device
        if (!isProxy)
        {
            isProxy = true;
            showText("^");
            navigator.deactivate();
        }

    }
    else if (packetType == 0x08)
    {
        // Request configuration
        // Send back configuration
        auto packet = ManagedString("\x09");
        packet = packet + ManagedString((char) config->twitterUsernames.size());
        for (auto &i : config->twitterUsernames)
        {
            packet = packet + ManagedString((char)i.length());
            packet = packet + i;
        }
        packet = packet + ManagedString((char) config->twitterHashtags.size());
        for (auto &i : config->twitterHashtags)
        {
            packet = packet + ManagedString((char)i.length());
            packet = packet + i;
        }
        packet = packet + ManagedString((char) config->weatherLocations.size());
        for (auto &i : config->weatherLocations)
        {
            packet = packet + ManagedString((char)i.length());
            packet = packet + i;
        }
        packet = packet + ManagedString((char) config->presetMessages.size());
        for (auto &i : config->presetMessages)
        {
            packet = packet + ManagedString((char)i.length());
            packet = packet + i;
        }

        packet = packet + ManagedString("\x0F");

        uBit.serial.send(packet);

    }
    else if (packetType == 0x01)
    {
        uBit.display.scroll("GOT CONFIG", 55);
        int cursor = 2;
        // Set configuration
        int numOfHandles = packet.charAt(1);
        int a = numOfHandles - '0';

        config->twitterUsernames.clear();
        for (int i = 0; i < a; i++)
        {
            int len = packet.charAt(cursor);
            cursor++;
            int ai = len - '0';
            ManagedString str = packet.substring(cursor, ai);
            config->twitterUsernames.push_back(str);
            cursor += ai;
        }
        numOfHandles = packet.charAt(cursor);
        a = numOfHandles - '0';
        cursor++;
        config->twitterHashtags.clear();
        for (int i = 0; i < a; i++)
        {
            int len = packet.charAt(cursor);
            cursor++;
            int ai = len - '0';
            ManagedString str = packet.substring(cursor, ai);
            config->twitterHashtags.push_back(str);
            cursor += ai;
        }
        numOfHandles = packet.charAt(cursor);
        a = numOfHandles - '0';
        cursor++;
        config->weatherLocations.clear();
        for (int i = 0; i < a; i++)
        {
            int len = packet.charAt(cursor);
            cursor++;
            int ai = len - '0';
            ManagedString str = packet.substring(cursor, ai);
            config->weatherLocations.push_back(str);
            cursor += ai;
        }
        numOfHandles = packet.charAt(cursor);
        a = numOfHandles - '0';
        cursor++;
        config->presetMessages.clear();
        for (int i = 0; i < a; i++)
        {
            int len = packet.charAt(cursor);
            cursor++;
            int ai = len - '0';
            ManagedString str = packet.substring(cursor, ai);
            config->presetMessages.push_back(str);
            cursor += ai;
        }
        // erase all memory, then save new configuration and call a restart
        Config::eraseAllMemory();
        Config::writeToFile(config);
        uBit.reset();
        // update file
    }
    else
    {
        // send the server response back
        intraSerialReturn(packet);
    }
}

void handlePacket(ManagedString packet)
{
    uBit.display.scroll("REC");
    auto packetType = packet.charAt(0);
    if (packetType == 0x03)
    {
        // Twitter message
        auto numOfTweets = (unsigned int)packet.charAt(1);
        auto tweets = vector<ManagedString>();
        auto cursor = 2;
        for (int i = 0; i < numOfTweets; i++)
        {
            auto len = (unsigned int)packet.charAt(cursor);
            cursor++;
            auto str = packet.substring(cursor, len);
            tweets.push_back(str);
            cursor += len;
        }
        showList(tweets);
    }
    else if (packetType == 0x05)
    {
        // Weather
        auto len = (unsigned int)packet.charAt(1);
        auto str = packet.substring(2, len);
        showText(str);
    }
    else if (packetType == 0x07)
    {
        // Sports
        auto len = (unsigned int)packet.charAt(1);
        auto str = packet.substring(2, len);
        showText(str);
    }
}


void showList(vector<ManagedString> tweets)
{
    auto *m = new Menu(NULL);
    for (auto &i : tweets)
        m->addEntry(i);
    navigator.transitTo(m);
}

void mainMenuHandler(vector<ManagedString> entries, int entryNum)
{
    if (entries.at(entryNum) == ManagedString("Twitter"))
    {
        auto *m = new Menu(&twitterMenuHandler);
        m->addEntry(ManagedString("@"));
        m->addEntry(ManagedString("#"));
        navigator.transitTo(m);
    }
    else if (entries.at(entryNum) == ManagedString("Weather"))
    {
        auto *m = new Menu(&weatherMenuHandler);
        for (auto &i : config->weatherLocations)
            m->addEntry(i);
        navigator.transitTo(m);
    }
    else if (entries.at(entryNum) == ManagedString("Sports"))
    {
        auto packet = ManagedString("\x06\x0F");
        intraSerialSend(packet);
    }
    else if (entries.at(entryNum) == ManagedString("Contacts"))
    {
        auto *m = new Menu(&contactsMenuHandler);
        for (auto &i : contacts)
            m->addEntry(i.alias);
        navigator.transitTo(m);
    }
    else if (entries.at(entryNum) == ManagedString("Piano"))
   {
       auto *m = new Menu(&pianoHandler);
       m->addEntry(ManagedString("Play Piano"));
       m->addEntry(ManagedString("Twinkle Star"));
       m->addEntry(ManagedString("Despacito"));
       m->addEntry(ManagedString("RepeatME"));
       navigator.transitTo(m);
   }
}

auto sendTo = ManagedString("");
void contactsMenuHandler(vector<ManagedString> entries, int entryNum)
{
    sendTo = contacts.at(entryNum).origin;
    auto *m = new Menu(&sendMenuHandler);
    for (auto &i : config->presetMessages)
        m->addEntry(i);
    navigator.transitTo(m);
}

void sendMenuHandler(vector<ManagedString> entries, int entryNum)
{
    auto payload = ManagedString();
    payload = payload + sendTo;
    int key = 0;
    for (auto &cr : contacts)
    {
        if (cr.origin == sendTo)
        {
            key = cr.key;
            break;
        }
    }
    if (key == 0)
    {
        auto encrypted = entries.at(entryNum);
        auto len = ManagedString(encrypted.length());

        payload = payload + len;
        payload = payload + encrypted;

        sendPacket(1, payload);
    }
    else
    {
        auto encrypted = aes(entries.at(entryNum), key);
        auto len = ManagedString(encrypted.length());

        payload = payload + len;
        payload = payload + encrypted;
        // uBit.display.scroll(key);
        // uBit.display.scroll(len);

        sendPacket(1, payload);
    }
}

ManagedString aes(ManagedString message, int key) {

    nrf_ecb_init();
    //initialize the key/counter buffers
    uint8_t* keyBuf = new uint8_t[16] {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t* counterBuf = new uint8_t[16] {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t* keyStr = new uint8_t[16] {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //set the last byte of the keybuffer to the shared key
    keyBuf[15] = (uint8_t) key;
    nrf_ecb_set_key(keyBuf);
    //create a buffer for the encoded message
    char* encMessage = new char[message.length()];

    nrf_ecb_crypt(keyStr, counterBuf);

    for (int i = 0; i < message.length(); i++) {
        //every 16 bytes, increment the last value of the counter buffer then generate
        //a new key to be used to xor the next 16 byte block
        if (i != 0 && i % 16 == 0) {
            counterBuf[15]++;
            nrf_ecb_crypt(keyStr, counterBuf);
        }
        //do the xor
        encMessage[i] = message.charAt(i) ^ keyStr[(i % 16)];
    }
    //convert to ManagedString
    ManagedString em(encMessage);
    delete[] keyBuf;
    delete[] counterBuf;
    delete[] keyStr;
    delete[] encMessage;

    return em;
}


void twitterMenuHandler(vector<ManagedString> entries, int entryNum)
{
    if (entries.at(entryNum) == ManagedString("@"))
    {
        auto *m = new Menu(&twitterUserMenuHandler);
        for (auto &i : config->twitterUsernames)
            m->addEntry(i);
        navigator.transitTo(m);
    }
    else if (entries.at(entryNum) == ManagedString("#"))
    {
        auto *m = new Menu(&twitterHashtagMenuHandler);
        for (auto &i : config->twitterHashtags)
            m->addEntry(i);
        navigator.transitTo(m);
    }
}
void weatherMenuHandler(vector<ManagedString> entries, int entryNum)
{
    auto packet = ManagedString("\x04");
    auto name = entries.at(entryNum);
    packet = packet + ManagedString((char)name.length());
    packet = packet + name;
    packet = packet + ManagedString("\x0F");

    intraSerialSend(packet);
}
void twitterUserMenuHandler(vector<ManagedString> entries, int entryNum)
{
    auto packet = ManagedString("\x02");
    auto tType = PacketBuffer(1);
    tType[0] = 0;
    packet = packet + tType;
    auto name = entries.at(entryNum);
    packet = packet + ManagedString((char)name.length());
    packet = packet + name;
    packet = packet + ManagedString("\x0F");
    intraSerialSend(packet);
}
void twitterHashtagMenuHandler(vector<ManagedString> entries, int entryNum)
{
    auto packet = ManagedString("\x02\x01");
    auto name = entries.at(entryNum);
    packet = packet + ManagedString((char)name.length());
    packet = packet + name;
    packet = packet + ManagedString("\x0F");

    intraSerialSend(packet);
}

void startKeyboard()
{
    initPiano();
    int pressed;
    //uBit.display.scroll("Free Play");
    int volume = 2;
    while (1)
    {
        pressed = readKeyPress();
        //while ((pressed = readKeyPress()) == 0)
        //    ;
        playKeyPress(pressed, volume, 300);
        uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, exitPiano);
    }
}

void playRandomTune()
{
    initPiano();
    vector<int> randomNotes(5, 0);
    vector<int> usrInput(5, 0);
    int volume = 2;
    int duration = 400;
    int score = 0;

    for (int i = 0; i < 5; i++)
    {
        randomNotes[i] = keys.at(uBit.random(keys.size() - 1));
        playKeyPress(randomNotes[i], volume, duration);
        uBit.sleep(100);
    }

    uBit.sleep(1000);
    //playKeyPress(KEY_K11, volume, 300);
    showText("Your turn!");

    int input = 0;

    while (input < 5)
    {
        int pressed = 0;
        pressed = readKeyPress();
        //while ((pressed = readKeyPress()) == 0)
        //    ;
        if (pressed != 0 && pressed != KEY_K9)
        {
            usrInput[input] = pressed;
            input++;
            playKeyPress(pressed, volume, 300);
        }

    }
    bool isMatch = std::equal(randomNotes.begin(), randomNotes.end(), usrInput.begin());
    if (isMatch)
    {
        //playKeyPress(KEY_K13, volume, duration);
        showText("You win!");
        score++;
    }
    else
    {
        //playKeyPress(KEY_K11, volume, duration);
        showText("Wrong tune played!");
        if (score == 0)
        {
            score = 0;
        }
        else
        {
            score--;
        }
    }
}

void exitPiano(MicroBitEvent e)
{
    uBit.reset();
}

void playNote(int volume, int duration, int freq)
{
    uBit.io.P0.setAnalogValue(volume);
    uBit.io.P0.setAnalogPeriodUs(freq);
    uBit.sleep(duration);
    uBit.io.P0.setAnalogValue(0);
    uBit.sleep(80);
}

void playTwinkle()
{
    initPiano();
    int volume = 5;
    int duration = 300;
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, exitPiano);

    playKeyPress(KEY_K9, volume, duration);
    playKeyPress(KEY_K9, volume, duration);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K14, volume, duration);
    playKeyPress(KEY_K14, volume, duration);
    playKeyPress(KEY_K13, volume, duration + 300);
    //uBit.sleep(200);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K9, volume, duration + 300);
    //uBit.sleep(200);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K10, volume, duration + 300);
    //uBit.sleep(200);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K10, volume, duration + 300);
    //uBit.sleep(200);
    playKeyPress(KEY_K9, volume, duration);
    playKeyPress(KEY_K9, volume, duration);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K13, volume, duration);
    playKeyPress(KEY_K14, volume, duration);
    playKeyPress(KEY_K14, volume, duration);
    playKeyPress(KEY_K13, volume, duration + 300);
    //uBit.sleep(200);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K12, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K9, volume, duration + 300);
    uBit.display.stopAnimation();
}

void playDesp()
{
    initPiano();
    int volume = 5;
    int duration = 100;
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, exitPiano);

    playKeyPress(KEY_K10, volume, duration + 400);
    playKeyPress(KEY_K1, volume, duration + 400);
    playNote(volume, duration + 100, 4049);
    playNote(volume, duration + 100, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration + 100, 4049);
    playNote(volume, duration, 4545);
    playNote(volume, duration + 100, 4049);
    playNote(volume, duration + 200, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration + 100, 4049);
    playKeyPress(KEY_K1, volume, duration);
    playKeyPress(KEY_K10, volume, duration + 100);
    playNote(volume, duration + 200, 4545);
    playNote(volume, duration, 4545);
    playNote(volume, duration, 4545);
    playNote(volume, duration, 4545);
    playNote(volume, duration, 4545);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration + 100);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K11, volume, duration + 100);
    playKeyPress(KEY_K1, volume, duration + 400);
    playKeyPress(KEY_K10, volume, duration + 400);
    playKeyPress(KEY_K1, volume, duration + 400);
    playNote(volume, duration + 100, 4049);
    playNote(volume, duration + 100, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 5405);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration + 100, 4049);
    playNote(volume, duration, 4545);
    playNote(volume, duration + 100, 4049);
    playNote(volume, duration + 200, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 5102);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration, 4049);
    playNote(volume, duration + 100, 4049);
    playKeyPress(KEY_K1, volume, duration);
    playKeyPress(KEY_K10, volume, duration + 100);
    playNote(volume, duration + 200, 4545);
    playNote(volume, duration, 4545);
    playNote(volume, duration, 4545);
    playNote(volume, duration, 4545);
    playNote(volume, duration, 4545);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration);
    playKeyPress(KEY_K10, volume, duration + 100);
    playKeyPress(KEY_K11, volume, duration);
    playKeyPress(KEY_K11, volume, duration + 100);
    playKeyPress(KEY_K1, volume, duration + 400);
    uBit.display.stopAnimation();
}

void playKeyPress(int pressed, int volume, int duration)
{
    // All keys have been hardcoded with the correct number for their corresponding pitch
    if (pressed == KEY_K1) //c#4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(3608);
        showNote("C", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K2) //d#4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(3214);
        showNote("D", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K3) //f#4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(2703);
        showNote("F", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K4) //g#4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(2408);
        showNote("G", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K5) //a#4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(2145);
        showNote("A", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K6) //b4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(2025);
        showNote("B", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K7) //c5
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(1911);
        showNote("C", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K9) //c4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(3822);
        showNote("C", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K10) //d4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(3405);
        showNote("D", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K11) //e4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(3034);
        showNote("E", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K12) //f4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(2863);
        showNote("F", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K13) //g4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(2551);
        showNote("G", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    else if (pressed == KEY_K14) //a4
    {
        uBit.io.P0.setAnalogValue(volume);
        uBit.io.P0.setAnalogPeriodUs(2273);
        showNote("A", duration);
        uBit.sleep(duration);
        uBit.io.P0.setAnalogValue(0);
    }
    uBit.sleep(80);
}

void pianoHandler(vector<ManagedString> entries, int entryNum)
{
    // Start the correct piano application based on
    switch (entryNum)
    {
    case 0:
        startKeyboard();
        break;
    case 1:
        playTwinkle();
        break;
    case 2:
        playDesp();
        break;
    case 3:
        playRandomTune();
        break;
    default:
        break;
    }
}

// Taken from the example code on studres
void initPiano()
{
    char buff[1] = {0};
    char buff2[2] = {0};
    char buff3[5] = {0};
    uBit.io.P1.setPull(PullUp);
    buff[0] = 0;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff, 1, false);
    while (buff[0] != 0x11)
    {
        uBit.i2c.read(CHIP_ADDRESS, buff, 1, false);
    }
    for (int sensitivityReg = 54; sensitivityReg < 69; sensitivityReg++)
    {
        buff2[0] = sensitivityReg;
        buff2[1] = keySensitivity;
        uBit.i2c.write(CHIP_ADDRESS, buff2, 0);
    }
    buff2[0] = 69;
    buff2[1] = 0;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);
    buff2[0] = 13;
    buff2[1] = keyNoiseThreshold;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);
    for (int aksReg = 22; aksReg < 37; aksReg++)
    {
        buff2[0] = aksReg;
        buff2[1] = 1;
        uBit.i2c.write(CHIP_ADDRESS, buff2, 2);
    }
    buff2[0] = 10;
    buff2[1] = 1;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);
    buff[0] = 2;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1);
    uBit.i2c.read(CHIP_ADDRESS, buff3, 5, 0);
    while (uBit.io.P1.getDigitalValue())
    {
        buff[0] = 2;
        uBit.i2c.write(CHIP_ADDRESS, buff, 1);
        uBit.i2c.read(CHIP_ADDRESS, buff3, 5, 0);
        INITIALISED = 1;
    }
}

int readKeyPress()
{
    char buff[1] = {0};
    char buff2[2] = {0};
    char buff3[5] = {0};
    buff[0] = 2;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff3, 5, false);
    //Address 3 is the addr for keys 0-7 (this will then auto move onto Address 4 for keys 8-15, both reads stored in buff2)
    buff[0] = 3;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff2, 2, false);
    //keyRegValue is a 4 byte number which shows which keys are pressed
    int keyRegValue = (buff2[1] + (buff2[0] * 256));

    return keyRegValue;
}


void lock(MicroBitEvent e)
{
    uBit.messageBus.ignore(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_HOLD, lock);
    navigator.deactivate();
    MicroBitImage locked("0,255,255,255, 0\n0,255,0,255,0\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n");
    uBit.display.stopAnimation();
    uBit.display.print(locked);
    while (1)
    {
        if (uBit.buttonB.isPressed() && !uBit.buttonA.isPressed())
        {
            uBit.sleep(500);
            if (uBit.buttonA.isPressed() && !uBit.buttonB.isPressed())
            {
                break;
            }
        }
    }
    hello();
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_HOLD, lock);
    navigator.activate();
}

void hello()
{
    const uint8_t smile_arr[]{
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        0,
        1,
        0,
        0,
        0,
        1,
        0,
        0,
        1,
        0,
        0,
        0,
        1,
        0,
        1,
        1,
        1,
        0,
    };

    const uint8_t wink_arr[]{
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        1,
        0,
        0,
        0,
        1,
        0,
        1,
        1,
        1,
        0,
    };
    MicroBitImage smile(5, 5, smile_arr);
    MicroBitImage wink(5, 5, wink_arr);
    uBit.display.print(wink);
    uBit.sleep(500);
    uBit.display.print(smile);
    uBit.sleep(500);
}
