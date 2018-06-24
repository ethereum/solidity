contract C {
    uint constant x = 2;
    function k() pure public returns (uint) {
        return x;
    }
}
