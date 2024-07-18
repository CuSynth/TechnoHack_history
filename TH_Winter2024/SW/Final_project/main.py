from python_tcp.client import SocketClient
import crc
import struct
import time
from sys import getsizeof



def my_handler(data: bytearray):

    massage = read_massage(data)
    info_packet = last_packet(massage)
    if info_packet is None:
       return 0, 0, 0, 0, 0, 0, 0, 0
    presure, temperature, humidity,longitude, latitude, hours, minuts, seconds = info_parsing(info_packet)

    hours = int.from_bytes(hours, "big")
    minuts = int.from_bytes(minuts, "big")
    seconds = int.from_bytes(seconds, "big")
    presure = struct.unpack('<f', presure)
    temperature = struct.unpack('<f', temperature)
    humidity = struct.unpack('<f', humidity)
    longitude = int.from_bytes(longitude, "little", signed=True)/600000
    latitude = int.from_bytes(latitude, "little", signed=True)/600000

    # print(f'{info_packet}\n')
    # print(f'Широта, Долгота = {int.from_bytes(latitude, "big")}, {int.from_bytes(longitude, "big")}\n'
    #   f'Часы:мнуты:секунды = {int.from_bytes(hours, "big")}:{int.from_bytes(minuts, "big")}:{int.from_bytes(seconds, "big")}\n'
    #   f'Давление = {struct.unpack('<f', presure)}\n' #{int.from_bytes(presure, "big")}\n
    #   f'Температура = {struct.unpack('<f', temperature)}\n'  #{int.from_bytes(temperature, "big")}\n
    #   f'Влажность = {struct.unpack('<f', humidity)}\n')# {int.from_bytes(humidity, "big")}\n)
    return presure, temperature, humidity,longitude, latitude, hours, minuts, seconds
    #print(f"Ответ от сервера: {info_packet}")ss
    # print(f"Ответ от сервера: {data.hex(' ').upper()}")

def info_parsing(info):
  sizes: list[int] = [1,1,4,4,4,1,4,4,1,1,1,1]
  info_array = split_bytearray(info, sizes)

  start_byte = info_array[0]
  sensor_id = info_array[1]
  presure = info_array[2]
  temperature = info_array[3]
  humidity = info_array[4]
  gps_id = info_array[5]
  longitude = info_array[6]
  latitude = info_array[7]
  hours = info_array[8]
  minuts = info_array[9]
  seconds = info_array[10]
  stop_byte = info_array[11]


  return presure, temperature, humidity,longitude, latitude, hours, minuts, seconds


def calculate_crc(data: bytes) -> int:
    return crc.Calculator(crc.Crc8.CCITT.value).checksum(data)


def short_telemetry(info):
    sizes = [1,2,2,2,4,4,1]
    p=split_bytearray(info, sizes)
    label = p[0]
    mim_status = p[1]
    err_status = p[2]
    rx_fifo_cnt = p[3]
    rx_total_cnt=p[4]
    tx_total_cnt = p[5]
    crc = p[6]

    print(f'label = {int.from_bytes(label, "little")}\n'
      f'mim_status = {int.from_bytes(mim_status, "little")}\n'
      f'err_status =  {int.from_bytes(err_status, "little")}\n'
      f'rx_fifo_cnt ={int.from_bytes(rx_fifo_cnt, "little")}\n'
      f'rx_total_cnt = {int.from_bytes(rx_total_cnt, "little")}\n'
      f'tx_total_cnt = {int.from_bytes(tx_total_cnt, "little")}\n'
      f'crc = {int.from_bytes(crc, "little")}\n')
    print(info)

def last_packet(info) -> bytearray:
  sizes_one: list[int] = [48,2]
  p = split_bytearray(info, sizes_one)

  size_info: int = int.from_bytes(p[1],'little')


  sizes: list[int] = [2,2,2,4,4,1,1,4,1,1,1,1,1,1,4,4,4,4,4,2,2, size_info]
  a = split_bytearray(info, sizes)

  packet_type = a[0]
  packet_size = a[1]
  tot_size = a[2]
  freq_hz = a[3]
  freq_offs = a[4]
  if_chan = a[5]
  status = a[6]
  count_us = a[7]
  dev_id = a[8]
  modem_id = a[9]
  modulation = a[10]
  bw = a[11]
  sf = a[12]
  cr = a[13]
  rssic = a[14]
  rssis = a[15]
  snr = a[16]
  snr_min = a[17]
  snr_max = a[18]
  crc = a[19]
  size = a[20]
  size = int.from_bytes(size, byteorder="little")
  if(size==0):
    return None
  #pad = a[21]
  payload = a[21]
  return payload

def split_bytearray(ba, sizes) -> list:
    parts = []
    offset = 0
    for size in sizes:
        parts.append(ba[offset:offset+size])
        offset += size
    return parts


def create_massage(info) -> bytearray:
    start_byte = 0xF0
    address_send = 0x00
    address_recip = 0x0C
    flags = 4
    info_size: bytes = struct.pack('>H',len(info))
    crc_data = bytearray([address_recip,address_send, flags])+info_size + info
    kc: int = calculate_crc(crc_data)
    stop_byte = 0x0F

    # massage:bytearray  = bytearray([start_byte,address_recip,address_send, flags, info_size, info.decode('utf-16'), kc, stop_byte],'utf-16')
    massage:bytearray  = bytearray([start_byte,address_recip,address_send, flags])+info_size + info + bytearray([ kc, stop_byte])


    return massage

# def read_massage(answer:bytearray,answer_size) -> bytearray:
#     a: bytearray = answer
#     sizes = [1,1,1,1,2,1,answer_size,1]
#     p=split_bytearray(a, sizes)

#     start_byte = p[0]
#     address_recip = p[1]
#     address_send = p[2]
#     flags = p[3]
#     info_size = p[4]
#     id_comand = p[5]
#     info = p[6]
#     stop_byte = p[7]

#     return info

def read_massage(answer:bytearray):
    a: bytearray = answer
    sizes_one = [1,1,1,1,2]
    i = split_bytearray(a, sizes_one)

    info_size = int.from_bytes(i[4],'little')

    sizes = [1,1,1,1,2,1,info_size,1]
    p=split_bytearray(a, sizes)

    start_byte = p[0]
    address_recip = p[1]
    address_send = p[2]
    flags = p[3]
    #info_size = p[4]
    id_comand = p[5]
    info = p[6]
    stop_byte = p[7]

    return info

def client_server():

    try:
        client.connect()
        #while True:
        in_data = bytearray([22]) #str = input('>')
        client.send(create_massage(in_data))
        time.sleep(5)
    except KeyboardInterrupt:
        client.disconnect()
        print('shutdown')

# if __name__ == "__main__":
#     client = SocketClient("10.6.1.127", 7777)
#     # client = SocketClient("localhost", 7777)

#     client.received.subscribe(my_handler)
#     client.transmited.subscribe(my_handler)
#     client.connected.subscribe(lambda: print('Connected'))
#     client.disconnected.subscribe(lambda: print('Discnnected'))
#     try:
#         client.connect()
#         #while True:
#         in_data = bytearray([22]) #str = input('>')
#         client.send(create_massage(in_data))
#         time.sleep(5)
#     except KeyboardInterrupt:
#         client.disconnect()
#         print('shutdown')