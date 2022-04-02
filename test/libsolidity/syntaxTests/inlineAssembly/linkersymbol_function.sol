contract C {
  function f() public pure {
    assembly {
      function linkersymbol(a) {}

      linkersymbol("contract/library.sol:L")
    }
  }
}
// ----
// DeclarationError 5017: (63-90='function linkersymbol(a) {}'): The identifier "linkersymbol" is reserved and can not be used.
