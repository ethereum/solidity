contract A {
  function f() public virtual returns (uint) { g(); }
  function g() internal virtual { revert(); }
}
contract B is A {
  function f() public override returns (uint) { A.f(); }
  function g() internal override {}
}
// ----
// Warning 6321: (52-56): Unnamed return variable can remain unassigned when the function is called when "B" is the most derived contract. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (173-177): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
