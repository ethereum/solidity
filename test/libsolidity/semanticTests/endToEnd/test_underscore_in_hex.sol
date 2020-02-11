contract test {
    function f(bool cond) public pure returns(uint) {
        uint32 x = 0x1234 _ab;
        uint y = 0x1234 _abcd_1234;
        return cond ? x : y;
    }
}

// ----
// f(bool): true -> 0x1234ab
// f(bool):"1" -> "1193131"
// f(bool): false -> 0x1234abcd1234
// f(bool):"0" -> "20017429942836"
