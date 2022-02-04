import sys
import pygame

from utils import *

# connection config
client = connectToServer()

# pygame config
pygame.init()
WIDTH, HEIGHT = 1200, 800
win = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Hangman")

# fonts
LETTER_FONT = pygame.font.SysFont('comicsans', 40)
WORD_FONT = pygame.font.SysFont('comicsans', 60)
TITLE_FONT = pygame.font.SysFont('comicsans', 70)

# load images
images = []
for i in range(8):
    image = pygame.image.load(f"images/{i}.png")
    images.append(image)

# colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
LIGHT_GRAY = (192, 192, 192)

# rectangles
createRoomRect = pygame.Rect(WIDTH / 4 - 200, 200, 400, 500)
joinRoomRect = pygame.Rect(3*WIDTH / 4 - 200, 200, 400, 500)
quitRoomRect = pygame.Rect(1000, 100, 150, 60)

# text variables
usernameText = ''
roomnameText = ''
roundNumberText = ''

# gamestate
gamestate = 0

# flags and consts
AMIHOST = False
NUMBERS = [pygame.K_1, pygame.K_2, pygame.K_3, pygame.K_4, pygame.K_5, pygame.K_6, pygame.K_7, pygame.K_8, pygame.K_9]

# lists
peopleInTheRoom = []


def drawUsernameWindow(response):
    win.fill(WHITE)

    titleText = TITLE_FONT.render("WISIELEC", True, BLACK)
    win.blit(titleText, (WIDTH / 2 - titleText.get_width() / 2, 20))

    usernameLabelText = LETTER_FONT.render("Wprowadź nick:", True, BLACK)
    win.blit(usernameLabelText, (WIDTH / 2 - usernameLabelText.get_width() / 2, 200))

    inputRect = pygame.Rect(WIDTH / 2 - 200, 300, 400, 70)

    pygame.draw.rect(win, BLACK, inputRect, 5)
    inputSurface = LETTER_FONT.render(usernameText, True, BLACK)
    win.blit(inputSurface, (inputRect.x + 5, inputRect.y + 5))

    responseText = LETTER_FONT.render(response, True, RED)
    win.blit(responseText, (WIDTH / 2 - responseText.get_width() / 2, 400))

    inputRect.w = max(400, inputSurface.get_width() + 10)

    pygame.display.update()


def drawLobby():
    win.fill(WHITE)

    titleText = TITLE_FONT.render("WISIELEC", True, BLACK)
    win.blit(titleText, (WIDTH / 2 - titleText.get_width() / 2, 20))

    pygame.draw.rect(win, LIGHT_GRAY, createRoomRect)
    pygame.draw.rect(win, LIGHT_GRAY, joinRoomRect)

    createRoomText = LETTER_FONT.render("Utwórz nowy pokój", True, BLACK)
    win.blit(createRoomText, (WIDTH / 4 - createRoomText.get_width() / 2, 450 - createRoomText.get_height() / 2))
    joinRoomText = LETTER_FONT.render("Dołącz do pokoju", True, BLACK)
    win.blit(joinRoomText, (3*WIDTH / 4 - joinRoomText.get_width() / 2, 450 - joinRoomText.get_height() / 2))

    pygame.display.update()


def drawCreateJoinRoom(response):
    win.fill(WHITE)

    titleText = TITLE_FONT.render("WISIELEC", True, BLACK)
    win.blit(titleText, (WIDTH / 2 - titleText.get_width() / 2, 20))

    roomnameLabelText = LETTER_FONT.render("Wprowadź nazwę pokoju:", True, BLACK)
    win.blit(roomnameLabelText, (WIDTH / 2 - roomnameLabelText.get_width() / 2, 200))

    inputRect = pygame.Rect(WIDTH / 2 - 200, 300, 400, 70)

    pygame.draw.rect(win, BLACK, inputRect, 5)
    inputSurface = LETTER_FONT.render(roomnameText, True, BLACK)
    win.blit(inputSurface, (inputRect.x + 5, inputRect.y + 5))

    responseText = LETTER_FONT.render(response, True, RED)
    win.blit(responseText, (WIDTH / 2 - responseText.get_width() / 2, 400))

    pygame.draw.rect(win, RED, quitRoomRect)
    quitRoomText = LETTER_FONT.render("Cofnij", True, WHITE)
    win.blit(quitRoomText, (quitRoomRect.centerx - quitRoomText.get_width() / 2, quitRoomRect.centery - quitRoomText.get_height() / 2))

    pygame.display.update()


def drawGameRoom(roomname, players):
    win.fill(WHITE)

    titleText = TITLE_FONT.render("WISIELEC", True, BLACK)
    win.blit(titleText, (WIDTH / 2 - titleText.get_width() / 2, 20))

    roomnameText = LETTER_FONT.render(roomname, True, BLACK)
    win.blit(roomnameText, (WIDTH / 2 - roomnameText.get_width() / 2, 100))

    if AMIHOST:
        roundNumberLabelText = LETTER_FONT.render("Podaj liczbę rund, przez które będziesz hostem:", True, BLACK)
        win.blit(roundNumberLabelText, (WIDTH / 2 - roundNumberLabelText.get_width() / 2, 200))

        inputRect = pygame.Rect(WIDTH / 2 - 200, 300, 400, 70)

        pygame.draw.rect(win, BLACK, inputRect, 5)
        inputSurface = LETTER_FONT.render(roundNumberText, True, BLACK)
        win.blit(inputSurface, (inputRect.x + 5, inputRect.y + 5))

    for i, player in enumerate(players):
        playerText = LETTER_FONT.render(player, True, BLACK) if player != usernameText else LETTER_FONT.render(player, True, RED)
        win.blit(playerText, (WIDTH / 2 - playerText.get_width() / 2, 400 + i * 80))

    pygame.draw.rect(win, RED, quitRoomRect)
    quitRoomText = LETTER_FONT.render("Wyjdź", True, WHITE)
    win.blit(quitRoomText, (quitRoomRect.centerx - quitRoomText.get_width() / 2, quitRoomRect.centery - quitRoomText.get_height() / 2))

    pygame.display.update()


def sendUsername(username):
    global gamestate
    send(client, f"{username}")

    response = receive(client)

    if response == "Correct username":
        gamestate = 1
        return "Poprawna nazwa użytkownika"
    else:
        return "Nazwa użytkownika zajęta"


# action - create or join
def sendRoomname(roomname, action):
    global gamestate, AMIHOST
    send(client, f"{action} {roomname}")
    response = receive(client)
    print(response)

    if response == "Host":
        AMIHOST = True
        response = receive(client)

    if response in ["Correct create roomname", "Correct join roomname"]:
        gamestate = 3
        return response
    else:
        return "Nazwa pokoju jest już zajęta" if action == 'create' else "Niepoprawna nazwa pokoju"


def sendQuit():
    global gamestate, AMIHOST

    send(client, "quit")
    AMIHOST = False
    gamestate = 1

    response = receive(client)
    return


def sendRoundNumber(roundNumber):
    return NotImplemented


def main():
    global usernameText, roomnameText, roundNumberText, gamestate, AMIHOST, peopleInTheRoom

    FPS = 60
    clock = pygame.time.Clock()

    response = ''
    createOrJoin = ''

    if receive(client) == 'Gamestate 0':
        gamestate = 0


    while True:
        clock.tick(FPS)

        serverMessage = receive(client)

        if serverMessage == "Host":
            AMIHOST = True

        if gamestate == 0:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        pygame.quit()
                        sys.exit()

                    if event.key == pygame.K_BACKSPACE:
                        usernameText = usernameText[:-1]
                    elif event.key == pygame.K_RETURN:
                        response = sendUsername(usernameText)
                    elif len(usernameText) > 20:
                        pass
                    else:
                        usernameText += event.unicode

            drawUsernameWindow(response)

        elif gamestate == 1:
            response = ''
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.MOUSEBUTTONDOWN:
                    if createRoomRect.collidepoint(event.pos):
                        createOrJoin = 'create'
                        gamestate = 2
                    elif joinRoomRect.collidepoint(event.pos):
                        createOrJoin = 'join'
                        gamestate = 2

            drawLobby()

        elif gamestate == 2:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.MOUSEBUTTONDOWN:
                    if quitRoomRect.collidepoint(event.pos):
                        sendQuit()
                        response = ''

                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        pygame.quit()
                        sys.exit()

                    if event.key == pygame.K_BACKSPACE:
                        roomnameText = roomnameText[:-1]
                    elif event.key == pygame.K_RETURN:
                        response = sendRoomname(roomnameText, createOrJoin)
                    elif len(roomnameText) > 20:
                        pass
                    else:
                        roomnameText += event.unicode

            drawCreateJoinRoom(response)

        elif gamestate == 3:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.MOUSEBUTTONDOWN:
                    if quitRoomRect.collidepoint(event.pos):
                        sendQuit()
                        response = ''

                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        pygame.quit()
                        sys.exit()

                    if AMIHOST:
                        if event.key == pygame.K_BACKSPACE:
                            roundNumberText = roundNumberText[:-1]
                        elif event.key == pygame.K_RETURN:
                            response = sendRoundNumber(roundNumberText)
                        elif len(roundNumberText) > 1:
                            pass
                        elif event.key in NUMBERS:
                            roundNumberText += event.unicode

            if serverMessage[:15] == "PeopleInTheRoom":
                peopleInTheRoom = getPeopleInTheRoom(serverMessage[16:])

            drawGameRoom(roomnameText, peopleInTheRoom)

        else:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        pygame.quit()
                        sys.exit()

        pygame.display.flip()
        clock.tick(60)


if __name__ == "__main__":
    main()
