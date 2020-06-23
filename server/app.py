from flask import Flask, render_template, request
from threading import Thread
from modify_config import MConfig
from mbit_connector import MConnect

app = Flask(__name__)
mconf = MConfig()
mconnect = MConnect(mconf)
Thread(target=mconnect.runServer).start()

@app.route("/")
def hello():
    return render_template("new.html", mconf = mconf)

@app.route('/delete_usernames', methods=['POST'])
def delete_usernames():
    usernames_to_delete = request.form.getlist('usernames_delete')
    mconf.deleteUsernames(usernames_to_delete)
    return hello()

@app.route('/add_username', methods=['POST'])
def add_username():
    username_to_add = request.form.get('username_add')
    mconf.addUsername(username_to_add)
    return hello()

@app.route('/delete_hashtags', methods=['POST'])
def delete_hashtags():
    hashtags_to_delete = request.form.getlist('hashtags_delete')
    mconf.deleteHashtags(hashtags_to_delete)
    return hello()


@app.route('/add_hashtag', methods=['POST'])
def add_hashtag():
    hashtag_to_add = request.form.get('hashtag_add')
    mconf.addHashtag(hashtag_to_add)
    return hello()


@app.route('/delete_wlocations', methods=['POST'])
def delete_wlocations():
    wlocations_to_delete = request.form.getlist('wlocation_delete')
    mconf.deleteWeatherLocations(wlocations_to_delete)
    return hello()


@app.route('/add_wlocation', methods=['POST'])
def add_wlocation():
    wlocation_to_add = request.form.get('wlocation_add')
    mconf.addWeatherLocation(wlocation_to_add)
    return hello()


@app.route('/delete_pmessages', methods=['POST'])
def delete_pmessages():
    pmessages_to_delete = request.form.getlist('pmessage_delete')
    mconf.deletePresetMessages(pmessages_to_delete)
    return hello()


@app.route('/add_pmessages', methods=['POST'])
def add_pmessages():
    pmessages_to_add = request.form.get('pmessages_add')
    mconf.addPresetMessages(pmessages_to_add)
    return hello()


@app.route('/select_league', methods=['POST'])
def select_league():

    league_to_select = request.form.get('league')
    print(league_to_select)
    mconf.selectSportsLeague(league_to_select)
    return hello()

@app.route('/update_config', methods=['POST'])
def update_config():
    mconf.flagUpdateReq()
    return hello()
