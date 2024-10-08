{
    function f() -> x, y
    {
        for { x := 0 } lt(x, 10) { x := add(x, 1) }
        {
            if eq(x, 5)
            {
                y := 1
                leave
            }
        }
        x := 9
    }
    let a, b := f()
}
// ====
// EVMVersion: >=constantinople
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   a = 5
//   b = 1
//
// Call trace:
//   [CALL] f()
//   │ [RETURN] 5, 1
