import socket
import pygame

from Player import Player


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
    players = []
    lis = message.split(";")[:-1]
    for name in lis:
        players.append(Player(name))
    return players


def handleScores(message):
    players = []
    lis = message.split(";")[:-1]
    roundNumber = lis[1]
    numberOfRounds = lis[3]
    playersData = lis[4:-2]
    password = lis[-1]
    for i in range(0, len(playersData), 5):
        player = Player(playersData[i])
        if playersData[i + 1] == 'host':
            player.isHost = True
        else:
            player.score = int(playersData[i + 2])
            player.lives = int(playersData[i + 4])
        players.append(player)

    return roundNumber, numberOfRounds, players, password.upper()


def handleResults(message):
    players = []
    lis = message.split(";")[:-1]
    roundNumber = lis[1]
    numberOfRounds = lis[3]
    playersData = lis[4:-2]
    password = lis[-1]
    print(playersData)
    for i in range(0, len(playersData), 6):
        player = Player(playersData[i])
        if playersData[i + 1] == 'host':
            player.isHost = True
        else:
            player.score = int(playersData[i + 2])
            player.lives = int(playersData[i + 4])
            player.disconnected = playersData[i + 5]
        players.append(player)

    players.sort(key=lambda x: x.score, reverse=True)

    return roundNumber, numberOfRounds, players, password.upper()


def spaceBetweenLetters(word):
    return "".join(letter + " " for letter in word)


def getLetterRects():
    rects = []
    GAP = 30
    startx = 100
    starty = 600
    for i in range(26):
        x = startx + ((50 + GAP) * (i % 13))
        y = starty + ((i // 13) * 100)
        letterRect = pygame.Rect(x - 30, y - 30, 60, 60)
        rects.append(letterRect)
    return rects


def loadImages():
    images = []
    for i in range(8):
        image = pygame.image.load(f"images/{i}.png")
        image = pygame.transform.scale(image, (200, 200))
        images.append(image)
    return images
