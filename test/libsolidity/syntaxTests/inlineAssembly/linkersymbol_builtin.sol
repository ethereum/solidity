contract C {
  function f() public pure {
    assembly {
      pop(linkersymbol("contract/library.sol:L"))
    }
  }
}
// ----
// DeclarationError 4619: (67-79): Function "linkersymbol" not found.
// TypeError 3950: (67-105): Expected expression to evaluate to one value, but got 0 values instead.
