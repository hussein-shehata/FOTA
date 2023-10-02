import serial

# Define the serial port and baud rate
serial_port = 'COM7'  # Replace with your serial port (e.g., 'COM1' on Windows)
baud_rate = 115200

try:
    # Open the serial port
    ser = serial.Serial(serial_port, baud_rate, timeout=1)

    # Define the data to send (0xAA)
    data_to_send = bytes.fromhex('AA')

    # Send the data over UART
    ser.write(data_to_send)

    print(f"Sent: {data_to_send.hex().upper()}")

    # Wait for and receive data
    received_data = ser.read(len(data_to_send))

    if received_data == data_to_send:
        print(f"Received: {received_data.hex().upper()}")
        print("Received the expected value.")
    else:
        print(f"Received: {received_data.hex().upper()}")
        print("Received a different value.")

except Exception as e:
    print(f"An error occurred: {e}")

