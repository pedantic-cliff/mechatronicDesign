import random
from Tkinter import *

class Board(object):
    
    def __init__(self, width=400, height=500):

        self.root = Tk()
        self.width = width
        self.height = height

        # create canvas
        self.canvas = Canvas(self.root,width=self.width,height=self.height)
        self.canvas.pack()
        
        # fix the size of the window
        self.root.resizable(width=FALSE, height=FALSE)

        
    # Override these methods when creating your own animation
    def init(self):pass
    def redrawAll(self): pass
    def timerFired(self): pass
    def mousePressed(self, event): pass
    def setWidget(self): pass

    # Call app.run to get your app started
    def run(self):
            
        def redrawAllWrapper():
            self.canvas.delete(ALL)
            self.redrawAll()
            
        def mousePressedWrapper(event):
            self.mousePressed(event)
            redrawAllWrapper()

        self.root.bind("<Button-1>", mousePressedWrapper)

        self.timerFiredDelay = 250   # milliseconds

        def timerFiredWrapper():
            self.timerFired()
            redrawAllWrapper()
            self.canvas.after(self.timerFiredDelay, timerFiredWrapper)

        # init and get timerFired running
        self.init()
        timerFiredWrapper()

        # launch the app
        # This call BLOCKS (so your program waits until you close the window!)
        self.root.mainloop()  
