#demonstrating a PID GUI using tkinter

from tkinter import *
import threading
import time
import serial
import struct


global output
global running

class Port: 
  """The serial communication port"""

  def __init__(self, port):
    self.port = port
    self.serial = serial.Serial(self.port,4800,timeout=0.1)

  def write(self, key, values = None):
    cnt = 0
    if type(key) is str:
      key = key.encode()

    cnt += self.serial.write(key)
    if values != None:
      if type(values) != list:
        values = [ values ]
      buf = struct.pack('%sf' % len(values), *values)
      cnt += self.serial.write(bytes(struct.pack('b', buf.__len__())));
      cnt += self.serial.write(bytes(buf))
    else:
      cnt += self.serial.write(bytes(struct.pack('b',0)));
    return cnt

  def read(self,size = 1): 
    return str(self.serial.read(size),'ascii')

global p
p = Port("/dev/ttyUSB0")

def updateSerialLog():
  i = 0
  while running:
    i+=1
    time.sleep(0.05)
    global p
    output.insert(0.0, p.read(20))

class Application(Frame):
  """ A GUI application"""

  def __init__(self,master):
    """ Initializes the frame"""
    Frame.__init__(self,master)
    self.grid()
    self.create_widgets()


  def create_widgets(self):
    "Creating the widgets"
    self.label_left_P=Label(self,text="Angle Position").grid(row=1, column=1, sticky=W)
    self.label_left_S=Label(self,text="Angle Sum").grid(row=1, column=2, sticky=W)
    self.label_left_D=Label(self,text="Angle Difference").grid(row=1, column=3, sticky=W)

    self.label_right_P=Label(self,text="Dist Position").grid(row=3, column=1, sticky=W)
    self.label_right_S=Label(self,text="Dist Sum").grid(row=3, column=2, sticky=W)
    self.label_right_D=Label(self,text="Dist Difference").grid(row=3, column=3, sticky=W)

    self.data_L_P= Entry(self)
    self.data_L_P.grid(row=2, column=1,sticky=W)                                                                  
    self.data_L_S= Entry(self)
    self.data_L_S.grid(row=2, column=2,sticky=W)
    self.data_L_D= Entry(self)
    self.data_L_D.grid(row=2, column=3,sticky=W)

    self.data_R_P= Entry(self)
    self.data_R_P.grid(row=4, column=1,sticky=W)
    self.data_R_S= Entry(self)
    self.data_R_S.grid(row=4, column=2,sticky=W)
    self.data_R_D= Entry(self)
    self.data_R_D.grid(row=4, column=3,sticky=W)

    self.data_L_P.insert(0,"0")
    self.data_L_S.insert(0,"0")
    self.data_L_D.insert(0,"0")
    self.data_R_P.insert(0,"0")
    self.data_R_S.insert(0,"0")
    self.data_R_D.insert(0,"0")

    self.button=Button(self,text="Send Values",command=self.send).grid(row=8, column=3,sticky=W)
    self.start=Button(self,text="START!",command=self.begin).grid(row=8, column=4,sticky=W)
    self.halt=Button(self,text="STOP!",command=self.stop).grid(row=9, column=3,sticky=W)                                                                   
    self.reset=Button(self,text="Reset",command=self.reset).grid(row=9, column=4,sticky=W)
    
    self.scroller=Scrollbar(self)
    self.scroller.grid(row=15,column=1,sticky='nsew')
    self.scroller.config(width='1')


    self.output=Text(self, width=30, height=25, wrap=WORD)
    self.output.grid(row=15, column=2,sticky='nsew')
    global output
    output = self.output

    self.scroller.config(command=self.output.yview)
    self.output.config(yscrollcommand=self.scroller.set)          

  def send(self):
    """collects and sends the value"""
    values = [self.data_L_P.get(),
        self.data_L_S.get(),
        self.data_L_D.get(),
        self.data_R_P.get(),
        self.data_R_S.get(),
        self.data_R_D.get()]
    values = list(map(lambda x: float(x), values))
    p.write('g', values); 
    
    
  def stop(self):
    """Stops the process""" 
    
    global running
    global p 
    p.write('h')
    running=0

  def begin(self):
    """Starts the process""" 
    global p 
    t = threading.Thread(target=updateSerialLog);
    global running
    running=1
    t.start()

    p.write('s')
    running = 1
  def reset(self):
    self.output.delete(0.0, END)

root=Tk()
root.title("PID Control")
root.geometry("600x600")

app=Application(root)

root.mainloop()                                                                    

