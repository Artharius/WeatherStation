import urllib2
import json
'''
After obtaining a <WUNDERGROUND API KEY> from Weather underground (there are free plans)
You can query your Personal weather station (PWS) by using the <PWS ID>
and get the data in Json format for further usage, like plotting your weather data
'''

myurl        = 'http://api.wunderground.com/api/<WUNDERGROUND API KEY>/conditions/settings/q/pws:<PWS ID>.json'

def get_report():
    rettup = []
    try:
        page = urllib2.urlopen(myurl)
        myj  = json.load(page)
        print "temp_c" , myj["current_observation"]["temp_c"] , '\n'
        print "dewpoint_c" , myj["current_observation"]["dewpoint_c"] , '\n'
        print "relative_humidity" , myj["current_observation"]["relative_humidity"] , '\n'
        rettup.append(myj["current_observation"]["temp_c"])
        rettup.append(myj["current_observation"]["dewpoint_c"])
        rettup.append(myj["current_observation"]["relative_humidity"])
    except:
        pass
    return rettup

    
if __name__ == "__main__":
    while(1):
        tup = get_report()
        if len(tup) == 3:
            print tup
        time.sleep(5 * 60)
    print 'Finished'
    
