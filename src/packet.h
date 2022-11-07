#ifndef DATA_TRANSFER_PROTOCOLS_PACKET_H
#define DATA_TRANSFER_PROTOCOLS_PACKET_H


struct Packet {
    int sequenceNumber;
    char contents[5000];//MAX_INPUT
    bool valid;
};

#endif //DATA_TRANSFER_PROTOCOLS_PACKET_H
