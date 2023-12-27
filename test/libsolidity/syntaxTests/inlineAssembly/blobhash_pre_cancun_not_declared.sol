contract C {
    function f() pure external returns (bytes32 ret) {
        assembly {
            ret := blobhash()
        }
    }
}
// ====
// EVMVersion: <=shanghai
// ----
// DeclarationError 4619: (106-114): Function "blobhash" not found.
// DeclarationError 8678: (99-116): Variable count for assignment to "ret" does not match number of values (1 vs. 0)
