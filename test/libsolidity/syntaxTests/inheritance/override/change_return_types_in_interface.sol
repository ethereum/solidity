interface I {
  function f() external pure returns (uint);
}
contract B is I {
  // The compiler used to have a bug where changing
  // the return type was fine in this situation.
  function f() public pure returns (uint, uint) {}
}
// ----
// TypeError: (182-230): Overriding function return types differ.
