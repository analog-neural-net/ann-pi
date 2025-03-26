import socket

my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

my_socket.bind(('192.168.0.15', 2663))
my_socket.listen()

while(True):
	conn, adddr = my_socket.accept()
	print("connection success")
	conn.close()
