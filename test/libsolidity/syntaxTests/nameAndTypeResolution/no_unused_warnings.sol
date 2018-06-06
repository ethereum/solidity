contract C {
    function f(uint a) pure public returns (uint b) {
        uint c = 1;
        b = a + c;
    }
}
