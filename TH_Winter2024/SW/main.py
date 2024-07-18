from python_tcp.client import *
from marathon_iot.api import MarathonIoT_API
import crc
import struct
from enum import Enum
from tabulate import tabulate
from typing import Generator, Literal, Any

class FrameType(Enum):
    REQUEST = 0x04
    ANSWER = 0x08
    RECEIPT = 0x10
    CMD = 0x12

def get_pack(rx_addr: int, tx_addr: int, flag: FrameType,
                data: bytes) -> None:
    _id: int = data[0]
    size: int = len(data)
    frame: bytes = struct.pack('>BBBH', rx_addr, tx_addr,
                                flag.value, size) + data
    crc8: int = crc.Calculator(crc.Crc8.CCITT.value).checksum(frame)

    frame: bytes = struct.pack('>BBBBH', 0xF0, rx_addr, tx_addr,
                                   flag.value, size)
    return frame + data + struct.pack('>BB', crc8, 0x0F)


def short_tmi(rx_addr=0x0C, tx_addr=0x00, flag=FrameType.REQUEST):
    _id = 0
    return get_pack(rx_addr, tx_addr, flag, struct.pack('<B', _id))

def tmi(rx_addr=0x0C, tx_addr=0x00, flag=FrameType.REQUEST):
    _id = 1
    return get_pack(rx_addr, tx_addr, flag, struct.pack('<B', _id))

def req_rec_cfg(rx_addr=0x0C, tx_addr=0x00, flag=FrameType.CMD, module_num=10):
    _id = 19
    data: bytes = struct.pack('<BHHBB', _id, 0x11, 6, module_num, 0)
    return get_pack(rx_addr, tx_addr, flag, data)

def read_rec_cfg(rx_addr=0x0C, tx_addr=0x00, flag=FrameType.REQUEST, module_num=10):
    _id = 20
    return get_pack(rx_addr, tx_addr, flag, struct.pack('<B', _id))

def _set_settings(cmd_num:int, parameters: bytes, rx_addr=0x0C, tx_addr=0x00, flag=FrameType.CMD) -> bytes:
    _id = 3
    HEADER = 0xABBC
    data: bytes = struct.pack('<BHH', _id, HEADER, cmd_num) + parameters
    return get_pack(rx_addr, tx_addr, flag, data)


def set_power_ch(modules_power_state: list[bool] | list[Literal[0, 1]]):
    val: int = sum([b << i for i, b in enumerate(modules_power_state)])
    return _set_settings(1, struct.pack('<H', val) + bytes(10))




api = MarathonIoT_API()

def my_handler(data: bytes):
    frame = api.parse_raw(data)
    print(tabulate(frame[0], headers = 'keys', tablefmt = 'psql'))

    # file1 = open("log.txt", "a")
    # file1.write(data.hex(' ').upper())
    # file1.write("\n")
    # file1.close()

    # print("Got data: ", data, "\n")

if __name__ == "__main__":
    # pack = "F0 00 0C 08 00 11 00 F1 01 00 10 00 38 00 41 15 00 00 00 00 00 00 60 2D 0F"
    # cont = bytearray.fromhex(pack)
    # print(tabulate(api.parse_raw(cont)[0]))

    if 0:
        print("Short tmi:")
        print("Def: ", api.request_short_telemetry().hex(' ').upper())
        print("     ", short_tmi().hex(' ').upper())
        print()

        print("Tmi:")
        print("Def: ", api.request_telemetry().hex(' ').upper())
        print("     ", tmi().hex(' ').upper())
        print()

        print("Req rec cfg:")
        print("Def: ", api.request_receiver_config(10).hex(' ').upper())
        print("     ", req_rec_cfg().hex(' ').upper())
        print()

        print("Read rec cfg:")
        print("Def: ", api.read_receiver_config().hex(' ').upper())
        print("     ", read_rec_cfg().hex(' ').upper())
        print()

        print("Params:")
        print("Def: ", api.set_settings_power_state([True for _ in range(13)]).hex(' ').upper())
        print("     ", set_power_ch([True for _ in range(13)]).hex(' ').upper())
        print()

        print("Params:")
        print("Def: ", api.read_received_msg().hex(' ').upper())
        # print("     ", set_power_ch([True for _ in range(13)]).hex(' ').upper())
        print()



        # config = SX1302_Settings(state=True, channels=tuple(Channel(True, i * 100) for i in range(8)),
        #                 service_ch=ServiceChannel(True, 500, 1, 2), central_freq=868_000_000, sync=1, ldro=False)
        # write_config = api.write_receiver_config(10, (config, config, config, config))



    else:
        client = SocketClient("10.6.1.127", 7777)
        # client = SocketClient("localhost", 4000)
        client.received.subscribe(my_handler)
        client.transmited.subscribe(my_handler)
        client.connected.subscribe(lambda: print('Connected'))
        client.disconnected.subscribe(lambda: print('Discnnected'))
        try:
            client.connect()
            time.sleep(1)

            # while True:
            # data = short_tmi()
            # client.send(data)
            # time.sleep(2)

            # data = tmi()
            # client.send(data)
            # time.sleep(2)

            # data = api.read_received_msg()
            # client.send(data)
            # time.sleep(2)

            for _ in range(63):
                data = short_tmi()
                client.send(data)
                time.sleep(0.5)

                # data = api.read_received_msg()
                # client.send(data)
                # time.sleep(0.5)


            # --------------------------------------------------------------------------------
            if 0:
                data = req_rec_cfg()
                client.send(data)
                time.sleep(2)

                data = read_rec_cfg()
                client.send(data)
                time.sleep(2)

                data = set_power_ch([True for _ in range(13)])
                client.send(data)
                time.sleep(2)
                
                data = api.set_settings_default(10)
                client.send(data)
                time.sleep(10)

                data = tmi()
                client.send(data)
                time.sleep(2)

                data = req_rec_cfg()
                client.send(data)
                time.sleep(2)

                data = read_rec_cfg()
                client.send(data)
                time.sleep(2)


        except KeyboardInterrupt:
            client.disconnect()
            print('shutdown')

