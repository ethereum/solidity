uint constant BASE = uint(0x42);

contract A layout at 7 + uint(0xff) {}
contract B layout at uint(0x10) * 4 {}
contract C layout at uint(BASE) + uint(0x100) {}
contract D layout at (uint(0x1) << 8) + uint(0x42) {}
// ----
