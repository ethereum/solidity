// The IR of this contract used to throw
contract A {
  constructor() m1 { }
  modifier m1 { _; }
}
contract B is A {
  modifier m2 { _; }
  constructor() A() m1 m2 {  }
}
// ====
// compileToEwasm: also
// ----
// constructor() ->
