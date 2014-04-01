import serial
import struct

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
    return cnt

  def read(self,size = 1): 
    return str(self.serial.read(size),'ascii')
