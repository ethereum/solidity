contract c {
    uint[] data;

    function test() public returns(uint x, uint l) {
        data.push(7);
        data.push(3);
        x = data.length;
        data.pop();
        x = data.length;
        data.pop();
        l = data.length;
    }
}

// ----
// test() -> 1, 0
