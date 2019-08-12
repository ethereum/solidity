contract test {
    function f(bool cond) public returns (uint) {
        uint8 x = 0xcd;
        uint16 y = 0xabab;
        return cond ? x : y;
    }
}
// ----
// f(bool): true -> 0xcd
// f(bool): false -> 0xabab
