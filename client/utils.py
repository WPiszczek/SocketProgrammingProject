import socket


def connectToServer():
    HOST = '141.144.224.178'
    PORT = 9000

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    client.connect((HOST, PORT))
    print('Socket Connected to ' + HOST)
    return client
