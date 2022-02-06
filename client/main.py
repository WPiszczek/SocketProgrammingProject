import sys
import time

from utils import *

# connection config
client = connectToServer()

# pygame config
pygame.init()
WIDTH, HEIGHT = 1200, 800
win = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Hangman")

# fonts
SCORE_FONT = pygame.font.SysFont('comicsans', 20)
LETTER_FONT = pygame.font.SysFont('comicsans', 40)
WORD_FONT = pygame.font.SysFont('comicsans', 60)
TITLE_FONT = pygame.font.SysFont('comicsans', 70)

# load images
images = loadImages()

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
passwordText = ''

# gamestate
gamestate = 0
startRoundTime = None
roundTime = None

# flags and consts
AMIHOST = False
ROUNDNUMBERSET = False
RESULTS = False
MIN_PLAYERS_IN_GAME = 2
NUMBERS = [pygame.K_1, pygame.K_2, pygame.K_3, pygame.K_4, pygame.K_5, pygame.K_6, pygame.K_7, pygame.K_8, pygame.K_9]
LETTERS = [pygame.K_a, pygame.K_b, pygame.K_c, pygame.K_d, pygame.K_e, pygame.K_f, pygame.K_g, pygame.K_h, pygame.K_i,
           pygame.K_j, pygame.K_k, pygame.K_l, pygame.K_m, pygame.K_n, pygame.K_o, pygame.K_p, pygame.K_q, pygame.K_r,
           pygame.K_s, pygame.K_t, pygame.K_u, pygame.K_v, pygame.K_w, pygame.K_x, pygame.K_y, pygame.K_z]
ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

# lists and dicts
peopleInTheRoom = []
letterRects = getLetterRects()


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

    if response != "Correct username":
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

    if response not in ["Correct create roomname", "Correct join roomname"]:
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
        if ROUNDNUMBERSET:
            passwordLabelText = LETTER_FONT.render("Podaj hasło do zgadywania:", True, BLACK)
            win.blit(passwordLabelText, (WIDTH / 2 - passwordLabelText.get_width() / 2, 200))

            inputRect = pygame.Rect(WIDTH / 2 - 200, 300, 400, 70)

            pygame.draw.rect(win, BLACK, inputRect, 5)
            inputSurface = LETTER_FONT.render(passwordText, True, BLACK)
            win.blit(inputSurface, (inputRect.x + 5, inputRect.y + 5))
        else:
            roundNumberLabelText = LETTER_FONT.render("Podaj liczbę rund, przez które będziesz hostem:", True, BLACK)
            win.blit(roundNumberLabelText, (WIDTH / 2 - roundNumberLabelText.get_width() / 2, 200))

            inputRect = pygame.Rect(WIDTH / 2 - 200, 300, 400, 70)

            pygame.draw.rect(win, BLACK, inputRect, 5)
            inputSurface = LETTER_FONT.render(roundNumberText, True, BLACK)
            win.blit(inputSurface, (inputRect.x + 5, inputRect.y + 5))

        if len(peopleInTheRoom) < MIN_PLAYERS_IN_GAME:
            messageText = LETTER_FONT.render("Poczekaj na więcej graczy", True, RED)
            win.blit(messageText, (WIDTH / 2 - messageText.get_width() / 2, 400))

    else:
        messageText = LETTER_FONT.render("Oczekiwanie na rozpoczęcie gry...", True, BLACK)
        win.blit(messageText, (WIDTH / 2 - messageText.get_width() / 2, 200))

    playersInTheRoomText = LETTER_FONT.render("GRACZE W POKOJU:", True, BLACK)
    win.blit(playersInTheRoomText, (WIDTH / 2 - playersInTheRoomText.get_width() / 2, 500))

    newWidth = 200
    height = 600
    for i, player in enumerate(players):
        if player.name != usernameText:
            playerText = LETTER_FONT.render(player.name, True, BLACK)
        else:
            playerText = LETTER_FONT.render(player.name, True, RED)
        if newWidth + playerText.get_width() > 900:
            height += 80
            newWidth = 200
        win.blit(playerText, (newWidth, height))
        newWidth += playerText.get_width() + 50

    pygame.draw.rect(win, RED, quitRoomRect)
    quitRoomText = LETTER_FONT.render("Wyjdź", True, WHITE)
    win.blit(quitRoomText, (quitRoomRect.centerx - quitRoomText.get_width() / 2, quitRoomRect.centery - quitRoomText.get_height() / 2))

    pygame.display.update()


def drawGame(players, password):
    win.fill(WHITE)

    titleText = TITLE_FONT.render("WISIELEC", True, BLACK)
    win.blit(titleText, (WIDTH / 2 - titleText.get_width() / 2, 20))

    passwordText = WORD_FONT.render(spaceBetweenLetters(password), True, BLACK)
    win.blit(passwordText, (WIDTH / 2 - passwordText.get_width() / 2, 200))

    newWidth = 100
    height = 300
    for i, player in enumerate(players):
        if player.isHost:
            continue

        if player.name != usernameText:
            playerNameText = SCORE_FONT.render(player.name, True, BLACK)
            playerScoreText = SCORE_FONT.render(f"{player.score} pkt", True, BLACK)
        else:
            playerNameText = SCORE_FONT.render(player.name, True, RED)
            playerScoreText = SCORE_FONT.render(f"{player.score} pkt", True, RED)
        if newWidth + playerNameText.get_width() > 1000:
            height += 130
            newWidth = 100
        win.blit(playerNameText, (newWidth, height))
        win.blit(playerScoreText, (newWidth, height + 30))
        win.blit(images[7 - player.lives], (newWidth, height + 60))
        newWidth += playerNameText.get_width() + 150

    for i, letter in enumerate(ALPHABET):
        letterRect = letterRects[i]
        x = letterRect.x + 30
        y = letterRect.y + 30
        letterText = WORD_FONT.render(letter, True, BLACK)

        pygame.draw.rect(win, BLACK, letterRect, 5)
        win.blit(letterText, (x - letterText.get_width() / 2, y - letterText.get_height() / 2))

    pygame.draw.rect(win, RED, quitRoomRect)
    quitRoomText = LETTER_FONT.render("Wyjdź", True, WHITE)
    win.blit(quitRoomText, (quitRoomRect.centerx - quitRoomText.get_width() / 2, quitRoomRect.centery - quitRoomText.get_height() / 2))

    pygame.display.update()


def drawResults(players, previousPassword):
    global passwordText
    win.fill(WHITE)

    titleText = TITLE_FONT.render("WISIELEC", True, BLACK)
    win.blit(titleText, (WIDTH / 2 - titleText.get_width() / 2, 20))

    previousPasswordText = LETTER_FONT.render(f"Hasło to {previousPassword}", True, BLACK)
    win.blit(previousPasswordText, (WIDTH / 2 - previousPasswordText.get_width() / 2, 100))

    if AMIHOST:
        if ROUNDNUMBERSET:
            passwordLabelText = LETTER_FONT.render("Podaj nowe hasło do zgadywania:", True, BLACK)
            win.blit(passwordLabelText, (WIDTH / 2 - passwordLabelText.get_width() / 2, 200))

            inputRect = pygame.Rect(WIDTH / 2 - 200, 300, 400, 70)

            pygame.draw.rect(win, BLACK, inputRect, 5)
            inputSurface = LETTER_FONT.render(passwordText, True, BLACK)
            win.blit(inputSurface, (inputRect.x + 5, inputRect.y + 5))
        else:
            roundNumberLabelText = LETTER_FONT.render("Podaj liczbę rund, przez które będziesz hostem:", True, BLACK)
            win.blit(roundNumberLabelText, (WIDTH / 2 - roundNumberLabelText.get_width() / 2, 200))

            inputRect = pygame.Rect(WIDTH / 2 - 200, 300, 400, 70)

            pygame.draw.rect(win, BLACK, inputRect, 5)
            inputSurface = LETTER_FONT.render(roundNumberText, True, BLACK)
            win.blit(inputSurface, (inputRect.x + 5, inputRect.y + 5))

        if len(peopleInTheRoom) < MIN_PLAYERS_IN_GAME:
            messageText = LETTER_FONT.render("Poczekaj na więcej graczy", True, RED)
            win.blit(messageText, (WIDTH / 2 - messageText.get_width() / 2, 400))

    else:
        messageText = LETTER_FONT.render("Oczekiwanie na rozpoczęcie gry...", True, BLACK)
        win.blit(messageText, (WIDTH / 2 - messageText.get_width() / 2, 200))

    resultsText = LETTER_FONT.render("WYNIKI:", True, BLACK)
    win.blit(resultsText, (WIDTH / 2 - resultsText.get_width() / 2, 500))

    newWidth = 200
    height = 600
    for i, player in enumerate(players):
        playerString = f"{player.name} - {player.score} punktów" if not player.isHost else f"{player.name} - host"
        if player.name != usernameText:
            playerText = SCORE_FONT.render(playerString, True, BLACK)
        else:
            playerText = SCORE_FONT.render(playerString, True, RED)
        if newWidth + playerText.get_width() > 900:
            height += 40
            newWidth = 200
        win.blit(playerText, (newWidth, height))
        newWidth += playerText.get_width() + 50

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
    global gamestate, AMIHOST, ROUNDNUMBERSET

    send(client, "quit")
    AMIHOST = False
    ROUNDNUMBERSET = False
    gamestate = 1

    response = receive(client)
    return


def sendRoundNumber(roundNumber):
    global ROUNDNUMBERSET
    send(client, f"round {roundNumber}")
    response = receive(client)

    if response == "Correct round number":
        ROUNDNUMBERSET = True
        return response
    else:
        return "Niepoprawna liczba rund"


def sendPassword(password):
    global gamestate
    print(password)
    send(client, f"set {password}")
    response = receive(client)

    if response == "Correct set":
        gamestate = 4
        return response
    elif response == "Wait for more people":
        gamestate = 3
        return "Poczekaj na więcej graczy"
    else:
        print(response)
        return response


def sendLetter(letter):
    send(client, f"letter {letter}")
    return


def main():
    global usernameText, roomnameText, roundNumberText, passwordText
    global gamestate, AMIHOST, ROUNDNUMBERSET, RESULTS, peopleInTheRoom
    global roundTime, startRoundTime

    FPS = 60
    clock = pygame.time.Clock()

    response = ''
    createOrJoin = ''

    peopleResults = []
    password1 = ''
    previousPassword = ''

    if receive(client) == 'Gamestate 0':
        gamestate = 0

    while True:
        clock.tick(FPS)

        serverMessage = receive(client)

        if startRoundTime is not None and gamestate == 4 and time.time() - startRoundTime > 60:
            send(client, "timeout")

        if serverMessage == "Host":
            AMIHOST = True
            ROUNDNUMBERSET = False
            password1 = ''
        elif serverMessage[:9] == 'Gamestate':
            gamestate = int(serverMessage[10:])
            if gamestate == 4:
                startRoundTime = time.time()

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
                    elif len(usernameText) >= 15:
                        pass
                    elif event.unicode in ALPHABET or event.unicode in ALPHABET.lower() or event.unicode == ' ':
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
                    elif len(roomnameText) >= 15:
                        pass
                    elif event.unicode in ALPHABET or event.unicode in ALPHABET.lower() or event.unicode == ' ':
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
                        if ROUNDNUMBERSET:
                            if event.key == pygame.K_BACKSPACE:
                                passwordText = passwordText[:-1]
                            elif event.key == pygame.K_RETURN and len(peopleInTheRoom) >= MIN_PLAYERS_IN_GAME:
                                response = sendPassword(passwordText)
                                gamestate = 4
                            elif len(passwordText) >= 15:
                                pass
                            elif event.unicode in ALPHABET or event.unicode in ALPHABET.lower():
                                passwordText += event.unicode.upper()
                        else:
                            if event.key == pygame.K_BACKSPACE:
                                roundNumberText = roundNumberText[:-1]
                            elif event.key == pygame.K_RETURN:
                                response = sendRoundNumber(roundNumberText)
                            elif len(roundNumberText) >= 1:
                                pass
                            elif event.key in NUMBERS:
                                roundNumberText += event.unicode

            if serverMessage[:15] == "PeopleInTheRoom":
                peopleInTheRoom = getPeopleInTheRoom(serverMessage[16:])

            if RESULTS:
                drawResults(peopleResults, previousPassword)
            else:
                drawGameRoom(roomnameText, peopleInTheRoom)

        elif gamestate == 4:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.MOUSEBUTTONDOWN:
                    if quitRoomRect.collidepoint(event.pos):
                        sendQuit()
                        response = ''

                    if not AMIHOST:
                        for i, rect in enumerate(letterRects):
                            if rect.collidepoint(event.pos):
                                sendLetter(ALPHABET[i])

            if serverMessage[:6] == "Scores":
                _, _, peopleInTheRoom, password1 = handleScores(serverMessage[7:])

            elif serverMessage[:7] == "Results":
                _, _, peopleResults, previousPassword = handleResults(serverMessage[8:])
                password1 = ''
                gamestate = 3
                RESULTS = True

            elif serverMessage == "Game over":
                AMIHOST = False
                password1 = ''

            drawGame(peopleInTheRoom, password1)

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
