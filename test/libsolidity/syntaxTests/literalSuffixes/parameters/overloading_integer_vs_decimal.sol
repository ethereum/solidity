function suffix256(uint) pure suffix returns (bool) {}
function suffix256(uint, uint) pure suffix returns (address) {}

function suffix8(uint) pure suffix returns (bool) {}
function suffix8(uint8, uint) pure suffix returns (address) {}

contract C {
    // Not ambiguous: no way to convert 1.1 into uint.
    address a = 1.1 suffix256;

    // Not ambiguous: 1024 won't fit into uint8.
    bool b = 1024 suffix8;
}
// ----
// TypeError 2144: (325-334): No matching declaration found after variable lookup.
