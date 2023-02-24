function suffix(uint) pure suffix returns (uint) {}

contract C {
    uint x = 1000 suffix.value;
}
// ----
// TypeError 8820: (84-96): Member "value" is only available for payable functions.
