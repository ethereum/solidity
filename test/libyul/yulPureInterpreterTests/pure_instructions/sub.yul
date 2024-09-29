{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(sub(0x0, 0x0), 0x0)
    check(sub(0x0, 0x1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sub(0x0, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(sub(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(sub(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x2)
    check(sub(0x0, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e84)
    check(sub(0x0, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd6)
    check(sub(0x0, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9de)
    check(sub(0x1, 0x0), 0x1)
    check(sub(0x1, 0x1), 0x0)
    check(sub(0x1, 0x2), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sub(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x2)
    check(sub(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x3)
    check(sub(0x1, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e85)
    check(sub(0x1, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd7)
    check(sub(0x1, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9df)
    check(sub(0x2, 0x0), 0x2)
    check(sub(0x2, 0x1), 0x1)
    check(sub(0x2, 0x2), 0x0)
    check(sub(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x3)
    check(sub(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x4)
    check(sub(0x2, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e86)
    check(sub(0x2, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd8)
    check(sub(0x2, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9e0)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e83)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd5)
    check(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9dd)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e82)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd4)
    check(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9dc)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x0), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x1), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17b)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x2), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17a)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17d)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17e)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0x0)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0x228806570f98613dae80c1017cf27cdee2891468b68f92e6e004d07cb67cef52)
    check(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x9566d2426a939b57cfabe27acfe34dc5e5537cf2e39944bdb0597528c809db5a)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x0), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x1), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd40229)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x2), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd40228)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022b)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022c)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xdd77f9a8f0679ec2517f3efe830d83211d76eb9749706d191ffb2f83498310ae)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0x0)
    check(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x72decbeb5afb3a1a212b217952f0d0e702ca688a2d09b1d6d054a4ac118cec08)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x0), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x1), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471621)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x2), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471620)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471623)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471624)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0x6a992dbd956c64a830541d85301cb23a1aac830d1c66bb424fa68ad737f624a6)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0x8d213414a504c5e5ded4de86ad0f2f18fd359775d2f64e292fab5b53ee7313f8)
    check(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outter most variable values:
//
// Call trace:
