function copy(uint dstOffset, uint srcOffset, uint length) pure returns (bytes memory out) {
    out =
        hex"2222222222222222333333333333333344444444444444445555555555555555"
        hex"6666666666666666777777777777777788888888888888889999999999999999"
        hex"aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd";
    assembly {
        mcopy(add(add(out, 0x20), dstOffset), add(add(out, 0x20), srcOffset), length)
    }
}

contract C {
    function mcopy_to_right_overlap() public pure returns (bytes memory) {
        return copy(0x20, 0x10, 0x30);
    }

    function mcopy_to_left_overlap() public pure returns (bytes memory) {
        return copy(0x10, 0x20, 0x30);
    }

    function mcopy_in_place() public pure returns (bytes memory) {
        return copy(0x10, 0x10, 0x40);
    }

    function mcopy_to_right_no_overlap() public pure returns (bytes memory) {
        return copy(0x30, 0x10, 0x20);
    }

    function mcopy_to_left_no_overlap() public pure returns (bytes memory) {
        return copy(0x10, 0x30, 0x20);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// mcopy_to_right_overlap()    -> 0x20, 0x60, 0x2222222222222222333333333333333344444444444444445555555555555555, 0x4444444444444444555555555555555566666666666666667777777777777777, 0x88888888888888889999999999999999ccccccccccccccccdddddddddddddddd
// mcopy_to_left_overlap()     -> 0x20, 0x60, 0x2222222222222222333333333333333366666666666666667777777777777777, 0x88888888888888889999999999999999aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd
// mcopy_in_place()            -> 0x20, 0x60, 0x2222222222222222333333333333333344444444444444445555555555555555, 0x6666666666666666777777777777777788888888888888889999999999999999, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd
// mcopy_to_right_no_overlap() -> 0x20, 0x60, 0x2222222222222222333333333333333344444444444444445555555555555555, 0x6666666666666666777777777777777744444444444444445555555555555555, 0x66666666666666667777777777777777ccccccccccccccccdddddddddddddddd
// mcopy_to_left_no_overlap()  -> 0x20, 0x60, 0x2222222222222222333333333333333388888888888888889999999999999999, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb88888888888888889999999999999999, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd
