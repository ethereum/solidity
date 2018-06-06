contract test {
    function f() pure public returns(uint) {
        uint8 x = 100;
        return uint8(10)**x;
    }
}
