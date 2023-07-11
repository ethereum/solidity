contract C {
  function g(bool b) public {}
  function f(bool b) public returns (bytes memory txt) {
    if (0==1)
      try this.g(b) {}
      catch (bytes memory s) {
        txt = s;
      }
  }
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (109-113): BMC: Condition is always false.
