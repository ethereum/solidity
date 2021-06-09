contract A {
  function f() public virtual returns (uint) { g(); }
  function g() internal virtual { revert(); }
}
contract B is A {
  function f() public virtual override returns (uint) { A.f(); }
  function g() internal virtual override { A.g(); }
}
contract C is B {
  function f() public virtual override returns (uint) { A.f(); }
  function g() internal virtual override { }
}
// ----
// Warning 6321: (52-56): Unnamed return variable can remain unassigned when the function is called when "C" is the most derived contract. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (181-185): Unnamed return variable can remain unassigned when the function is called when "C" is the most derived contract. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (318-322): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
