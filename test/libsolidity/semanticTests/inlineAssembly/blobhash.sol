contract C {
    function f() public view returns (bytes32 ret) {
        assembly {
            ret := blobhash(0)
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 0x0100000000000000000000000000000000000000000000000000000000000001
