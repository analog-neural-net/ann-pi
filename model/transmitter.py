import socket
import numpy as np
import os

FIFO_PATH = '/tmp/cpp_to_py_fifo'

# Ensure FIFO exists
if not os.path.exists(FIFO_PATH):
	print("FIFO DOES NOT EXIST")
    #os.mkfifo(FIFO_PATH)

# Setup socket
my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
my_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

my_socket.bind(('100.113.45.98', 2663))
my_socket.listen()

conn, addr = my_socket.accept()
print("Connection received from", addr)

# Open FIFO for reading
print("Waiting for trigger signal from C++...")
with open(FIFO_PATH, 'r') as fifo:
    while True:
		
        char = fifo.readline().strip()
        if char == "1":
            print("Sending image...")
            softmax_data = np.loadtxt('./data/softmax_results.csv')

            with open('./data/step_1.jpg', "rb") as preprocessed_image, \
                 open('./data/step_8.jpg', "rb") as processed_image:

                #softmax = np.loadtxt("./data/softmax_results.csv", delimiter=",")
                raw_image_bytes = preprocessed_image.read()
                raw_processed = processed_image.read()
                raw_softmax_bytes = softmax_data.astype(np.float32).tobytes()

                # Construct payload: [size][data]
                #payload = int.to_bytes(len(raw_processed), 8, 'big') + raw_processed

                # (Optional: include other data here)
                payload = (
                     int.to_bytes(len(raw_image_bytes), 8, 'big') + raw_image_bytes +
                     int.to_bytes(len(raw_processed), 8, 'big') + raw_processed +
                     int.to_bytes(len(raw_softmax_bytes), 1, 'big') + raw_softmax_bytes +
                     b'\x00'
                )
                
                payload = int.to_bytes(len(payload), 8, 'big') + payload;

                conn.sendall(payload)
                print("Data sent to dashboard.")
