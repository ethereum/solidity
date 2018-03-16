contract A { modifier mod(uint a) { _; } }
contract B is A { modifier mod(uint8 a) { _; } }
// ----
// TypeError: Override changes modifier signature.
