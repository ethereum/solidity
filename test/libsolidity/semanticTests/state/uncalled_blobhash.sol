contract C {
    function f() public returns (bytes32) {
        // NOTE: The `tx_context.blob_hashes` is injected into EVMHost with the following hashes, indexed accordingly:
        // 0 -> 0x0100000000000000000000000000000000000000000000000000000000000001
        // 1 -> 0x0100000000000000000000000000000000000000000000000000000000000002
        return (blobhash)(0);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 0x0100000000000000000000000000000000000000000000000000000000000001
