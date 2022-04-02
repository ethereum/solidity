contract A { modifier mod(uint a) { _; } }
contract B is A { modifier mod(uint8 a) { _; } }
// ----
// TypeError 1078: (61-89='modifier mod(uint8 a) { _; }'): Override changes modifier signature.
// TypeError 9456: (61-89='modifier mod(uint8 a) { _; }'): Overriding modifier is missing "override" specifier.
// TypeError 4334: (13-40='modifier mod(uint a) { _; }'): Trying to override non-virtual modifier. Did you forget to add "virtual"?
