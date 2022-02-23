contract C {
  function f() public pure {
    assembly {
      let shl := 1
    }
    assembly {
      pop(shl(1, 2))
    }
  }
}
// ====
// EVMVersion: =byzantium
// ----
// DeclarationError 5017: (67-70): The identifier "shl" is reserved and can not be used.
// TypeError 6612: (107-110): The "shl" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// TypeError 3950: (107-116): Expected expression to evaluate to one value, but got 0 values instead.
