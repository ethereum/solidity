contract c {
    uint[2] dataArray;
    function set5th() public returns (bool) {
        dataArray[5] = 2;
        return true;
    }
}
// ----
// TypeError: (90-102): Out of bounds array access.
