{
    function fake_mstore(pos, val) { }
    {
        let a := 0x20
        fake_mstore(a, 2)
    }
    let a
    fake_mstore(a, 3)
}
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   a = 0
//
// Call trace:
//   [CALL] fake_mstore(32, 2)
//   │ [RETURN]
//   [CALL] fake_mstore(0, 3)
//   │ [RETURN]
