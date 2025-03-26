import socket
import sys

my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

my_socket.bind(('192.168.0.15', 2663))
my_socket.listen()

conn, adddr = my_socket.accept()
print("connection received")

while(True):
	char = sys.stdin.read(1)
	if (char == "1"):
		image = open('./data/image.jpg', "rb")
		
		raw_bytes = image.read()
		image.close()
		
		length = int.to_bytes(len(raw_bytes), 8, 'big')
		
		send_msg = length + raw_bytes
		
		conn.sendall(send_msg)
		print("image sent")
	else:
		continue
