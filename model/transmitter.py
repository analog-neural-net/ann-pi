import socket
import sys
import numpy as np

my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
my_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

my_socket.bind(('192.168.0.15', 2663))
my_socket.listen()

conn, adddr = my_socket.accept()
print("connection received")

while(True):
	char = sys.stdin.read(1)
	if (char == "1"):
		preproccessed_image = open('./data/step_1.jpg', "rb")
		processed_image = open('./data/step_7.jpg', "rb")
		
		softmax = np.loadtxt("./data/softmax_results.csv", delimiter=",");
		raw_image_bytes = preproccessed_image.read()
		raw_proccessed = processed_image.read()
		raw_softmax_bytes = softmax.tobytes()
		
		raw_send_bytes = int.to_bytes(len(raw_image_bytes), 8, 'big') + raw_image_bytes + int.to_bytes(len(raw_proccessed), 8, 'big') + raw_proccessed + int.to_bytes(len(raw_softmax_bytes), 1, 'big') + raw_softmax_bytes
		preproccessed_image.close()
		processed_image.close()
		
		
		length = int.to_bytes(len(raw_send_bytes), 8, 'big')
		
		send_msg = length + raw_send_bytes
		
		conn.sendall(send_msg)
		print("image sent")
	else:
		continue
