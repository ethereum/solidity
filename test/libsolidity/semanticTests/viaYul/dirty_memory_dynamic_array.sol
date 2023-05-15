contract C {
    function f() public pure returns (bool correct) {
        uint8[] memory m = new uint8[](1);
        assembly {
            mstore(add(m, 32), 258)
        }
        uint8 x = m[0];
        uint r;
        assembly {
            r := x
        }
        correct = (m[0] == 0x02) && (r == 0x02);
    }
}
// ====
// compileViaYul: true
// ----
// f() -> true
