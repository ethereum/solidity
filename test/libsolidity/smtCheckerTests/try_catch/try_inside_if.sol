pragma experimental SMTChecker;
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

// ----
// Warning 6838: (141-145): BMC: Condition is always false.
