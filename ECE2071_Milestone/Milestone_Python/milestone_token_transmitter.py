import serial
import time

chosenPort = 'COM3' # Determines the head -> we'll be using a PC each so this can remain.

ser = serial.Serial(chosenPort, 115200, timeout=5)
time.sleep(3) # Buffer to allow for STM to boot up
ser.reset_input_buffer() # Discards any erroneous bytes that aren't meant to be read

print("! G04 PROGRAM BEGUN !")

print("Ctrl+C -> exit")

counter = 0 # Keeps track of how many tests we've done

while True:
    input("Any key...") # Press any key to toggle next
    ser.reset_input_buffer() # Discards any erroneous bytes / things that aren't meant to be read
    ser.write(b'starting message\n') # Writes the inital message to the head STM
    print("Sent, waiting for response...") 
    
    response = ser.readline().decode('utf-8').strip() # Formats output from the STM daisy-chain
    if response:
        print(f"Counter {counter}: After the daisy-chaining... {response}") # Prints the output
    else:
        print(f"Counter {counter}: No response received :<") # Tells user an error occurred
    
    counter+=1
    time.sleep(1) # Little buffer to ensure user does not spam the console


   