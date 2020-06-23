import random
from twitter_scraper import get_tweets
from subprocess import run
import serial
import requests
import json
import sys
import time
from threading import Thread
# sys.path.append('gapp')
from modify_config import MConfig

class MBit:
    def __init__(self):
        self._tty = False
        self._serial = False
        # self.connect()

    def send(self, msg, a, leng):
        if a == 3:
            self._serial.write(bytes([3]))
            self._serial.write(bytes([leng]))
            self._serial.write(msg)
        elif a == 5:
            self._serial.write(bytes([5]))
            self._serial.write(bytes([leng]))
            self._serial.write(msg)
        elif a == 7:
            self._serial.write(bytes([7]))
            self._serial.write(bytes([leng]))
            self._serial.write(msg)
        elif a == 8:
            self._serial.write(bytes([8]))
            #self._serial.write(0x0f)
            self._serial.write(msg)
            print('sending 8')
        elif a == 1:
            self._serial.write(bytes([1]))
            #self._serial.write(bytes([leng]))
            self._serial.write(msg)


    def readline(self):
        x = []
        while True:
            c = self._serial.read()
            if c[0] == 0x0f:
                break
            x.append(c)
        #print(x)
        return x

    def connect(self):
        attempts = 1
        connected = False
        while (not connected):
            ttys = []
            dmesg = run("dmesg", shell=True,
                        capture_output=True).stdout.decode("utf-8")
            for l in dmesg.split("\n"):
                if "tty" in l and "USB" in l:
                    tty = l.split("tty", 1)[1].split(":")[0]
                    tty = "/dev/tty{}".format(tty)
                    if tty not in ttys:
                        ttys.append(tty)

            for tty in ttys:
                qstring = "udevadm info -q all -n {}".format(tty)
                udevadm = run(
                    qstring,
                    shell=True, capture_output=True).stdout.decode("utf-8")
                if "ARM mbed" in udevadm:
                    self._tty = tty
                    print(serial.Serial)
                    self._serial = serial.Serial(self._tty, 115200)
                    print("Device detected")
                    connected = True

            print("Device not detected, attempt", attempts)
            attempts = attempts+1
            time.sleep(1)

    def disconnect(self):
        self._serial.close()


    def sendtweet(self, user):
        tt = []
        for t in get_tweets(user, pages=2):
            tt.append(t)
        t = random.choice(tt)["text"][:50]
        v = random.choice(tt)["text"][:50]
        u = random.choice(tt)["text"][:50]
        message = t + " " + u + "  " + v
        print("GOT TWEETS: " + message)
        leng = len(message)
        load = bytearray(message, "utf-8")
        load.append(0x0f)
        self.send(load, 3, leng)

    def sendweather(self, place):
        url = 'http://api.openweathermap.org/data/2.5/weather?q={}&units=metric&appid=3ef30a2b3d836feffd03ea7cba4a859c'
        r = requests.get(url.format("place")).json()
        tempeer = r['main']['temp']
        weather = r['weather'][0]['description']
        payload = weather +  " " + str(tempeer) + " degrees"
        size = len(payload)
        load = bytearray(payload, "utf-8")
        load.append(0x0f)
        self.send(load, 5, size)

    def sendscores(self, league):
        r = requests.get('https://apifootball.com/api/?action=get_events&from=' + week_ago + '&to=' +  today + '&league_id=' + league + '&APIkey=c2d84aa3c77e0f4b4c858b63a15660f4449687d0f017f028b28e304bcac7aa38').json()
        for key in r:
            hometeam = r[i]['match_hometeam_name']
            hometeam_score = r[i]['match_hometeam_score']
            awayteam = r[i]['match_awayteam_name']
            awayteam_score = r[i]['match_awayteam_score']
            payload = payload + '       '  + hometeam + ' ' + hometeam_score + ' - ' + awayteam_score + ' ' + awayteam
        size = len(payload)
        load = bytearray(payload, "utf-8")
        load.append(0x0f)
        self.send(load, 7, size)

    def sendConfig(self, mc):
        users = mc.username_list
        hashtags = mc.hashtag_list
        locations = mc.weatherLocation_list
        messages = mc.presetMsg_list

        config = str(len(users))
        for x in users:
            config += str(len(x)) + x
        config += str(len(hashtags))
        for x in hashtags:
            config += str(len(x)) + x
        config += str(len(locations))
        for x in locations:
            config += str(len(x)) + x
        config += str(len(messages))
        for x in messages:
            config += str(len(x)) + x
        size = len(config)
        load = bytearray(config, "utf-8")
        load.append(0x0f)
        print(load)
        self.send(load, 1 , size)


class MConnect:

    def __init__(self, mconf):
        self.m = MBit()
        self.mc = mconf

    def extractString(self, r):
        username = ""
        for x in r:
            username += x.decode('utf-8')
        return username

    def helloMbit(self):
        while 1:
            time.sleep(1)
            self.m._serial.write(b"\x09")
            self.m._serial.write(b"\x0f")

    def checkConfig(self):
        while 1:
            time.sleep(3)
            self.checkForUpdate()

    def byteListConcat(l):
        s = ''
        for x in l:
            t = str(x)
            s = s + t[2]
        return s


    def configJson(self, c):
        #sent as list need to fix this
        print('configuring json')

        self.mc.username_list = []
        self.mc.hashtag_list = []
        self.mc.weatherLocation_list = []
        self.mc.presetMsg_list = []

        usernames = ord(c[1])
        pos = 2
        i = 1
        while (i <= usernames):
            usernameLen = ord(c[pos])
            endpos = pos+1 + usernameLen
            username = c[(pos +1):(endpos)]
            username = byteListConcat(username)
            self.mc.username_list.append(username)
            pos = endpos
            i = i + 1
        hastags = ord(c[pos])
        pos = pos + 1
        i = 1
        while (i <= hastags):
            hashtagLen = ord(c[pos])
            endpos = pos+1 + hashtagLen
            hashtag = c[(pos +1):(endpos)]
            hashtag = byteListConcat(hashtag)
            self.mc.hashtag_list.append(hashtag)
            pos = endpos
            i = i + 1
        locations = ord(c[pos])
        pos = pos + 1
        i = 1
        while (i <= locations):
            locationLen = ord(c[pos])
            endpos = pos+1 + locationLen
            location = c[(pos +1):(endpos)]
            location = byteListConcat(location)
            self.mc.weatherLocation_list.append(location)
            pos = endpos
            i = i + 1
        messages = ord(c[pos])
        pos = pos + 1
        i = 1
        while (i <= messages):
            messageLen = ord(c[pos])
            endpos = pos+1 + messageLen
            message = c[(pos +1):(endpos)]
            message = byteListConcat(message)
            self.mc.presetMsg_list.append(message)
            pos = endpos
            i = i + 1
        self.mc.updateJSON()

    def checkForUpdate(self):
        with open('updateFlag.json', 'r') as f:
            json_data = json.load(f)
            update = json_data["update"]
            if(update[0] == "y"):
                self.m.sendConfig(self.mc)
                with open('updateFlag.json', 'w') as w:
                    json.dump({"update": "n"}, fp = w)

    def runServer(self):
        # attempt to connect
        self.m.connect()
        # once connected, say hello
        #self.m._serial.write(b"\x08")
        #self.m._serial.write(b"\x0f")
        Thread(target=self.helloMbit).start()
        #Thread(target=self.checkConfig).start()
        while True: # run server read microbit
            r = self.m.readline()
            print(r)
            pType = int.from_bytes(r[0], 'big')
            if pType == 0x02:
                # Handle twitter
                reqType = int.from_bytes(r[1], 'big')
                contentLen = int.from_bytes(r[2], 'big')
                content = extractString(r[3:(3+contentLen)])
                if reqType == 0x00:
                    print(f"User request: {content}")
                    self.m.sendtweet(content)
                elif reqType == 0x01:
                    print(f"Hashtag request: {content}")
            elif pType == 0x04:
                # Handle weather
                contentLen = int.from_bytes(r[1], 'big')
                content = extractString(r[2:(2+contentLen)])
                print(f"Weather request: {content}")
                self.m.sendweather(content)
            elif pType == 0x06:
                # Handle sports
                print("Sports request")
                for x in mc.sportsleague_list:
                    self.m.sendscores(x)
            elif pType == 0x09:
                configJson(r)

# mconnect = MConnect()
# Thread(target=mconnect.helloMbit).start()
# Thread(target=mconnect.runServer).start()
