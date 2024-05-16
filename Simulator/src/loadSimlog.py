
import numpy as np


def readnullstring(f):
    chars = bytearray()
    while True:
        c = f.read(1)
        if c == b'\x00':
            return chars.decode()
        chars+=c


def loadSimlog(file):
    f=open(file,"rb")
    nD=np.fromfile(f,dtype=np.int32,count=1)[0]
    nB=np.fromfile(f,dtype=np.int32,count=1)[0]
    baselines=np.empty(dtype=np.dtype([('name','U32'),('dl-',np.int32),
                                       ('dl+',np.int32),('nL',np.int32),
                                       ('L0',np.float64),
                                       ('L1',np.float64)]),shape=(nB))
    for i in range(0,nB):
        baselines[i]['name']=readnullstring(f)
        baselines[i]['dl-']=np.fromfile(f,dtype=np.int32,count=1)[0]
        baselines[i]['dl+']=np.fromfile(f,dtype=np.int32,count=1)[0]
        baselines[i]['nL']=np.fromfile(f,dtype=np.int32,count=1)[0]
        baselines[i]['L0']=np.fromfile(f,dtype=np.float64,count=1)[0]
        baselines[i]['L1']=np.fromfile(f,dtype=np.float64,count=1)[0]
    nv2=np.fromfile(f,dtype=np.float64,count=nB)

    recs=np.fromfile(f,dtype=np.dtype([('s',np.uint32),('ns',np.uint32),
                                       ('nn',np.float64),('n',np.float64,nB),
                                       ('nv',np.float64,nB),
                                       ('nv2',np.float64,nB),
                                       ('delay',np.float64,nD),
                                       ('ext_delay',np.float64,nD)]))

    f.close()

    return {'nD':nD,'nB':nB,'baselines':baselines,'nv2':nv2,'recs':recs}
