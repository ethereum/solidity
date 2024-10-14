{
    function f() -> x { x := 0x1337 }
    pop(byte(0, shr(0x8, f())))
}
// ====
// EVMVersion: >=constantinople
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
//   [CALL] f()
//   │ [RETURN] 4919
