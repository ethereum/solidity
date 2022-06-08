contract C {
    bytes x = "012345678901234567890123456789A";
    function test() external returns(uint) {
        (x[0], x.push()) = (0x80,0x42);
        return x.length;
        // used to return 0x4000000000000000000000000000000000000000000000000000000000000020
    }
}
// ----
// test() -> 0x20
