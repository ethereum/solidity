// NOTE: This does not really test copying from storage to ABI directly,
// because it will always copy to memory first.
contract c {
    int16[] x;

    function test() public returns (int16[] memory) {
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

// ====
// compileViaYul: also
// ----
// test() -> 0x20, 0x8, -1, -1, 8, -16, -2, 6, 8, -1
