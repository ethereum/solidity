// This generated an invalid warning on m1 in some compiler versions.
contract A {
  constructor() m1 public { }
  modifier m1 { _; }
}
contract B is A {
  modifier m2 { _; }
  constructor() A() m1 m2 public {  }
}
