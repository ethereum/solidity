contract A {
    uint public x;
    // Signature is d88e0b00
    function fow() public { x = 3; }
}
// ----
// () -> FAILURE
// (): hex"d8" -> FAILURE
// (): hex"d88e" -> FAILURE
// (): hex"d88e0b" -> FAILURE
// x() -> 0
// (): hex"d88e0b00"
// x() -> 3
// (): hex"d88e0bff" -> FAILURE
// x() -> 3
