contract C {
    function f() public pure {
        bytes.concat(0, -0, 0.0, -0.0, 0e10, 0e-10, (0), 0x00, hex"00", unicode"abc", "abc");
    }
}
// ----
