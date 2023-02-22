contract A {
    uint public x;
    // Signature is d88e0b00
    function fow() public { x = 3; }
    fallback () external { x = 2; }
}
// ----
// (): hex"d88e0b"
// x() -> 2
// (): hex"d88e0b00"
// x() -> 3
// (): hex"d88e"
// x() -> 2
// (): hex"d88e0b00"
// x() -> 3
// (): hex"d8"
// x() -> 2
