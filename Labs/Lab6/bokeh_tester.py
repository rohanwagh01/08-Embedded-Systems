import sqlite3
import datetime
from bokeh.plotting import figure,show
import os
os.environ["BOKEH_RESOURCES"] = 'inline'
#script is meant for local development and experimentation with bokeh
ht_db = 'example.db' #database has table called sensor_data with entries: time_ timestamp, user text, temperature real, pressure real
USERS = ["dog","cat"] # the two users in this database are users called "dog" and "cat"
now = datetime.datetime.now()
conn = sqlite3.connect(ht_db)
c = conn.cursor()
dog_data = c.execute('''SELECT * FROM sensor_data WHERE user = ? ORDER by time_ ASC;''',("dog",)).fetchall()
cat_data = c.execute('''SELECT * FROM sensor_data WHERE user = ? ORDER by time_ ASC;''',("cat",)).fetchall()
#make sure the plots have titles, x labels, y labels and each user is a different color with legend annotation
dog_time = []
dog_temp = []
dog_press = []
for step in dog_data:
    dog_time.append(datetime.datetime.strptime(step[0],'%Y-%m-%d %H:%M:%S.%f'))
    dog_temp.append(step[2])
    dog_press.append(step[3])
cat_time = []
cat_temp = []
cat_press = []
for step in cat_data:
    cat_time.append(datetime.datetime.strptime(step[0],'%Y-%m-%d %H:%M:%S.%f'))
    cat_temp.append(step[2])
    cat_press.append(step[3])
#now plot
temp_plot = figure(title="Temperature", x_axis_label='Time', y_axis_label='Temperature (C)', x_axis_type='datetime')

temp_plot.line(dog_time, dog_temp, legend_label="Dog", line_color="yellow")
temp_plot.line(cat_time, cat_temp, legend_label="Cat", line_color="blue")

press_plot = figure(title="Pressure", x_axis_label='Time', y_axis_label='Pressure (hPa)', x_axis_type='datetime')

press_plot.line(dog_time, dog_press, legend_label="Dog", line_color="yellow")
press_plot.line(cat_time, cat_press, legend_label="Cat", line_color="blue")

#either show plot1 or plot2
show(temp_plot)
show(press_plot)