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

# text variables
usernameText = ''
roomnameText = ''

# gamestate
gamestate = 0


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
    win.blit(joinRoomText, (3*WIDTH / 4 - joinRoomText.get_width() / 2, 450 - createRoomText.get_height() / 2))

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

    pygame.display.update()


def sendUsername(username):
    global gamestate
    client.send(bytes(username + '\0', 'ascii'))

    response = client.recv(1024).decode('ascii')
    print(response)

    if response == "Correct username":
        gamestate = 1
        return "Correct username"
    else:
        return "Username zajęty"


# action - create or join
def sendRoomname(roomname, action):
    global gamestate
    client.send(bytes(f"{action} {roomname}\0", 'ascii'))

    response = client.recv(1024).decode('ascii')
    print(response)

    if response in ["Correct create roomname", "Correct join roomname"]:
        gamestate = 3
        return response
    else:
        return "Nazwa pokoju jest już zajęta" if action == 'create' else "Niepoprawna nazwa pokoju"


def main():
    global usernameText, roomnameText, gamestate

    FPS = 60
    clock = pygame.time.Clock()

    response = ''
    createOrJoin = ''

    if client.recv(1024).decode('ascii') == 'Gamestate 0':
        gamestate = 0

    while True:
        clock.tick(FPS)

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

        else:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()
                    # run = False

                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        pygame.quit()
                        sys.exit()

        pygame.display.flip()
        clock.tick(60)


if __name__ == "__main__":
    main()
