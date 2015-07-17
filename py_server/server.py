#!/usr/bin/python

import sys;
import db;
import socket_manage;
import message;
from client import *;
import struct;

clients = [];
num_clients = 0;
client_id_inc = 0;

def client_accepted(client_sock, client_ip, client_port):
    global clients
    global num_clients
    global client_id_inc
    
    c = Client();
    c.sock = client_sock;
    c.id = client_id_inc;
    c.ip = client_ip;
    c.port = client_port;

    clients.append(c);

    print("accepted client (client-id: %d, ip: %s, port: %d)" % (c.id, c.ip, c.port));

    num_clients += 1;
    client_id_inc += 1;
    c.sock.send("ayyyyyyyyyyyyyyyyyy2");
    #message.send(c.sock, message.MID_CLIENT_ID, c.id, "test");
    c.sock.send("second msg lol");

def client_disconnected(sock):
    global clients
    global num_clients

    for c in clients:
        if (c.sock == sock):
            print("client disconnected (client-id: %d, ip: %s, port: %d)" % (c.id, c.ip, c.port));
            clients.remove(c);
            num_clients -= 1;
            break;

def got_message(byte_data):
    mid = message.extract_mid(byte_data);
    if (mid != message.MID_UNKNOWN):
        (params, err) = message.extract_params(mid, byte_data);
        if (err != -1):
            if (mid == message.MID_CLIENT_USER_PASS):
                print("username: %s, password: %s" % (params[0], params[1]));
                db.add_user_account(params[0], params[1]);
    else:
        print("recv message has an unknown MID");

if __name__ == "__main__":
    db.init();
    socket_manage.init();
    socket_manage.listen("0.0.0.0", 4222);