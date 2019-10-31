contract A { modifier mod(uint a) { _; } }
contract B is A { modifier mod(uint8 a) { _; } }
// ----
// TypeError: (61-89): Overriding modifier is missing 'override' specifier.
// TypeError: (61-89): Override changes modifier signature.
