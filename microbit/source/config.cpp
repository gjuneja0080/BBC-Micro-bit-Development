#include "includes.h"


Config *Config::getFromFile()
{
    // TODO Read from filesystem or create file, current populating with dummy info
    auto *config = new Config();
    config->twitterUsernames.push_back(ManagedString("elonmusk"));
    config->twitterUsernames.push_back(ManagedString("standrews"));
    config->twitterHashtags.push_back(ManagedString("compsci"));
    config->twitterHashtags.push_back(ManagedString("cplusplus"));
    config->weatherLocations.push_back(ManagedString("St Andrews"));
    config->weatherLocations.push_back(ManagedString("London"));
    config->presetMessages.push_back(ManagedString("Hello!"));
    config->presetMessages.push_back(ManagedString("Good bye!"));
    return config;
}

/**
 * Reads config from persistent storage
 */
Config *Config::readFromFile()
{
    // TODO Read from filesystem or create file, current populating with dummy info
    Config *localConfig = new Config();
    int MAX_VALUES = 5;
    vector<vector<ManagedString>> vectors = {
        localConfig->twitterUsernames,
        localConfig->twitterHashtags,
        localConfig->weatherLocations,
        localConfig->presetMessages
    };

    for (int i = 0; i < vectors.size(); i = i + 1) {
        for (int j = 0; j < MAX_VALUES; j++) {
            KeyValuePair* kvp = uBit.storage.get(ManagedString(i*MAX_VALUES + j));
            if (kvp != NULL) {
                PacketBuffer b(32);
                uint8_t* a = b.getBytes();
                memcpy(a, kvp->value, 32);
                ManagedString whole(b);
                //uBit.display.scroll(whole);
                int len = (whole.charAt(0) - '0') * 10 + whole.charAt(1) - '0';
                //int ai = len - '0';
                ManagedString value = whole.substring(2, len);
                switch (i) {
                    case 0: //usernames
                        localConfig->twitterUsernames.push_back(value);
                        break;
                    case 1: //hashtags
                        localConfig->twitterHashtags.push_back(value);
                        break;
                    case 2: //weather locaations
                        localConfig->weatherLocations.push_back(value);
                        break;
                    case 3: //sports
                        localConfig->presetMessages.push_back(value);
                        break;

                }
                delete kvp;
                //delete a;
            }
        }
    }

    return localConfig;
}

/**
 * Writes config to persistent storage
 */
void Config::writeToFile(Config* localConfig)
{
    int MAX_VALUES = 5; // Assigned 5 each since max size 21

    vector<vector<ManagedString>> vectors = {
        localConfig->twitterUsernames,
        localConfig->twitterHashtags,
        localConfig->weatherLocations,
        localConfig->presetMessages
    };

    /*
     * KVP STORAGE INDEX
     *
     * 0-4 Twitter usernames
     * 5-9 Twitter hashtags
     * 10-14 Weather locations
     * 15-19 Sports scores
     *
     */

    for (int i = 0; i < vectors.size(); i = i + 1) {
        for (int j = 0; j < vectors.at(i).size(); j = j + 1) {
            //auto size = ManagedString(vectors.at(i).at(j).length());
            int len = vectors.at(i).at(j).length();
            ManagedString full;
            if (len < 10) {
                full = ManagedString(0);
            }
            full = full + ManagedString(len) + vectors.at(i).at(j);
            // TODO: combine size and data into uint8_t array
            //uBit.display.scroll(full);
            uint8_t* data = (uint8_t*) full.toCharArray();
            //vectors.at(i).at(j).toCharArray();
            //(uint8_t *)&vectors.at(i).at(j)
            uBit.storage.put(ManagedString(i*MAX_VALUES + j), data, 32);
            //delete data;
        }
    }
}

/**
 * Function to erase all the key-value pairs in the flash memory
 */
void Config::eraseAllMemory()
{
    for (int i = 0; i < 20; i = i + 1) {
        if (uBit.storage.get(ManagedString(i)) != NULL) {
            uBit.storage.remove(ManagedString(i));
        }
    }
}

/**
 * Creates sample config, writes to storage.
 */
void Config::writeDummyToFile()
{
    Config* localConfig = new Config();

    localConfig->twitterUsernames.push_back(ManagedString("elonmusk"));
    localConfig->twitterUsernames.push_back(ManagedString("standrews"));

    localConfig->twitterHashtags.push_back(ManagedString("compsci"));
    localConfig->twitterHashtags.push_back(ManagedString("cplusplus"));

    localConfig->weatherLocations.push_back(ManagedString("St Andrews"));
    localConfig->weatherLocations.push_back(ManagedString("London"));

    localConfig->presetMessages.push_back(ManagedString("Hello!"));
    localConfig->presetMessages.push_back(ManagedString("Good bye!"));

    writeToFile(localConfig);
}

/**
 * Checks whether config exists in persistent storage
 */
bool Config::fileExists()
{
    for (int i = 0; i < 20; i = i + 1) {
        if (uBit.storage.get(ManagedString(i)) != NULL) {
            return true;
        }
    }
    return false;
}
