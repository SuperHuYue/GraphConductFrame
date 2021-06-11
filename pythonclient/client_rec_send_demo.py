
#!/usr/bin/env python
# This tutorial is copy from : https://websockets.readthedocs.io/en/stable/intro.html

import asyncio
import websockets
import cv2
import time
import numpy as np

from websockets.http import Headers


class SysHeader(object):
    """
    docstring:
        using uint32 encode
    """

    def __init__(self):
        super().__init__()
        self.header_size = None
        self.version = None
        # self.from_id =None
        self.body_size = None
        self.unused = (1, 1, 1, 1, 1, 1)
        # self.to_id = list()

    def generate(self):
        # head_size_tmp = 10*4 + len(self.to_id) * 4
        head_size_tmp = 9*4
        header_bytes = (head_size_tmp).to_bytes(4, 'little', signed=False)
        version_bytes = (self.version).to_bytes(4, 'little', signed=False)
        # from_id_bytes = (self.from_id).to_bytes(4, 'little', signed = False)
        body_size_bytes = (self.body_size).to_bytes(4, 'little', signed=False)
        #####
        unused_bytes = bytes()
        for i in self.unused:
            unused_bytes = i.to_bytes(4, 'little', signed=False) + unused_bytes
        # to_id_bytes = bytes()
        # for i in self.to_id:
        #     to_id_bytes = int(i).to_bytes(4, 'little', signed = False) + to_id_bytes
        return header_bytes+version_bytes+body_size_bytes+unused_bytes

    def parse(self, data):
        offset = 4
        self.header_size = int.from_bytes(
            data[0:offset], 'little', signed=False)
        self.version = int.from_bytes(
            data[offset:offset+4], 'little', signed=False)
        offset += 4
        # self.from_id = int.from_bytes(data[offset:offset+4], 'little', signed=False)
        # offset+=4
        self.body_size = int.from_bytes(
            data[offset:offset+4], 'little', signed=False)
        offset += 4
        for i in self.unused:
            i = int.from_bytes(data[offset:offset+4], 'little', signed=False)
            offset += 4
        # to_count = self.header_size - offset
        # for i in range(to_count):
        #     self.to_id.append(int.from_bytes(data[offset:offset+4]))
        #     offset+=4
        #     pass
    pass

#
# Method that extract key value
#


def getKeyValue(para, start='?', div='&', assign='=', split=True):
    keyValuePair = dict()
    start_pos = str.find(para, start)
    if start_pos == -1:
        return False
    tmp_para = para[start_pos + 1:]
    while True:
        key_pos = str.find(tmp_para, assign)
        if key_pos == -1:
            break
        value_pos = str.find(tmp_para, div, key_pos + 1)
        if(value_pos == -1):
            keyValuePair[tmp_para[0:key_pos]] = inner_split(
                tmp_para[key_pos+1:], valid=split)
            break
        key = tmp_para[0:key_pos]
        value = tmp_para[key_pos + 1:value_pos]
        tmp_para = tmp_para[value_pos+1:]
        keyValuePair[key] = inner_split(value, valid=split)
    return keyValuePair


def inner_split(para, split=',', valid=False):
    inner = list()
    if valid is False:
        inner.append(para)
        return inner
    while True:
        pos = str.find(para, split)
        if pos == -1:
            inner.append(para)
            break
        inner.append(para[0:pos])
        para = para[pos + 1:]
        pass
    return inner


async def hello():
    # uri = "ws://192.168.2.75:9106/?Method=ECHO"
    name = input("SetMyName:")
    uri = "ws://localhost:9106/?Alias=" + name
    async with websockets.connect(uri) as websocket:
        websocket.read_limit = 2**64
        sysConnectRecv = await websocket.recv()
        out_pair = getKeyValue(sysConnectRecv)
        got = out_pair.get('SysConnectInit', None)[0]
        if got != "Got":
            # add some wrong operator
            return
        toAlisa = input("ToAlAisa:")

        if toAlisa == "":
            uri = "?SysSetMethod=ImgRetransmission"
        else:
            uri = "?SysSetMethod=ImgRetransmission&FromAlisa="+name+"&ToAlisa="+toAlisa
        await websocket.send(uri)
        data = await websocket.recv()
        while True:
            img_data_binary = await websocket.recv()
            if len(img_data_binary) <= 52:
                continue
            img_data_binary_np = np.frombuffer(img_data_binary, np.uint8)
            img = cv2.imdecode(img_data_binary_np, cv2.IMREAD_COLOR)
        #     header_size = int.from_bytes(img_data[0:4], 'little', signed=False)
        #     version = int.from_bytes(img_data[4:8], 'little', signed=False)
        #     body_size = int.from_bytes(img_data[12:16], 'little', signed=False)
        #     img_bytes = img_bytes[52:]
        #     img_bytes = cv2.imdecode(img_bytes, cv2.COLOR_YUV2BGR)
           # //cv2.imwrite("hei.jpeg", img_yuv)
           # //img_binary = None
           # #transfer raw pic is very big so we first compress it
           # with open("hei.jpeg", "rb") as img_handle:
           #     img_binary = img_handle.read()
           #     pass
            if img.dtype.name != 'uint8':
                break
            if img.data.contiguous is not True:
                break
            shape = img.shape
            row = int.to_bytes(shape[0], 4, 'little', signed=False)
            col = int.to_bytes(shape[1], 4, 'little', signed=False)
            deepth = int.to_bytes(shape[2], 4, 'little', signed=False)
            pic_data = img_data_binary
            pic_len = int.to_bytes(len(pic_data), 4, 'little', signed=False)
            final_body_data = row + col + deepth + pic_len + pic_data
            # add header
            header = SysHeader()
            # header.from_id = 10086
            header.version = 1
            header.body_size = len(final_body_data)
            # header.to_id.append(int(id_remote))#just try a single to_id
            # header.to_id = 95527

            out = header.generate()
            final_data = out + final_body_data

            # this place must have a sleep and i don't know why yet
            time.sleep(0.03)
            await websocket.send(final_data)
            pass

asyncio.get_event_loop().run_until_complete(hello())
