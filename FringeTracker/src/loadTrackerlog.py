
import numpy as np

def readnullstring(f):
    chars = bytearray()
    while True:
        c = f.read(1)
        if c == b'\x00':
            return chars.decode()
        chars += c


def loadTrackerlog(file):
    f=open(file,"rb")
    nB=np.fromfile(f,dtype=np.int32,count=1)[0]
    nD=np.fromfile(f,dtype=np.int32,count=1)[0]
    nS=np.fromfile(f,dtype=np.int32,count=1)[0]
    bnames=np.chararray(nB,itemsize=32)
    for i in range(0,nB):
        bnames[i]=readnullstring(f)
    snames=np.chararray(nS,itemsize=32)
    for i in range(0,nS):
        snames[i]=readnullstring(f)
    recs=np.fromfile(f,dtype=np.dtype([('s',np.uint32),('ns',np.uint32),
                                       ('b',np.dtype([('s',np.uint8),
                                                      ('snrf',np.float32),
                                                      ('snrl',np.float32),
                                                      ('snr',np.float32)]),nB),
                                       ('d',np.float32,nD)]))
    
    f.close()
    return {'nB':nB,'nD':nD,'nS':nS,'bnames':bnames,'snames':snames,
            'recs':recs}

    
