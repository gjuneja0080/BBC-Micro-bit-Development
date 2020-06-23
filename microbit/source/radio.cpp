#include "includes.h"


const int BASE = 5;
const int SECRET = 6;
const int MOD = 23;
vector<Contact> contacts;
ManagedString relayTo = "";

void addBroadcast()
{
    Contact contact;
    contact.origin = ManagedString("0000000000");
    contact.group = 0;
    contact.alias = ManagedString("Broadcast");
    contact.key = 0;
    contacts.push_back(contact);
}

void heartbeat()
{
    auto payload = ManagedString();
    // Adds custom group protocol type 0x03 for group 3 (us)
    payload = payload + ManagedString("\x00");
    // Adds current serial as sender ID
    payload = payload + getSerial();

    payload = payload + ManagedString("\x03");
    payload = payload + ManagedString("A31");

    // Send packet repeatedly every second
    while (1)
    {
        // Send packet type 0 with payload heartbeat
        sendPacket(0, payload);
        //uBit.sleep(1000);
        sendKey();
        uBit.sleep(1000);
    }
}

void intraSerialSend(ManagedString s)
{

    auto payload = ManagedString("\x03");
    // Adds current serial as sender ID
    payload = payload + getSerial();
    // Add 0x00 packet to indicate send packet
    auto ts = PacketBuffer(1);
    ts[0] = 0;
    payload = payload + ts;
    payload = payload + s;
    showText("CS");
    uBit.radio.datagram.send(payload);
}

void intraSerialReturn(ManagedString s)
{
    auto payload = ManagedString("\x03");
    payload = payload + relayTo;
    //payload = payload + ManagedString("0000000000");
    payload = payload + ManagedString("\x03");
    payload = payload + s;
    showText("PS");
    uBit.radio.datagram.send(payload.substring(0, 180));
}

void sendPacket(char type, ManagedString payload)
{
    auto serial = getSerial();

    auto packet = PacketBuffer(12 + payload.length());
    packet[0] = 0;
    memcpy(packet.getBytes() + 1, serial.toCharArray(), 10);
    packet[11] = type;
    memcpy(packet.getBytes() + 12, payload.toCharArray(), payload.length());

    uBit.radio.datagram.send(packet);
}

void radioRecv(MicroBitEvent e)
{
    ManagedString s = uBit.radio.datagram.recv();
    auto protocol = (char)s.charAt(0);

    if (protocol == 3)
    {
        //showText("hurr durr");
        auto origin = s.substring(1, 10);
        // Extract packet type
       auto type = s.charAt(11);
       // Extract rest of packet
       auto rest = s.substring(12, s.length() - 12);
       if (type == 0 && isProxy)
       {
           // If type is 0 and we are proxy device, send over serial
           // Store the requesting microbit
           relayTo = origin;
           // Indicate proxy received a packet
           showText("PR");

           // Send packet over proxy
           uBit.serial.send(rest);
       }
       else if (type == 3)
       {
           // If packet type 3, is response packet
           // Indicate client received packet
           showText("CR");

           // Handle packet
           handlePacket(rest);
       }
   }
   else if (protocol == 0)
   {
       // Extract origin microbit ID
       auto origin = s.substring(1, 10);
       // Extract packet type
        auto type = (char)s.charAt(11);

        if (type == 0)
        {
            // Heartbeat
            auto group = (char)s.charAt(12);
            auto nameSize = (char)s.charAt(13);
            auto name = s.substring(14, nameSize);

            Contact contact;
            contact.origin = origin;
            contact.group = group;
            contact.alias = name;
            contact.key = -1;

            bool alreadyIn = false;
            for (auto &cr : contacts)
            {
                if (cr.origin == origin)
                {
                    alreadyIn = true;
                    break;
                }
            }
            if (!alreadyIn)
            {
                contacts.push_back(contact);
                // sendKey();
            }
        }
        else if (type == 4)
        {
            // Key Exchange
            auto num = (char)s.charAt(12);
            //int pow = num - '0';
            int key = calculateKey(num, SECRET, MOD);
            for (auto &cr : contacts)
            {
                if (cr.origin == origin)
                {
                    if (cr.key < 0)
                    {
                        cr.key = key;
                    }
                    break;
                }
            }
        }
        else if (type == 1)
        {
            // Message
            //uBit.display.scroll(s, 55);
            auto dest = s.substring(12, 10);
            auto serial = getSerial();

            if (dest == serial)
            {
                auto key = -1;
                for (auto &cr : contacts)
                {
                    if (cr.origin == origin)
                    {
                        key = cr.key;
                        break;
                    }
                }
                if (key != -1)
                {
                    auto length = (char)s.charAt(22);
                    auto payload = s.substring(23, length);
                    //uBit.display.scroll(length);
                    //ManagedString ting = payload.substring(0, payload.length()-1);
                    //uBit.display.scroll(key);
                    ManagedString decrypted = aes(payload, key);
                    showText(decrypted);
                } else {
                    showText("failed");
                }
            }
            // for broadcasts, which shouldnt be encrypted
            else if (dest == ManagedString("0000000000"))
            {
                uBit.display.scroll("Broadcast:", 55);
                auto length = (char)s.charAt(22);
                auto payload = s.substring(23, length);
                showText(payload);
            }
        }
    }
}

void sendKey()
{
    auto serial = uBit.getSerial();
    for (int i = 0; i < 10 - serial.length(); i++)
    {
        serial = serial + ManagedString("0");
    }
    auto packet = PacketBuffer(13);
    packet[0] = 0;
    memcpy(packet.getBytes() + 1, serial.toCharArray(), 10);
    packet[11] = 4;
    packet[12] = (uint8_t) calculateKey(BASE, SECRET, MOD);

    uBit.radio.datagram.send(packet);
}

//function to calculate the shared key for diffie hellman
int calculateKey(int base, int exp, int mod) {
    int result = (int) pow(base, exp) % mod;

    return result;
}
