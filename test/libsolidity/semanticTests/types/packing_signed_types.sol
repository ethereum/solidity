contract test {
    function run() public returns(int8 y) {
        uint8 x = 0xfa;
        return int8(x);
    }
}
// ----
// run() -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa
