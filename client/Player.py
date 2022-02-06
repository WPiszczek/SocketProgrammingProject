class Player:
    def __init__(self, name, isHost=False):
        self.name = name
        self.score = 0
        self.lives = 7
        self.isHost = isHost
        self.disconnected = False

    def __str__(self):
        return f"Name: {self.name}, Score: {self.score}, Lives: {self.lives}, IsHost: {self.isHost}, Disconnected: {self.disconnected}"

    def __repr__(self):
        return f"Name: {self.name}, Score: {self.score}, Lives: {self.lives}, IsHost: {self.isHost}, Disconnected: {self.disconnected}"
