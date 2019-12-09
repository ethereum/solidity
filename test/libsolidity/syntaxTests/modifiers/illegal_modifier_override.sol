contract A { modifier mod(uint a) { _; } }
contract B is A { modifier mod(uint8 a) { _; } }
// ----
// TypeError: (61-89): Override changes modifier signature.
// TypeError: (61-89): Overriding modifier is missing "override" specifier.
// TypeError: (13-40): Trying to override non-virtual modifier. Did you forget to add "virtual"?
