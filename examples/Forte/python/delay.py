import re

fileServer = open('server.log', 'r')
fileServer = iter(fileServer)

fileMN = open('MPU.log', 'r')
fileMN = iter(fileMN)

class packet:
    num=0
    tx_time = 0
    rx_time=0
    cast=''

sent_packets = []

# Strips the newline character
for line in fileMN:
    if "Done Sending:" in line:
        pkt = packet()
        pkt_line=line.split("Done Sending: ")[1]
        pkt.cast = pkt_line.split(' ')[0]
        pkt.num = pkt_line.split(' ')[1].split('\n')[0]
        tsch_line=next(fileMN) 
        result=tsch_line.split('{')[1].split('}')[0]
        pkt.tx_time = result.split(' ')[1]
        print(tsch_line)
        if result is None:
            print("None")
        else:
            print("tx:{} {}".format(pkt_line, result))
        # print("Line: {}".format(pkt_line))
        sent_packets.append(pkt)



for line in fileServer:
    if "Received request" in line:

        pkt_line=line.split("\'")[1]
        num = pkt_line.split(' ')[1]
        # print(num)
        tsch_line=next(fileServer) 
        result=tsch_line.split('{')[1].split('}')[0]
        
        
        for x in sent_packets:
            if x.num == num:
                x.rx_time = result.split(' ')[1]
                print(result.split(' '))

        # if result is None:
        #     print("None")
        # else:
        #     print("rx:{}".format(result.split(' ')))
        # print("Line: {}".format(pkt_line))

for i in sent_packets:
    if i.rx_time == 0:
        print("i: {} {} {} {}".format(i.num,i.cast, i.tx_time, i.rx_time))    
    else:
        print("i: {} {} {} {} {}".format(i.num,i.cast, i.tx_time, i.rx_time, int(i.rx_time[3:],16)-int(i.tx_time[3:],16)))