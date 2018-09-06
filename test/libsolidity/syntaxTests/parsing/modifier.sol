contract c {
    modifier mod { if (msg.sender == 0x0000000000000000000000000000000000000000) _; }
}
