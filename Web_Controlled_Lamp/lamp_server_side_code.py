import sqlite3
lamp_db = '/var/jail/home/rwagh/dex05/lamp.db'

def request_handler(request): 
    if request["method"] == "POST": #from website
        conn = sqlite3.connect(lamp_db)
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS color_table (red int, green int, blue int);''')
        c.execute('''INSERT into color_table VALUES (?,?,?);''', (request["form"]["red"], request["form"]["green"], request["form"]["blue"])) 
        conn.commit()
        conn.close() 
        return """
        <!DOCTYPE html>
<html>
<body style="background-color:rgb("""+str(request["form"]["red"])+""","""+str(request["form"]["green"])+""","""+str(request["form"]["blue"])+""");">

<h1>Lamp Control</h1>
<form action="https://608dev-2.net/sandbox/sc/rwagh/dex05/lamp_server_side_code.py" method="post">
  <label for="red">Red (between 0 and 255):</label>
  <input type="range" id="red" name="red" min="0" max="255">
  <br>
  <label for="green">Green (between 0 and 255):</label>
  <input type="range" id="green" name="green" min="0" max="255">
  <br>
  <label for="blue">Blue (between 0 and 255):</label>
  <input type="range" id="blue" name="blue" min="0" max="255">
  <br>
  <input type="submit">
</form>

</body>
</html>
        
        """
    else: #from esp32
        conn = sqlite3.connect(lamp_db)
        c = conn.cursor()
        things = c.execute('''SELECT * FROM color_table''').fetchall()
        for i in things:
            output = i #saves the newest result
        output_str = {'red': output[0], 'green': output[1], 'blue': output[2]}
        return output_str
