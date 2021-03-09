{
    function f() -> x { pop(address()) let y := callvalue() }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xD
// JUMP
// JUMPDEST
// ADDRESS
// POP
// PUSH1 0x0
// CALLVALUE
// POP
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
