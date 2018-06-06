contract test {
    function f() pure public returns(uint) {
        return 2 << 80;
    }
}
