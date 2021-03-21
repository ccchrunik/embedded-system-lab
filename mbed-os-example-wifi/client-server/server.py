#!/usr/bin/env python3

import socket
import json
import numpy
import time
import sys
import os
from datetime import datetime
import matplotlib.pyplot as plt


def main():
    fig, axes = plt.subplots(nrows=3, ncols=2, sharex=True,
                             sharey=True, figsize=(16, 8))

    # HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
    HOST = '192.168.50.252'
    PORT = 30007       # Port to listen on (non-privileged ports are > 1023)
    acce = [[] for _ in range(3)]
    gyro = [[] for _ in range(3)]
    acce_colors = ['r', 'g', 'b']
    gyro_colors = ['c', 'm', 'y']
    acce_labels = ['a_x', 'a_y', 'a_z']
    gyro_labels = ['g_x', 'g_y', 'g_z']
    axis = ['X', 'Y', 'Z']
    array_s = []
    data_count = 0
    error_count = 0
    sample_rate = 0.1
    threshold = True
    init_flag = True
    count_flag = False

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # s.setsockopt(socket.SOL_SOCKET, socket.REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        try:
            with conn:
                print('Connected by', addr)
                while True:
                    try:
                        # for every 300 lines of data (30 seconds), we plot another figure
                        # when we have 250 lines of data, we know that it's about to reinitialize
                        if data_count % 300 > 250:
                            count_flag = True

                        # if the count go back to 0 and it once go up to 550 and it's not initalization state
                        # we recycle all of the resources
                        if data_count % 300 < 50 and count_flag and not init_flag:
                            plt.savefig(f'image/image-{t}.png')
                            fig, axes = plt.subplots(nrows=3, ncols=2, sharex=True,
                                                     sharey=True, figsize=(16, 8))
                            file.close()
                            acce = [[] for _ in range(3)]
                            gyro = [[] for _ in range(3)]
                            array_s = []
                            count_flag = False
                            threshold = True

                        # re-initialization
                        if threshold:
                            now = datetime.now()
                            t = now.strftime("%d::%m::%Y %H:%M:%S")
                            fig.suptitle(f"Motion (start time: {t})")
                            file = open(f'data/data-{t}.txt', 'w+')
                            init_flag = False
                            threshold = False

                        data = conn.recv(1024).decode('utf-8')
                        dec = json.JSONDecoder()
                        pos = 0
                        datas = []
                        # https://stackoverflow.com/questions/36967236/parse-multiple-json-objects-that-are-in-one-line/43807246
                        # decode several json objects in a single line
                        while not pos == len(str(data)):
                            j, json_len = dec.raw_decode(str(data)[pos:])
                            pos += json_len
                            datas.append(j)

                        for data in datas:
                            data_count += 1
                            file.write(json.dumps(data) + '\n')
                            json_data = data
                            # json_data = json.loads(data)
                            array_s.append(json_data['s'] * sample_rate)
                            acce_data = [json_data['a_x'],
                                         json_data['a_y'], json_data['a_z']]
                            gyro_data = [json_data['g_x'] / 400,
                                         json_data['g_y'] / 400, json_data['g_z'] / 400]
                            # put data in the array
                            for i in range(3):
                                acce[i].append(acce_data[i])
                                gyro[i].append(gyro_data[i])

                    except json.decoder.JSONDecodeError:
                        print("JSONDecodeError: more than one set of data!!!")
                        error_count += 1
                        continue

                    # plot acceleration of each direction
                    for i in range(3):
                        ax = axes[i][0]
                        ax.set_ylabel(f'{axis[i]}')
                        ax.plot(array_s, acce[i], color=acce_colors[i])
                    # plot gyro of each direction
                    for i in range(3):
                        ax = axes[i][1]
                        ax.set_ylabel(f'{axis[i]} (0.4K)')
                        ax.plot(array_s, gyro[i], color=gyro_colors[i])

                    # acce metadata
                    axes[0][0].set_title('Acceleration')
                    axes[-1][0].set_xlabel('time')

                    # gyro metadata
                    axes[0][1].set_title('Gyro')
                    axes[-1][1].set_xlabel('time')

                    plt.pause(0.01)
                    print(json_data)

                plt.show()
        except KeyboardInterrupt:
            print('Interrupted')
            plt.savefig(f'image/image-{t}.png')
            file.close()
            print(f"error_count = {error_count}")
            try:
                sys.exit(1)
            except SystemExit:
                os._exit(1)


if __name__ == '__main__':
    main()
