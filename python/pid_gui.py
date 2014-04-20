#demonstrating a PID GUI using tkinter

from Tkinter import *
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
    self.serial = serial.Serial(self.port,9600,timeout=0.1)

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
    print cnt
    return cnt

  def read(self,size = 1): 
    return str(self.serial.read(size))

global p
p = Port("/dev/ttyUSB0")

def updateSerialLog():
  i = 0
  while running:
    i+=1
    time.sleep(0.05)
    global p
    output.insert(END, p.read(10))
    output.yview(END)

class Application(Frame):
  """ A GUI application"""

  def __init__(self,master):
    """ Initializes the frame"""
    Frame.__init__(self,master)
    self.grid()
    self.create_widgets()


  def create_widgets(self):
    "Creating the widgets"
    self.label_P=Label(self,text="Kp").grid(row=1, column=3, sticky=W)
    self.label_S=Label(self,text="Ks").grid(row=1, column=4, sticky=W)
    self.label_D=Label(self,text="Kd").grid(row=1, column=5, sticky=W)
    
    
    self.label_dist=Label(self,text="Distance").grid(row=2, column=2, sticky=W)
    self.label_angle=Label(self,text="Angle").grid(row=3, column=2, sticky=W)
    self.label_motor=Label(self,text="Motor").grid(row=4, column=2, sticky=W)
    
    self.label_Dist=Label(self,text="FWD Dist").grid(row=1, column=6, sticky=W)
    self.label_min_pwm_l=Label(self,text="Min PWN Left").grid(row=1, column=7, sticky=W)
    self.label_min_pwn_r=Label(self,text="Min PWM Right").grid(row=1, column=8, sticky=W)
    self.label_encoder_weight_l=Label(self,text="Left encoder weight").grid(row=3, column=7, sticky=W)
    self.label_encoder_weight_r=Label(self,text="Right encoder weight").grid(row=3, column=8, sticky=W)
    self.label_pwm_bias_l=Label(self,text="Left PWM Bias").grid(row=5, column=7, sticky=W)
    self.label_pwm_bias_r=Label(self,text="Right PWM Bias").grid(row=5, column=8, sticky=W)

    #self.label_integ_thres=Label(self,text="Integral Threshold").grid(row=1, column=9, sticky=W)

    self.data_aP= Entry(self)
    self.data_aP.grid(row=2, column=3,sticky=W)                                                                  
    self.data_aS= Entry(self)
    self.data_aS.grid(row=2, column=4,sticky=W)
    self.data_aD= Entry(self)
    self.data_aD.grid(row=2, column=5,sticky=W)

    self.data_dP= Entry(self)
    self.data_dP.grid(row=3, column=3,sticky=W)
    self.data_dS= Entry(self)
    self.data_dS.grid(row=3, column=4,sticky=W)
    self.data_dD= Entry(self)
    self.data_dD.grid(row=3, column=5,sticky=W)

    self.data_mP= Entry(self)
    self.data_mP.grid(row=4, column=3,sticky=W)
    self.data_mS= Entry(self)
    self.data_mS.grid(row=4, column=4,sticky=W)
    self.data_mD= Entry(self)
    self.data_mD.grid(row=4, column=5,sticky=W)

    self.data_fwd= Entry(self)
    self.data_fwd.grid(row=2, column=6,sticky=W)

    self.data_min_pwm_l= Entry(self)
    self.data_min_pwm_l.grid(row=2, column=7,sticky=W)
    self.data_min_pwm_r= Entry(self)
    self.data_min_pwm_r.grid(row=2, column=8,sticky=W)

    self.data_encoder_weight_l= Entry(self)
    self.data_encoder_weight_l.grid(row=4, column=7,sticky=W)

    self.data_encoder_weight_r= Entry(self)
    self.data_encoder_weight_r.grid(row=4, column=8,sticky=W)

    self.data_pwm_bias_l= Entry(self)
    self.data_pwm_bias_l.grid(row=6, column=7,sticky=W)

    self.data_pwm_bias_r= Entry(self)
    self.data_pwm_bias_r.grid(row=6, column=8,sticky=W)

    #self.data_integ_thres= Entry(self)
    #self.data_integ_thres.grid(row=2, column=9,sticky=W)
    
    self.data_aP.insert(0,"0")
    self.data_aS.insert(0,"0")
    self.data_aD.insert(0,"0")
    
    self.data_dP.insert(0,"0")
    self.data_dS.insert(0,"0")
    self.data_dD.insert(0,"0")

    self.data_mP.insert(0,"0")
    self.data_mS.insert(0,"0")
    self.data_mD.insert(0,"0")

    self.data_fwd.insert(24,"24")
    #self.data_integ_thres.insert(5,"5")
    self.data_min_pwm_l.insert(4000,"4000")
    self.data_min_pwm_r.insert(4000,"4000")

    self.data_encoder_weight_l.insert(1,"1")
    self.data_encoder_weight_r.insert(1,"1")

    self.data_pwm_bias_l.insert(1,"1")
    self.data_pwm_bias_r.insert(1,"1")

    self.button=Button(self,text="Send Values",command=self.send).grid(row=5, column=2,sticky=W)
    self.start=Button(self,text="START!",command=self.begin).grid(row=5, column=3,sticky=W)
    self.halt=Button(self,text="STOP!",command=self.stop).grid(row=5,column=4,sticky=W)                                                                   
    self.reset=Button(self,text="Reset",command=self.reset).grid(row=5, column=5,sticky=W)
    self.forward=Button(self,text="FWD",command=self.forward).grid(row=3, column=6,sticky=W)
    self.right=Button(self,text="Right",command=self.right).grid(row=4, column=6,sticky=W)
    self.left=Button(self,text="Left",command=self.left).grid(row=5, column=6,sticky=W)
    
    self.left=Button(self,text="Calibrate",command=self.calibrate).grid(row=6, column=6,sticky=W)
    self.scroller=Scrollbar(self)
    self.scroller.grid(row=19,column=1,sticky='nsew')
    self.scroller.config(width='20')


    self.output=Text(self, width=90, height=25, wrap=WORD)
    self.output.grid(row=19, column=2,columnspan=6,sticky='nsew')
    global output
    output = self.output

    self.scroller.config(command=self.output.yview)
    self.output.config(yscrollcommand=self.scroller.set)          

  def send(self):
    """collects and sends the value"""
    values = [
        self.data_aP.get(),
        self.data_aS.get(),
        self.data_aD.get(),
        
        self.data_dP.get(),
        self.data_dS.get(),
        self.data_dD.get(),

        self.data_mP.get(),
        self.data_mS.get(),
        self.data_mD.get(),

	#self.data_integ_thres.get(),
        self.data_min_pwm_l.get(),
        self.data_min_pwm_r.get(),

   	    self.data_encoder_weight_l.get(),
	      self.data_encoder_weight_r.get(),

	      self.data_pwm_bias_l.get(),
	      self.data_pwm_bias_r.get(),
      ]
    values = list(map(lambda x: float(x), values))
    p.write('g', values); 
    
  def calibrate(self):
    global p
    p.write('c')
    
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

  def forward(self):
    """Moves the robot forward by a certain distance""" 
    global p
    fwd_value=self.data_fwd.get()
    p.write('f',float(fwd_value));

  def right(self):
    """Turns the robot right""" 
    global p
    p.write('r');

  def left(self):
    """Turns the robot right""" 
    global p
    p.write('l');

root=Tk()
root.title("PID Control")
root.geometry("1200x1200")

app=Application(root)

root.mainloop()                                                                    

