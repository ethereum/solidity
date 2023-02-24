function suffix256(uint) pure suffix returns (uint) {}
function suffix256(uint, uint) pure suffix returns (int) {}

function suffix8(uint) pure suffix returns (uint8) {}
function suffix8(uint8, uint) pure suffix returns (int8) {}

contract C {
    int a = 1.1 suffix256;
    uint8 b = 1024 suffix8;
}
