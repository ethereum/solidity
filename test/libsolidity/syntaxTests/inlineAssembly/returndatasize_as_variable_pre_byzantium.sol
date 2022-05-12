contract C { function f() public pure { uint returndatasize; returndatasize; assembly { pop(returndatasize()) }}}
// ====
// EVMVersion: =homestead
// ----
// TypeError 4778: (92-106): The "returndatasize" instruction is only available for Byzantium-compatible VMs (you are currently compiling for "homestead").
// TypeError 3950: (92-108): Expected expression to evaluate to one value, but got 0 values instead.
