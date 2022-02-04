import socket


def connectToServer():
    f = open("config.txt", "r")
    HOST = f.readline()[:-1]
    PORT = int(f.readline())

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    client.connect((HOST, PORT))
    print(f'Socket Connected to {HOST} on port {PORT}')

    return client


def receive(client):
    client.settimeout(0.1)
    try:
        messageLength = client.recv(3).decode('UTF-8')
        if messageLength == "":
            return ""

        messageLength = int(messageLength)
        message = client.recv(messageLength).decode('UTF-8')
        print(message)
    except socket.timeout:
        message = ""
    return message


def send(client, message):
    messageLength = len(message)
    messageLength = str(messageLength).zfill(3)
    print(messageLength + message)
    client.send(bytes(messageLength + message, 'UTF-8'))
    return


def getPeopleInTheRoom(message):
    print(message)
    return message.split(";")[:-1]

