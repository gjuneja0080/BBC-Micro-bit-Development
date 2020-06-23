import json
import requests

class MConfig:

    def __init__(self):
        self.loadFromFile()
        self.initValues()

    def loadFromFile(self):
        with open('config.json', 'r') as f:
            self.json_data = json.load(f)

    def initValues(self):
        self.username_list = self.json_data["usernames"]
        self.hashtag_list = self.json_data["hashtags"]
        self.weatherLocation_list = self.json_data["weatherLocations"]
        self.sportsleague_list = self.json_data["league"]
        self.presetMsg_list = self.json_data["presetMessages"]
        self.u_list_len = len(self.username_list)
        self.h_list_len = len(self.hashtag_list)
        self.wl_list_len = len(self.weatherLocation_list)
        self.pm_list_len = len(self.presetMsg_list)

    def updateJSON(self):
        with open('config.json', 'w') as f:
            json.dump({"usernames": self.username_list, "hashtags": self.hashtag_list, "weatherLocations": self.weatherLocation_list,
                "league": self.sportsleague_list, "presetMessages": self.presetMsg_list}, fp=f)

    def deleteUsernames(self, usernames_to_delete):
        for username in usernames_to_delete:
            self.username_list.remove(username)
        self.updateJSON()

    def deleteHashtags(self, hashtags_to_delete):
        for hashtag in hashtags_to_delete:
            self.hashtag_list.remove(hashtag)
        self.updateJSON()

    def deleteWeatherLocations(self, wlocations_to_delete):
        for wlocation in wlocations_to_delete:
            print(wlocation)
            self.weatherLocation_list.remove(wlocation)
        self.updateJSON()

    def addPresetMessages(self, message):
        if(message != ''): 
            if (len(message) <= 32):
                if(len(self.presetMsg_list) <5):
                    self.presetMsg_list.append(message)
                    self.updateJSON()

    def deletePresetMessages(self, pmessages_to_delete):
        for message in pmessages_to_delete[:]:
            #print(message)
            self.presetMsg_list.remove(message)
        self.updateJSON()

    def addUsername(self, username):
        if(username != ''):
            if (len(username) <= 32):
                if(len(self.username_list) < 5):
                    self.username_list.append(username)
                    self.updateJSON()


    def addHashtag(self, hashtag):
        if(hashtag != ''):
            if (len(hashtag) <= 32):
                if(len(self.hashtag_list) < 5):
                    self.hashtag_list.append(hashtag)
                    self.updateJSON()

    def addWeatherLocation(self, wlocation):
        if(wlocation != ''):
            if (len(wlocation) <= 32):
                    if(len(self.weatherLocation_list) < 5):
                        url = 'http://api.openweathermap.org/data/2.5/weather?q=' +wlocation + '&units=metric&appid=3ef30a2b3d836feffd03ea7cba4a859c'
                        r = requests.get(url).json()
                        if(r['cod'] != '404'):
                            self.weatherLocation_list.append(wlocation)
                            self.updateJSON()

    def selectSportsLeague(self, league):
        self.sportsleague_list = []
        if(league == "Ligue"):
            self.sportsleague_list.append('128')
        else:
              self.sportsleague_list.append('63')
        self.updateJSON()
        self.printAllValues()

    def printAllValues(self):
        print(self.username_list)
        print(self.hashtag_list)
        print(self.weatherLocation_list)
        print(self.sportsleague_list)
        print(self.presetMsg_list)

    def flagUpdateReq(self):
        with open('updateFlag.json', 'w') as f:
            json.dump({"update": "y"}, fp = f)
