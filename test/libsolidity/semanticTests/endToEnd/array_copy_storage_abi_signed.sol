contract c {
    int16[] x;

    function test() public returns(int16[] memory) {
        x.push(int16(-1));
        x.push(int16(-1));
        x.push(int16(8));
        x.push(int16(-16));
        x.push(int16(-2));
        x.push(int16(6));
        x.push(int16(8));
        x.push(int16(-1));
        return x;
    }
}

// ----
// test() -> 0x20, 8, -1, -1, 8, -16, -2, 6, 8, -1 
