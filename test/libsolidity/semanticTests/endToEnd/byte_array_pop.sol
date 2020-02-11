contract c {
    bytes data;

    function test() public returns(uint x, uint y, uint l) {
        data.push(0x07);
        data.push(0x03);
        x = data.length;
        data.pop();
        data.pop();
        data.push(0x02);
        y = data.length;
        l = data.length;
    }
}

// ----
// test() -> 2, 1, 1
