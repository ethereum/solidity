contract C { constructor(uint) {} }
contract D is C(D.t = 2) {
  uint immutable t;
}
// ----
// TypeError 1581: (52-55): Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor.
