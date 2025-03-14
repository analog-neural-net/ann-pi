import serial
import time

# Change this to the correct UART device (e.g., /dev/ttyAMA2 for UART2)
UART_DEVICE = "/dev/ttyAMA0"  

# Open the serial port
ser = serial.Serial(UART_DEVICE, baudrate=115200, timeout=1)

def send_data(data):
    """Send a string over UART"""
    ser.write(data.encode())  # Convert string to bytes
    print(f"Sent: {data}")

def receive_data():
    """Receive data over UART"""
    time.sleep(0.1)  # Allow time for data to arrive
    if ser.in_waiting > 0:  # Check if data is available
        received = ser.read(ser.in_waiting).decode()  # Read and decode bytes
        print(f"Received: {received}")
        return received
    else:
        print("No data received.")
        return None

# Test Loopback: Send and Receive "Hello"
send_data("Hello UART")
time.sleep(0.5)  # Small delay before reading
receive_data()

# Close the serial port
ser.close()
