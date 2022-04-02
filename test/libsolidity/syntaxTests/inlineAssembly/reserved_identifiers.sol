contract C {
  function f() public pure {
    assembly {
      let linkersymbol := 1
      let datacopy := 1
      let swap16 := 1
    }
  }
}
// ----
// DeclarationError 5017: (67-79='linkersymbol'): The identifier "linkersymbol" is reserved and can not be used.
// DeclarationError 5017: (95-103='datacopy'): The identifier "datacopy" is reserved and can not be used.
// DeclarationError 5017: (119-125='swap16'): The identifier "swap16" is reserved and can not be used.
