function suffix256(uint) pure returns (uint) {}
function suffix256(uint, uint) pure returns (int) {}

function suffix8(uint) pure returns (uint8) {}
function suffix8(uint8, uint) pure returns (int8) {}

contract C {
    int a = 1.1 suffix256;
    uint8 b = 1024 suffix8;
}
// ----
