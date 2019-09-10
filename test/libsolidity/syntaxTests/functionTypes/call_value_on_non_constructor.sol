contract C {
  // Tests that we don't get a wrong error about constructors
  function f() public view returns (C) { return this; }
  function g() public { this.f.value(); }
}
// ----
// TypeError: (155-167): Member "value" is only available for payable functions.
