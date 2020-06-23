#ifndef H_CONFIG
#define H_CONFIG

#include "includes.h"

class Config
{
public:
  vector<ManagedString> twitterUsernames;
  vector<ManagedString> twitterHashtags;
  vector<ManagedString> weatherLocations;
  vector<ManagedString> presetMessages;

  static Config *readFromFile();
  static void writeToFile(Config* config);
  static void writeDummyToFile();
  static bool fileExists();
  static Config *getFromFile();
  static void eraseAllMemory();
};

// class Pair
// {
//   public:
//   // 0 is user
//   // 1 is hashtag
//   // 2 is weather locations
//   // 3 is sports
//     char type;
//     ManagedString data;
// };

#endif
