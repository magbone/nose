
import socket, binascii, random, struct

# stun attributes
MappedAddress = '0001'
ResponseAddress = '0002'
ChangeRequest = '0003'
SourceAddress = '0004'
ChangedAddress = '0005'
Username = '0006'
Password = '0007'
MessageIntegrity = '0008'
ErrorCode = '0009'
UnknownAttribute = '000A'
ReflectedFrom = '000B'
XorOnly = '0021'
XorMappedAddress = '8020'
ServerName = '8022'
SecondaryAddress = '8050'  # Non standard extension

dict_val_to_attr = {
      '0001': 'MappedAddress',
      '0002': 'ResponseAddress',
      '0003': 'ChangeRequest',
      '0004': 'SourceAddress', 
      '0005': 'ChangedAddress',
      '0006': 'Username',
# Password = '0007'
# MessageIntegrity = '0008'
# ErrorCode = '0009'
# UnknownAttribute = '000A'
# ReflectedFrom = '000B'
# XorOnly = '0021'
      '8020': 'XorMappedAddress',
      '8022': 'ServerName'
# SecondaryAddress = '8050'  # Non standard extension
}
def rsp_packet_unpacked(data: bytes):
      msg_type, msg_length, msg_magic, tran_id = struct.unpack('>2sh4s12s', data[:20])
      msg_type = binascii.b2a_hex(msg_type).decode('ascii')
      tran_id = binascii.b2a_hex(tran_id).decode('ascii')
      msg_magic = binascii.b2a_hex(msg_magic).decode('ascii')
      print(msg_type, msg_length, msg_magic, tran_id)
      print('----------------------------------------------')
      base = 20
      while base < len(data):
            attr_type = binascii.b2a_hex(data[base : base + 2]).decode('ascii')
            attr_len = struct.unpack('>H', data[base + 2 : base + 4])[0]
            if attr_type == MappedAddress:
                  # print(struct.unpack('>bBH4s', data[base + 4 : base + 4 + attr_len]))
                  reserved, protocol_family, port, ip = struct.unpack('>bBH4s', data[(base + 4) : (base + 4 + attr_len)])

                  print('Attribute Type: Mapped Address')
                  print('Attribute Length: %d' % (attr_len))
                  print('Port Family: %d' % (protocol_family))
                  print('Port: %d' %  (port))
                  print('IP:' + '.'.join([str(int(i)) for i in ip]))
            elif attr_type == SourceAddress:
                  reserved, protocol_family, port, ip = struct.unpack('>bBH4s', data[base + 4 : base + 4 + attr_len])
                  
                  print('Attribute Type: Source Address')
                  print('Attribute Length: %d' % (attr_len))
                  print('Port Family: %d' % (protocol_family))
                  print('Port: %d' %  (port))
                  print('IP:' + '.'.join([str(int(i)) for i in ip]))
                 
            elif attr_type == ChangedAddress:
                  reserved, protocol_family, port, ip = struct.unpack('>bBH4s', data[base + 4 : base + 4 + attr_len])
                  print('Attribute Type: Changed Address')
                  print('Attribute Length: %d' % (attr_len))
                  print('Port Family: %d' % (protocol_family))
                  print('Port: %d' %  (port))
                  print('IP:' + '.'.join([str(int(i)) for i in ip]))
                  
            elif attr_type == XorMappedAddress:
                  reserved, protocol_family, port, ip = struct.unpack('>bBH4s', data[base + 4 : base + 4 + attr_len])
                  print('Attribute Type: XOR Mapped Address')
                  print('Attribute Length: %d' % (attr_len))
                  print('Port Family: %d' % (protocol_family))
                  print('Port: %d' %  (port))
                  print('IP:' + '.'.join([str(int(i)) for i in ip]))
                 
            elif attr_type == ServerName:
                  print('Attribute Type: Software')
                  print('Attribute Length: %d' % (attr_len))
                  print("Software: " + (data[base + 4 : base + 4 +attr_len]).decode('ascii'))
                  
            base += (4 + attr_len)
            print('----------------------------------------------')

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('23.224.186.33', 46786))
sock.settimeout(2)
print('----------------------------------------------')
print(sock.getsockname())
print('----------------------------------------------')

data = b'\x00\x01\x00\x00\x21\x12\xA4\x42' + binascii.a2b_hex(''.join(random.choice('0123456789ABCDEF') for i in range(24)))
print(b'[Send]: ' + data)
sock.sendto(data, ('216.93.246.18', 3478))
while True:
      try:
            recv, _ = sock.recvfrom(1024)
            print(b'[Recv]:' + recv)
            rsp_packet_unpacked(recv)
      except socket.timeout:
            print('Timeout, resend')
            sock.sendto(data, ('216.93.246.18', 3478))
      else:
            break


change_req_atr = b'\x00\x03\x00\x04\x00\x00\x00\x06'
data = b'\x00\x01' + binascii.a2b_hex("%#04d" % (len(change_req_atr))) + b'\x21\x12\xA4\x42' \
+ binascii.a2b_hex(''.join(random.choice('0123456789ABCDEF') for i in range(24))) + change_req_atr

print(b'[Send]:' + data)
sock.sendto(data, ('216.93.246.18', 3478))

while True:
      try:
            recv, _ = sock.recvfrom(1024)
            print(b'[Recv]:' + recv)
            rsp_packet_unpacked(recv)
      except socket.timeout:
            print('Timeout, resend')
            sock.sendto(data, ('216.93.246.18', 3478))
      else:
            break

sock.close()