function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = 1000 suffix();
}
// ----
// TypeError 5704: (100-113): Type is not callable
