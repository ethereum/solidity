contract c {
    bytes data;

    function test() public returns (bool x) {
        data.push(0x05);
        if (data.length != 1) return true;
        if (data[0] != 0x05) return true;
        data.push(0x04);
        if (data[1] != 0x04) return true;
        data.push(0x03);
        uint256 l = data.length;
        if (data[2] != 0x03) return true;
        if (l != 0x03) return true;
    }
}

// ----
// test() -> false
