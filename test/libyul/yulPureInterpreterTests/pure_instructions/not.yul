{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(not(0x0), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(not(0x1), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(not(0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd)
    check(not(0x3), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    check(not(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(not(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(not(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd), 0x2)
    check(not(0xb9caa39ea86683d4e096bd6edd32ae3ead3c1b3a5c0c9ff6aaf9f46e9859aa7e), 0x46355c6157997c2b1f69429122cd51c152c3e4c5a3f3600955060b9167a65581)
    check(not(0xe4fe151407cfb034d8ec5ae737b2fb264e0b4084b976fa07648d31ec1b1bedcb), 0x1b01eaebf8304fcb2713a518c84d04d9b1f4bf7b468905f89b72ce13e4e41234)
    check(not(0xd6bfc8a63b534b270ddfd63ce081db9109bc4355465b6a40b200b01fac47eb4e), 0x29403759c4acb4d8f22029c31f7e246ef643bcaab9a495bf4dff4fe053b814b1)
    check(not(0xe40000700672cffea275f0f1a48f6096ffb97924fa63024f6d5411b5a529a84d), 0x1bffff8ff98d30015d8a0f0e5b709f69004686db059cfdb092abee4a5ad657b2)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outter most variable values:
//
// Call trace:
