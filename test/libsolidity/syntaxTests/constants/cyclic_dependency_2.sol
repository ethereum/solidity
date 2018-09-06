contract C {
    uint constant a = b * c;
    uint constant b = 7;
    uint constant c = b + uint(keccak256(abi.encodePacked(d)));
    uint constant d = 2 + a;
}
// ----
// TypeError: (17-40): The value of the constant a has a cyclic dependency via c.
// TypeError: (71-129): The value of the constant c has a cyclic dependency via d.
// TypeError: (135-158): The value of the constant d has a cyclic dependency via a.
