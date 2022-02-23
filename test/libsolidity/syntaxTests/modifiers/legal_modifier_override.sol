contract A { modifier mod(uint a) virtual { _; } }
contract B is A { modifier mod(uint a) override { _; } }
// ----
