contract C {
    uint constant a = b * c;
    uint constant b = 7;
    uint constant c = b + uint(keccak256(abi.encodePacked(d)));
    uint constant d = 2 + a;
}
// ----
// TypeError 5210: (39-40): Cyclic constant definition (or maximum recursion depth exhausted).
