contract test {
    function run() public returns(uint256 y) {
        int64 x = -int32(0xff);
        if (x >= 0xff) return 0;
        return -uint256(x);
    }
}

// ----
// run():  -> 0xff
