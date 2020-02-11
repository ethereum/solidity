contract c {
    bytes data;

    function test() public returns(bytes memory) {
        for (uint i = 0; i < 34; i++)
            data.push(0x03);
        data.pop();
        return data;
    }
}

// ----
// test() ->  0x20, 33, hex"0303030303030303030303030303030303030303030303030303030303030303", hex"03" 
// test():"" -> "32, 33, \x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0, \x0"
