import sqlite3
import random
key_db = '/var/jail/home/rwagh/dex08/key_db'

def request_handler(request): 
    if request["method"] == "GET": 
        conn = sqlite3.connect(key_db)
        c = conn.cursor()
        if request["args"]: #if not empty then there is a message
            things = c.execute('''SELECT * FROM key_table''').fetchall()
            for i in things:
                output = i #saves the newest result
            m,p,a = output
            t_mcu = int(request["values"]["t"])
            k = (t_mcu^a)%p
            input_message = vigenere_cipher(request["values"]["message"], str(key_to_keyword(k)), False)
            ##reverse input and send it back encrypted
            output_message = ""
            for i in range(len(input_message)):
                output_message += input_message[len(input_message)-1-i]
            ##now encrypt
            conn.commit()
            conn.close()
            return {"message": vigenere_cipher(output_message,str(key_to_keyword(k)), True), "t_server": (m^a)%p}
        else: #no message so create and send back m, p
            #pick random p
            p = int(100*random.random())
            m = int(100*random.random())
            a = int(100*random.random())
            t_serv = (m^a)%p
            c.execute('''CREATE TABLE IF NOT EXISTS key_table (m int, p int, a int);''')
            c.execute('''INSERT into key_table VALUES (?,?,?);''', (m,p,a)) 
            conn.commit()
            conn.close()
            return {"m": m, "p": p, "t_server": t_serv}


def key_to_keyword(k):
    return int(100000*(k/7.654567943))

def vigenere_cipher(message,keyword,encrypt):
    output = ""
    for index in range(len(message)):
        ascii_num = ord(message[index])
        ind = index
        while ind > len(keyword)-1:
            ind -= len(keyword)
        shift = ord(keyword[ind])-32
        new_num = ascii_num + shift if encrypt else ascii_num - shift
        while new_num < 32:
            new_num += 95
        while new_num > 126:
            new_num -= 95
        output += chr(new_num)
    return output
