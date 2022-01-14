import sqlite3
import datetime
ht_db = '/var/jail/home/rwagh/lab06/lab06.db' #assumes you have a lab06b dir in your home dir
now = datetime.datetime.now()

def request_handler(request):
    if request["method"]=="POST":
        user = request['form']['user']
        temp = request['form']['temperature']
        press = request['form']['pressure']
        time = datetime.datetime.now()
        with sqlite3.connect(ht_db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS sensor_data (time_ timestamp, user text, temperature real, pressure real);""")
            c.execute("""INSERT into sensor_data VALUES (?,?,?,?);""",(time,user,temp, press))
        return "Data POSTED successfully"
    else:
        return "invalid HTTP method for this url."