{
    function fake_mstore(pos, val) {}

    for { let x := 2 } lt(x, 10) { x := add(x, 1) } {
        fake_mstore(mul(x, 5), mul(x, 0x1000))
    }
}
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
//   [CALL] fake_mstore(10, 8192)
//   │ [RETURN]
//   [CALL] fake_mstore(15, 12288)
//   │ [RETURN]
//   [CALL] fake_mstore(20, 16384)
//   │ [RETURN]
//   [CALL] fake_mstore(25, 20480)
//   │ [RETURN]
//   [CALL] fake_mstore(30, 24576)
//   │ [RETURN]
//   [CALL] fake_mstore(35, 28672)
//   │ [RETURN]
//   [CALL] fake_mstore(40, 32768)
//   │ [RETURN]
//   [CALL] fake_mstore(45, 36864)
//   │ [RETURN]
