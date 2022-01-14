import sqlite3
import random
import string
import sqlite3
import datetime
visits_db = '/var/jail/home/rwagh/lab04b/time_example.db'

#DATABASE CODE

def bounding_box(point_coord,box):
    x_points = [coord[0] for coord in box]
    y_points = [coord[1] for coord in box]
    x_max = max(x_points)
    y_max = max(y_points)
    x_min = min(x_points)
    y_min = min(y_points)
    if point_coord[0] < x_max and point_coord[0] > x_min and point_coord[1] < y_max and point_coord[1] > y_min:
        return True
    else:
        return False

def translate(point, origin):
    return (point[0] - origin[0], point[1] - origin[1])

def within_area(point_coord,poly):
    # normalize to the point at origin
    x_delta = -point_coord[0]
    y_delta = -point_coord[1]
    poly_norm = [(coord[0]+x_delta,coord[1]+y_delta) for coord in poly]
    edge_crossings = 0
    #find p and see if between x_1 and x_2 and if positive
    for val_index in range(len(poly_norm)):
        p_1 = poly_norm[val_index]
        if val_index+1 == len(poly_norm):
            p_2 = poly_norm[0]
        else:
            p_2 = poly_norm[val_index+1]
        if not p_2[1] == p_1[1]:
            p = (p_1[0]*p_2[1]-p_1[1]*p_2[0])/(p_2[1]-p_1[1])
            #check
            if min(p_1[0],p_2[0])<=p<=max(p_1[0],p_2[0]) and min(p_1[1],p_2[1])<=0<=max(p_1[1],p_2[1]) and p >= 0:
                edge_crossings += 1
    if edge_crossings%2 == 0:
        return False
    else:
        return True

locations={
    "Student Center":[(-71.095863,42.357307),(-71.097730,42.359075),(-71.095102,42.360295),(-71.093900,42.359340),(-71.093289,42.358306)],
    "Dorm Row":[(-71.093117,42.358147),(-71.092559,42.357069),(-71.102987,42.353866),(-71.106292,42.353517)],
    "Simmons/Briggs":[(-71.097859,42.359035),(-71.095928,42.357243),(-71.106356,42.353580),(-71.108159,42.354468)],
    "Boston FSILG (West)":[(-71.124664,42.353342),(-71.125737,42.344906),(-71.092478,42.348014),(-71.092607,42.350266)],
    "Boston FSILG (East)":[(-71.092409,42.351392),(-71.090842,42.343589),(-71.080478,42.350900),(-71.081766,42.353771)],
    "Stata/North Court":[(-71.091636,42.361802),(-71.090950,42.360811),(-71.088353,42.361112),(-71.088267,42.362476),(-71.089769,42.362618)],
    "East Campus":[(-71.089426,42.358306),(-71.090885,42.360716),(-71.088310,42.361017),(-71.087130,42.359162)],
    "Vassar Academic Buildings":[(-71.094973,42.360359),(-71.091776,42.361770),(-71.090928,42.360636),(-71.094040,42.359574)],
    "Infinite Corridor/Killian":[(-71.093932,42.359542),(-71.092259,42.357180),(-71.089619,42.358274),(-71.090928,42.360541)],
    "Kendall Square":[(-71.088117,42.364188),(-71.088225,42.361112),(-71.082774,42.362032)],
    "Sloan/Media Lab":[(-71.088203,42.361017),(-71.087044,42.359178),(-71.080071,42.361619),(-71.082796,42.361905)],
    "North Campus":[(-71.11022,42.355325),(-71.101280,42.363934),(-71.089950,42.362666),(-71.108361,42.354484)],
    "Technology Square":[(-71.093610,42.363157),(-71.092130,42.365837),(-71.088182,42.364188),(-71.088267,42.362650)]
}


def sign(x):
    if x > 0:
        return 1
    elif x == 0:
        return 0
    else:
        return -1

def get_area(point_coord,locations):
    # go through locations
    # return if in one of them
    for place in locations.keys():
        if within_area(point_coord, locations[place]):
            return place
    return "Off Campus"



def request_handler(request):
    if request["method"] == "GET":
        lat = 0
        lon = 0
        try:
            lat = float(request['values']['lat'])
            lon = float(request['values']['lon'])
        except Exception as e:
            # return e here or use your own custom return message for error catch
            #be careful just copy-pasting the try except as it stands since it will catch *all* Exceptions not just ones related to number conversion.
            return "Error: lat, lon are missing or not numbers"

        return get_area((lon,lat), locations)
    else:
        lat = float(request['form']['lat'])
        lon = float(request['form']['lon'])
        user = str(request['form']['user'])
        loc = get_area((lon,lat), locations)
        conn = sqlite3.connect(visits_db)  #connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  #move cursor into database (allows us to execute commands)
        c.execute('''CREATE TABLE IF NOT EXISTS test_table (user text,latitude float, longitude float, location text, timing timestamp);''') # run a CREATE TABLE command
        thirty_seconds_ago = datetime.datetime.now()- datetime.timedelta(seconds = 30) #create time for fifteen minutes ago!
        c.execute('''INSERT into test_table VALUES (?,?,?,?,?);''', (user,lat,lon,loc,datetime.datetime.now()))
        things = c.execute('''SELECT user FROM test_table WHERE location = ? AND timing > ? ORDER BY timing DESC;''',(loc, thirty_seconds_ago)).fetchall()
        outs = ""
        for x in things:
            outs+=str(x[0])+"\n"
        conn.commit() #commit commands
        conn.close() #close connection to database
        return loc + "\n" + outs