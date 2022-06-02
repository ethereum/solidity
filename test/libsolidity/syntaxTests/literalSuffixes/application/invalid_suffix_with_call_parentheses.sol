function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = 1000 suffix();
}
// ----
// TypeError 5704: (93-106): Type is not callable
