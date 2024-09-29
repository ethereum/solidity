{
    function f(a, b) -> x, y {
        x := add(a, b)
        y := mul(a, b)
    }
    let r, t := f(6, 7)
}
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   r = 13
//   t = 42
//
// Call trace:
//   [CALL] f(6, 7)
//   │ [RETURN] 13, 42
