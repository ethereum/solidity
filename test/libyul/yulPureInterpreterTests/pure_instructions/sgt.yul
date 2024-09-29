{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(sgt(0x0, 0x0), 0x0)
    check(sgt(0x0, 0x1), 0x0)
    check(sgt(0x0, 0x2), 0x0)
    check(sgt(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(sgt(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(sgt(0x0, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x1)
    check(sgt(0x0, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x0)
    check(sgt(0x0, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
    check(sgt(0x1, 0x0), 0x1)
    check(sgt(0x1, 0x1), 0x0)
    check(sgt(0x1, 0x2), 0x0)
    check(sgt(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(sgt(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(sgt(0x1, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x1)
    check(sgt(0x1, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x0)
    check(sgt(0x1, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
    check(sgt(0x2, 0x0), 0x1)
    check(sgt(0x2, 0x1), 0x1)
    check(sgt(0x2, 0x2), 0x0)
    check(sgt(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(sgt(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(sgt(0x2, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x1)
    check(sgt(0x2, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x0)
    check(sgt(0x2, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0x0)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0x0)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0x0)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x1)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x0)
    check(sgt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0x0)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0x0)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0x0)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x1)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x0)
    check(sgt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0x0), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0x1), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0x2), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x0)
    check(sgt(0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0x0), 0x1)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0x1), 0x1)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0x2), 0x1)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x1)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x0)
    check(sgt(0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0x0), 0x1)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0x1), 0x1)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0x2), 0x1)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0xc1a8f6ade2306920f87bbaa51747608772d5c66c1d9944c8326ae9262279fcb4), 0x1)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0x79a6410ddb1ec06466e380d4d2e2ae4a66c4c6ba06c270868274d037ac6cf53), 0x1)
    check(sgt(0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445, 0x7c4c8a3d9eb0f7dc6a89d59dfd710f4ef6fd7a9eea21020eebd16cb1baa50445), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
