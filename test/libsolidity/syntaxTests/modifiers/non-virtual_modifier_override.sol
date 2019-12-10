contract A { modifier mod(uint a) { _; } }
contract B is A { modifier mod(uint a) override { _; } }
// ----
// TypeError: (13-40): Trying to override non-virtual modifier. Did you forget to add "virtual"?
