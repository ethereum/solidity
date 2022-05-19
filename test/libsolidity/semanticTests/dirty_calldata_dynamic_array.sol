contract C {
    function f(int16[] calldata a) external returns (bool correct) {
        uint32 x = uint32(uint16(a[1]));
        uint r;
        assembly {
            r := x
        }
        correct = r == 0x7fff;
    }
}
// ----
// f(int16[]): 0x20, 0x02, 0x7fff, 0x7fff -> true
