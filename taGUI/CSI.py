"""
Confined Space Inspector Contest GUI for course 24-778 Mechatronic Design

written by Hang Cui, 03/24/2014

You should install python 2.7.x, not python 3

For Linux or Mac, after the installation, run commmand "sudo apt-get install python-tk" in your terminal

To run the GUI, for windows user double click "CSI.py", for Linux or Mac, run "python CSI.py" in the terminal


Instructions:

1. choose the colors of the defects, at least one color
2. choose the number of the defects
3. click generate (it will generate the pattern randomly)
4. set up the testbed, the same patterns with the GUI
5. you can use start button to calculate the time or just use your watch
6. when you generate the datas showed in file pattern(the format should be the same), using TA's protocol
7. click button calculation to calculate your total score

For now it is not a real-time display GUI. But the final version should be the same with this one except it can
show patterns in real-time when your robot moves.

"""

#!/usr/bin/python
import time
import random
import threading
from Tkinter import *
from Board import Board

import tkMessageBox

def make2dList(rows, cols):
    a=[]
    for row in xrange(rows): a += [[0]*cols]
    return a

class BoardGame(Board):

############################## Event Detection ###############################
    
    # call by mousePressed
    def cellPressed(self, row, col):
        print "cell pressed: (%d, %d)" % (row, col)
        # self.board[row][col] = self.getCurrentPlayer()

    def mousePressed(self, event):
        if (self.isOnBoard(event.x, event.y)):
            # check the cell which is pressed
            (row, col) = self.getCellFromLocation(event.x, event.y)
            self.cellPressed(row, col)
            
    def getCellFromLocation(self, x, y):
        (boardX0, boardY0, boardX1, boardY1) = self.getBoardBounds()
        row = (y - boardY0) / self.cellSize
        col = (x - boardX0) / self.cellSize
        return (row, col)

    def getBoardBounds(self):
        # coordinate of top left corner
        boardX0 = self.boardMargin
        boardY0 = self.titleMargin + self.boardMargin
        # coordinate of bottom right corner
        boardX1 = self.width - self.boardMargin
        boardY1 = self.height - self.boardMargin
        return (boardX0, boardY0, boardX1, boardY1)

##############################################################################
    
    def redrawAll(self):
        self.drawTitle()
        self.drawBoard()

    def drawTitle(self):
        self.canvas.create_text(self.width/2, self.titleMargin/2, text=self.title, font=self.titleFont, fill=self.titleFill)
        self.canvas.create_line(0, self.titleMargin, self.width, self.titleMargin, fill=self.titleFill)

    def drawBoard(self):
        # get how many rows and columns
        for row in xrange(self.rows):
            for col in xrange(self.cols):
                self.drawCell(row, col)

    def drawCell(self, row, col):
        (x0, y0, x1, y1) = self.getCellBounds(row, col)
        # draw the whole square
        self.canvas.create_rectangle(x0, y0, x1, y1, fill=self.cellBorderColor)                
        self.drawCellContents(row, col, self.getCellContentsBounds(row, col))

    def getCellBounds(self, row, col):
        # get each cell bounds
        (boardX0, boardY0, boardX1, boardY1) = self.getBoardBounds()
        cellX0 = boardX0 + col*self.cellSize
        cellX1 = cellX0 + self.cellSize
        cellY0 = boardY0 + row*self.cellSize
        cellY1 = cellY0 + self.cellSize
        return (cellX0, cellY0, cellX1, cellY1)
    
    def getCellContentsBounds(self, row, col):
        # get get each cell bounds
        (cellX0, cellY0, cellX1, cellY1) = self.getCellBounds(row, col)
        cm = self.cellMargin
        return (cellX0+cm, cellY0+cm, cellX1-cm, cellY1-cm)

    def drawCellContents(self, row, col, bounds):
        (x0, y0, x1, y1) = bounds
        # draw each cell square
        self.canvas.create_rectangle(x0, y0, x1, y1, fill=self.cellBackgroundColor)

        color = self.getCellColor(row, col)
      
        if (color != None):
            self.canvas.create_rectangle(x0, y0, x1, y1, fill=color)
    

    def getCellColor(self, row, col):
        # get the value of each cell 
        value = self.board[row][col]
        if (type(value) == str):
            # string values should be color names, like "blue", etc...
            return value
        elif (type(value) == int):
            assert(-1 < value < len(self.cellColors))
            return self.cellColors[value]
        else:
            raise Exception("Unknown board value: %r" % value)        

    def isOnBoard(self, x, y):
        (boardX0, boardY0, boardX1, boardY1) = self.getBoardBounds()
        return ((x >= boardX0) and (x <= boardX1) and
                (y >= boardY0) and (y <= boardY1))

 
    def __init__(self, rows, cols, cellSize=30):

        self.cellMargin = 0
        self.titleFill = "blue"
        self.cellBorderColor = "black"
        self.titleFont = "Arial 15 bold"
        self.cellBackgroundColor = "grey"
        self.title = "Confined Space Inspector Contest"
        
        self.board = make2dList(rows, cols)
        self.cellColors = [None,"red","yellow"]

        self.rows = rows
        self.cols = cols
        self.titleMargin = 40
        self.boardMargin = 20
        self.cellSize = cellSize
        width = self.cols*self.cellSize + 2*self.boardMargin
        height = (self.rows * self.cellSize) + self.titleMargin + 2*self.boardMargin
        super(BoardGame, self).__init__(width+200, height)

        self.score = StringVar()
        self.timer = StringVar()
        self.missed = StringVar()
        self.detected = StringVar()
        

    def init(self):
        # reset cells
        for row in xrange(self.rows):
            for col in xrange(self.cols):
                self.board[row][col] = 0
                
        self.score.set("Score: 0")
        self.timer.set("Timer: 00:00")
        self.missed.set("Missed: 0")
        self.detected.set("Detected: 0")
        # self.redDefect.get() 1->checked 0->unchecked
        self.redDefect = IntVar()
        self.yellowDefect = IntVar()
        
        self.defectNum = 0
        self.detectNum = 0
        self.totalTime = 0
        self.isTimer = False
        self.setWidget()
        
        
    # set the widgets 
    def setWidget(self):
        
        # Set Labels finished
        self.scoreLabel = Label(self.root, textvariable = self.score, fg="black",font="Arial 15 bold")
        self.timerLabel = Label(self.root, textvariable = self.timer, fg="black",font="Arial 15 bold")
        self.missLabel = Label(self.root, textvariable = self.missed, fg="black",font="Arial 15 bold")
        self.detectLabel = Label(self.root, textvariable = self.detected, fg="black",font="Arial 15 bold")

        self.scoreLabel.place(x=self.width-200,y=60)
        self.timerLabel.place(x=self.width-200,y=95)
        self.missLabel.place(x=self.width-200,y=130)
        self.detectLabel.place(x=self.width-200,y=165)
        
        
        # Set Checkbutton finished
        self.colorLabel = Label(text="Defect Setting:",fg="black",font="Arial 15 bold").place(x=self.width-200,y=205)
        self.redCheck = Checkbutton(text="Red",font="Arial 13 bold",variable=self.redDefect)
        self.yellowCheck = Checkbutton(text="Yellow",font="Arial 13 bold",variable=self.yellowDefect)
        self.redCheck.place(x=self.width-200,y=240)
        self.yellowCheck.place(x=self.width-120,y=240)
        

        # Set Spinbox finished
        self.numSpinbox = Spinbox(from_=0,to=50,font="Arial 13 bold",width="14",command = self.spinbox)
        self.numSpinbox.place(x=self.width-200,y=280)

        # Set Buttons
        self.generateButton = Button(text="Generate",fg="black",font="Arial 15 bold",width="12",command = self.generatePattern)
        self.resetButton = Button(text="Reset",fg="black",font="Arial 15 bold",width="12",command = self.resetPattern)
        self.startButton = Button(text="Start",fg="black",font="Arial 15 bold",width="12",command = self.startTimer)
        self.stopButton = Button(text="Stop",fg="black",font="Arial 15 bold",width="12",command = self.stopTimer)
        self.calButton = Button(text="Calculation",fg="black",font="Arial 15 bold",width="12",command = self.calScore)
        self.gameButton = Button(text="New Game",fg="black",font="Arial 15 bold",width="12",command = self.newGame)
        
        self.generateButton.place(x=self.width-200,y=330)
        self.resetButton.place(x=self.width-200,y=390)
        self.startButton.place(x=self.width-200,y=450)
        self.stopButton.place(x=self.width-200,y=510)
        self.calButton.place(x=self.width-200,y=570)
        self.gameButton.place(x=self.width-200,y=630)

        
    # get the number of the defects
    def spinbox(self):
        num = int(self.numSpinbox.get())
        self.defectNum = num
        # print self.defectNum
        

    def generatePattern(self):
        if(not(self.redDefect.get() or self.yellowDefect.get())):
            tkMessageBox.showwarning("Defect Color", "Please choose at least one color for the defects!")
        elif(self.defectNum==0):
            tkMessageBox.showwarning("Defect Number", "Please choose the number of the defects!")
        else:
            # initialize missed defects
            tempMiss =  "Missed: "+ str(self.defectNum)
            self.missed.set(tempMiss)
            counter = self.defectNum
            while(counter>0):
                row = random.randint(0,8)
                col = random.randint(0,8)
                if(self.redDefect.get()==1 and self.yellowDefect.get()==1):
                    if(self.board[row][col]==0):
                        self.board[row][col] = random.randint(1,2)
                        counter = counter -1
                elif(self.redDefect.get()==1 and self.yellowDefect.get()==0):
                    if(self.board[row][col]==0):
                        self.board[row][col] = 1
                        counter = counter -1
                else:
                    if(self.board[row][col]==0):
                        self.board[row][col] = 2
                        counter = counter -1
                # print "Generate the patterns!"
            

    def resetPattern(self):
        # print "resetPatterns!"
        self.isTimer=False
        self.totalTime = 0
        self.score.set("Score: 0")
        self.detected.set("Detected: 0")
        self.defectNum = int(self.numSpinbox.get())
        tempMiss =  "Missed: "+ str(self.defectNum)
        self.missed.set(tempMiss)
        self.timer.set("Timer: 00:00")
        
        
    def countTime(self):
        while(self.isTimer):
            self.totalTime=self.totalTime+1
            if(self.totalTime >= 60):
                countMin = self.totalTime/60
                countSed = self.totalTime%60
                if(countMin<=9):
                    if(countSed<=9):
                        countTimer = "Timer: 0" + str(self.totalTime/60) + ":0" + str(self.totalTime%60)
                    else:
                        countTimer = "Timer: 0" + str(self.totalTime/60) + ":" + str(self.totalTime%60)
                else:
                    if(countSed<=9):
                        countTimer = "Timer: " + str(self.totalTime/60) + ":0" + str(self.totalTime%60)
                    else:
                        countTimer = "Timer: " + str(self.totalTime/60) + ":" + str(self.totalTime%60)     
                self.timer.set(countTimer)
            else:
                if(self.totalTime<=9):
                    countTimer = "Timer: 00:0" + str(self.totalTime)
                else:
                    countTimer = "Timer: 00:" + str(self.totalTime)  
                self.timer.set(countTimer)
            time.sleep(1)
            # print self.totalTime

    def startTimer(self):
        # print "start timer change self.timer.set()"
        timerThread = threading.Thread(target = self.countTime)
        self.isTimer = True
        timerThread.start()

    def stopTimer(self):
        self.isTimer = False
        # print "stop timer calculate the total time"
        # print self.totalTime

    # do the calculation of the total score
    def calScore(self):
        # print self.totalTime
        # update self.dectNum by reading the file and then calculate the total score

        with open("pattern","r") as fread:
            contents = fread.read()

        contents = str(contents)
        contents = contents[5:]
        print contents
        length = len(contents)
        counter = 0
        data = ""

        for i in xrange(length):
            if(contents[i]!=","):
                data += contents[i]

        lenData = len(data)

        for row in xrange(self.rows):
            for col in xrange(self.cols):
                if(counter!=lenData):
                    if(int(data[counter])!=0 and self.board[row][col]!=0):
                        self.detectNum += 1
                        counter += 1
                    else:
                        counter += 1

        tempScore = self.defectNum*(self.defectNum-self.detectNum)
        tempScore = tempScore /(81-self.defectNum)
        tempScore = self.detectNum - tempScore
        tempScore = tempScore*100/81

        self.detected.set("Detected: "+str(self.detectNum))
        self.missed.set("Missed: "+str(self.defectNum-self.detectNum))
        self.score.set("Score: "+str(tempScore))

    def newGame(self):
        # print "start a new game!"
        start = tkMessageBox.askokcancel("New Game","Are you sure to start a new game?")
        if(start):
            self.init()
        
    

###########################################
# BoardGameTest class (to test BoardGame)
###########################################

class BoardGameTest(BoardGame):
    def __init__(self, rows, cols, cellSize=70):
        super(BoardGameTest, self).__init__(rows, cols, cellSize)
        self.board[0][0] = 1


if (__name__ == "__main__"):
    game = BoardGameTest(9, 9)
    game.run()
