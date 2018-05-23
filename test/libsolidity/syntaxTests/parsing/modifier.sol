contract c {
    modifier mod { if (msg.sender == 0) _; }
}
