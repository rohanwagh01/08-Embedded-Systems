def request_handler(request): 
    text = request['values']['speech']
    #five commands
    if "alert" in text or "alarm" in text or "intruder" in text:
        return {'play': 1}
    elif "tune" in text:
        return {'play': 2}
    elif "Sorry" in text or "Justin" in text or "Beiber" in text:
        return {'play': 3}
    elif "Beyonce" in text or "formation" in text:
        return {'play': 4}
    elif "kendrick" in text or "humble" in text:
        return {'play': 5}
    return {'play': 0}