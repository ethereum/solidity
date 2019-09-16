contract A { modifier mod(uint a) { _; } }
contract B is A { modifier mod(uint a) override { _; } }
