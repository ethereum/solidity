contract C {
    bytes20 x;
    function f(bytes16 b) public view {
        b[uint8(x[2])];
    }
}
