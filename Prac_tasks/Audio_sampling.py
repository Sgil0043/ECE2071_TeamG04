import numpy as np
import wave 
import serial 
import time

import serial.tools.list_ports


#empty list for storing data

aud_data= []
samp_rate= 8000 #sample rate 
samp_time= 5 #seconds


#loop for 5 secs
#reads i byte of data from UART 
#appends it to aud_data

ser= serial.Serial('COM6', 115200, timeout=1.0)
time.sleep(3) # Buffer to allow for STM to boot up
ser.reset_input_buffer() # Discards any erroneous bytes that aren't meant to be read

print(f"Audio sampling beggining for {samp_time} seconds...\n ")

for i in range(samp_time*samp_rate):
    data= ser.read(1)
    if data:
        aud_data.append(data[0])
    

    else:
        print("Exiting...\n ")
        break


#converting to an array numpy
aud_data= np.array(aud_data)

#normalising 

aud_data= (aud_data - aud_data.min())/aud_data.max() #scals from 0-1
aud_data= aud_data*255
aud_data= aud_data.astype(np.uint8) #converting to uint8 type

#writing to wav file 

with wave.open('audio.wav', 'wb') as wf:
    wf.setnchannels(1) #mono aud (single channel)
    wf.setsampwidth(1) #1 byte per samp
    wf.setframerate(samp_rate) # setting same samp rate data recorded at  
    wf.writeframes(aud_data.tobytes())
