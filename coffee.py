#!/usr/bin/python3
#
# **** coffee.py v1.0 ****
#
# - (C) 20200120 Bernd Künnen
# - demo project for heidolph web2rs232 connector (https://github.com/berndkuennen/heidolph_web2rs232)
# - brews coffee with Heidolph MR Hei-Connect stirrer
#   - put beaker with water and coffee powder on stirrer
#   - water gets heated to 97°C while stirring
# - temperature data gets recorded as tsv filek
#

import requests
import time
import json
from datetime import datetime
import urllib.parse

#== config section ================
stirrer_ip  = "192.168.100.100"   # set here ip address of esp32
tsv_file    = "/tmp/coffee.tsv"   # file to write temp values to

temp_target = 100   # in °C
temp_stop   =  97   # in °C
rpm_target  = 120   # rpm
sleep_time  =  10   # in s


#== code ================
stirrer_url = "http://" + stirrer_ip + "/heidolph/text?payload="

# basic function to send command to stirrer
def heidolph_action(cmd):
  r = requests.get(stirrer_url + urllib.parse.quote_plus(cmd))
  if r.status_code == 200:
    return r.text
  else:
    print('!!  Error on sending command ' + cmd)
    return 'error'

# request current temp  
def get_temp():
  answer = heidolph_action('IN_PV_1')
  if answer[0:7] == 'IN_PV_1':
    return float(answer[8:])
  else:
    return -999.9

# set target temp
def set_temp(T):
  answer = heidolph_action('OUT_SP_1 ' + str(T) )
  if answer[0:8] == 'OUT_SP_1':
    return float(answer[9:])
  else:
    print('! error on setting temperature')
    return -999.9
    
# request current rpm
def get_rpm():
  answer = heidolph_action('IN_PV_5')
  if answer[0:7] == 'IN_PV_5':
    return float(answer[8:])
  else:
    return -999.9

# set target rpm
def set_rpm(R):
  answer = heidolph_action('OUT_SP_3 ' + str(R) )
  if answer[0:8] == 'OUT_SP_3':
    return float(answer[9:])
  else:
    print('! error on setting speed')
    return -999.9

# open tsv file for recording data
def f_open():
  try:
    return open( tsv_file, 'a')
  except:
    print("!! cannot open tsv file " +  tsv_file)
    exit(5)

# switch heating plate on/off
def switch_heating(state):
  print('switching heating ' + state)
  if state == 'on':
    heidolph_action("START_1")
  if state == 'off':
    heidolph_action("STOP_1")

# switch stirring function on/off
def switch_stirring(state):
  print('switching stirring ' + state)
  if state == 'on':
    heidolph_action("START_2")
  if state == 'off':
    heidolph_action("STOP_2")

# record temp data until target temp is reached
def wait4temp(Operator,Temp,Time):
  while True:
    value_t = get_temp()
    value_r = get_rpm()
    output = datetime.now().strftime("%H:%M:%S")  +  "\t"  +  str(value_t) +  "\t"  +  str(value_r)
    print( output )
    f.write( output + "\n" )
    if (Operator == "ge") and (value_t >= Temp ):
      return
    if (Operator == "le") and (value_t <= Temp ):
      return
    time.sleep(Time)


#### main() ###############################

print("""
**** coffee.py ****
- demo for remotely controlling a Heidolph MR Hei-Connect stirrer
- to brew coffee, put a beaker with wter and coffee powder on the stirrer
- the code heats the water to 97°C while stirring
- records temperature data as tsv in """ + tsv_file )

# open tsv file
f = f_open()

# init stirrer
heidolph_action("PA_NEW")

# set target values
if  int(set_temp(temp_target)) != temp_target :
  exit(1)
if  int(set_rpm(rpm_target))   != rpm_target  :
  exit(2)

# turn on heating & stirring
switch_heating('on')
switch_stirring('on')

# measure temp every 5s until target temp is reached
wait4temp( "ge", temp_stop, sleep_time )

# turn off heating, wait, the turn off stirring, too
switch_heating('off')

print('Stirring still active for another 60s!')
time.sleep(60)
switch_stirring('off')

f.close()
print("Ready. Enjoy your coffee.")

