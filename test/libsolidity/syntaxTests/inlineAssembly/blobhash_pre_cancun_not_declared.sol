contract C {
    function f() pure external returns (bytes32 ret) {
        assembly {
            ret := blobhash()
        }
    }
}
// ====
// EVMVersion: =shanghai
// ----
// TypeError 8314: (106-114): The "blobhash" instruction is only available for Cancun-compatible VMs (you are currently compiling for "shanghai").
// DeclarationError 8678: (99-116): Variable count for assignment to "ret" does not match number of values (1 vs. 0)
