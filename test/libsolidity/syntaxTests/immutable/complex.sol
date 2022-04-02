contract A {
  int immutable a;
  constructor() { a = 5; }
  function f() public { a += 7; }
}

// ----
// TypeError 1581: (83-84='a'): Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor.
