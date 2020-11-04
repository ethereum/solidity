{
    { if 42 {} }
    { if 42 { let x := 3 } }
    { function f() -> x {} if f() { pop(f()) } }
    { let x := 0 if eq(calldatasize(), 0) { x := 1 } mstore(0, x) }
}
// ====
// dialect: evm
