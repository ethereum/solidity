function suffix8(uint) pure suffix returns (bool) {}
function suffix8(uint8, uint) pure suffix returns (address) {}

contract C {
    bool b = 1024 suffix8;
}
// ----
// TypeError 2144: (148-155): No matching declaration found after variable lookup.
