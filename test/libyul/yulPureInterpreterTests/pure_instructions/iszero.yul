{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(iszero(0x0), 0x1)
    check(iszero(0x1), 0x0)
    check(iszero(0x2), 0x0)
    check(iszero(0x3), 0x0)
    check(iszero(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(iszero(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(iszero(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd), 0x0)
    check(iszero(0xdc91451b4d11884662ecce3baf532f24813b65c6e9ff858cb3269bf73ce701a9), 0x0)
    check(iszero(0x5a1caa33a14b8f9c03f754609a5eef3932e3c82f2632fb74bb1dc3f5ea044701), 0x0)
    check(iszero(0xfe8be50fa47cacbcb36d160b4116bdaef12e1b99ce9ec6d4df90d398aecb34be), 0x0)
    check(iszero(0x77652457195e8499212c1a3b87ff6d112cfb377bf4b33eec9f9d7ea95c763b6d), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
