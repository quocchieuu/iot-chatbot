class AppContext:
    def __init__(self, db, ser, kit, speak):
        self.db = db
        self.ser = ser
        self.kit = kit
        self.speak = speak