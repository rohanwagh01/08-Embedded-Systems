import sqlite3
import time
example_db = "example.db" #make sure this is run in the same directory as your other file.
conn = sqlite3.connect(example_db)
c = conn.cursor()

start = time.time()
things = c.execute('''SELECT * FROM test_table ORDER BY favorite_number DESC;''').fetchall()
print(things[0])
print(time.time()-start)

start = time.time()
thing = c.execute('''SELECT * FROM test_table ORDER BY favorite_number DESC;''').fetchone()
print(thing)
print(time.time()-start)

conn.commit()
conn.close()