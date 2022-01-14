import sqlite3
from bokeh.plotting import figure
from bokeh.embed import components
import datetime
ht_db = '/var/jail/home/rwagh/lab06/lab06.db' #assumes you have a lab06b dir in your home dir

USERS = [] #update

def request_handler(request):
    if request['method'] =="GET":
        with sqlite3.connect(ht_db) as c:
            USERS = ["rwagh","sample_partner"]
            now = datetime.datetime.now()
            conn = sqlite3.connect(ht_db)
            c = conn.cursor()
            rwagh_data = c.execute('''SELECT * FROM sensor_data WHERE user = ? ORDER by time_ ASC;''',("rwagh",)).fetchall()
            sample_data = c.execute('''SELECT * FROM sensor_data WHERE user = ? ORDER by time_ ASC;''',("sample",)).fetchall()
        rwagh_time = []
        rwagh_temp = []
        rwagh_press = []
        for step in rwagh_data:
            rwagh_time.append(datetime.datetime.strptime(step[0],'%Y-%m-%d %H:%M:%S.%f'))
            rwagh_temp.append(step[2])
            rwagh_press.append(step[3])
        sample_time = []
        sample_temp = []
        sample_press = []
        for step in sample_data:
            sample_time.append(datetime.datetime.strptime(step[0],'%Y-%m-%d %H:%M:%S.%f'))
            sample_temp.append(step[2])
            sample_press.append(step[3])
        #now plot
        temp_plot = figure(title="Temperature", x_axis_label='Time', y_axis_label='Temperature (C)', x_axis_type='datetime')

        temp_plot.line(rwagh_time, rwagh_temp, legend_label="rwagh", line_color="yellow")
        temp_plot.line(sample_time, sample_temp, legend_label="sample_partner", line_color="blue")

        press_plot = figure(title="Pressure", x_axis_label='Time', y_axis_label='Pressure (hPa)', x_axis_type='datetime')

        press_plot.line(rwagh_time, rwagh_press, legend_label="rwagh", line_color="yellow")
        press_plot.line(sample_time, sample_press, legend_label="sample_partner", line_color="blue")
        script1, div1 = components(temp_plot)
        script2, div2 = components(press_plot)
        return f'''<!DOCTYPE html>
<html> <script src="https://cdn.bokeh.org/bokeh/release/bokeh-2.3.0.min.js"></script>
    <body>
        {div1}
    </body>
    {script1}
    <body>
        {div2}
    </body>
    {script2}
</html>
'''
    else:
      return "invalid HTTP method for this url."