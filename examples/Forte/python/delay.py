import re
import matplotlib.pyplot as plt


class packet:
    num=0
    tx_time = 0
    rx_time=0
    cast=''
    delay=-1

def Proecess(fileServer,fileMN,plt,marker):
    fileServer = iter(fileServer)
    fileMN = iter(fileMN)
    sent_packets = []

    # Strips the newline character
    for line in fileMN:
        if "Start Sending: Unicast" in line or "Start Sending: Broadcast" in line:
            pkt = packet()
            pkt_line=line.split("Start Sending: ")[1]
            pkt.cast = pkt_line.split(' ')[0]
            pkt.num = pkt_line.split(' ')[1].split('\n')[0]

            # if "{" not in tsch_line:
            #     tsch_line = next(fileMN) 
            # # print("pkt_line:"+pkt_line)
            # # print("tsch_line:"+tsch_line)
            # result=tsch_line.split('{')[1].split('}')[0]
            # pkt.tx_time = result.split(' ')[1]
            pkt.tx_time = pkt_line.split(' ')[3].split('\n')[0]
            if pkt.tx_time is None:
                print("None")
            else:
                print("tx:{} {}".format(pkt_line, pkt.tx_time))
            # print("Line: {}".format(pkt_line))
            sent_packets.append(pkt)

    for line in fileServer:
        if "Received request: Broadcast" in line or "Received request: Unicast" in line :
            pkt_line=line.split("Received request:")[1]
            num = pkt_line.split(' ')[2]
            print(num)
            # tsch_line=next(fileServer) 
            # result=tsch_line.split('{')[1].split('}')[0]
            
            for x in sent_packets:
                if x.num == num:
                    x.rx_time = pkt_line.split(' ')[4]
                    print(pkt_line.split(' '))

            # if result is None:
            #     print("None")
            # else:
            #     print("rx:{}".format(result.split(' ')))
            # print("Line: {}".format(pkt_line))
    rx_count = 0
    for i in sent_packets:
        if int(i.num) > 909326381:
            i.num = int(i.num) - 909326388
        if i.rx_time == 0:
            i.delay = -1
            print("i: {} {} {} {}".format(i.num,i.cast, i.tx_time, i.rx_time))    
        else:
            rx_count += 1
            i.delay = int(i.rx_time[3:],16)-int(i.tx_time[3:],16)
            print("i: {} {} {} {} {}".format(i.num,i.cast, i.tx_time, i.rx_time, i.delay))
    pdr = rx_count/len(sent_packets)
    print(pdr)

    return sent_packets,pdr
    

paths = [ "orchestra-lowpower" ]
marker=8
plt.figure(figsize=(12,6))
plt.ylim(0,200)
pdrs=[]

for path in paths:
    marker += 1
    fileServer = open(path+'/server.log', 'r')
    fileMN = open(path+'/MPU.log', 'r')
    sent_packets, pdr = Proecess(fileServer,fileMN,plt,marker)
    x=[pkt.num for pkt in sent_packets]
    y=[pkt.delay for pkt in sent_packets]
    plt.plot(x, y,marker=marker, label=path)
    pdrs.append(pdr)

plt.xlabel('Time (s)')
plt.ylabel('Delay (ms)')
plt.legend()
plt.show()


plt.xlabel('Protocol')
plt.ylabel('Packet Delivery Ratio (%)')
plt.ylim(0.9,1.0)


for path,pdr in zip(paths,pdrs):
    plt.bar(path,pdr)
plt.show()

