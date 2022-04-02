contract A { modifier mod(uint a) { _; } }
contract B is A { modifier mod(uint a) override { _; } }
// ----
// TypeError 4334: (13-40='modifier mod(uint a) { _; }'): Trying to override non-virtual modifier. Did you forget to add "virtual"?
