{
    function f() -> x
    {
        for { leave x := 2 } eq(x, 0) { }
        { }
    }
    let a := f()
}
// ====
// EVMVersion: >=constantinople
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   a = 0
//
// Call trace:
//   [CALL] f()
//   │ [RETURN] 0
