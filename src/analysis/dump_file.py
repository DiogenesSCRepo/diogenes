import struct

file = open("TF_delta.bin", "rb")
data = file.read()
file.close()

deltalength = int(int(len(data) / 8) / 2)
f = open("testingDump.txt", "w")
for x in range(0, deltalength):
    deltatmp = struct.unpack_from("Qd", data, offset=(x*8*2))
    f.write(str(deltatmp[0]) + "," + str(deltatmp[1]) + "\n")

f.close()