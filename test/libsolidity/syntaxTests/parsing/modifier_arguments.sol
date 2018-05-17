contract c {
    modifier mod(address a) { if (msg.sender == a) _; }
}
