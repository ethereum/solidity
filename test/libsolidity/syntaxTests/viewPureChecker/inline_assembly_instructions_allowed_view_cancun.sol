contract C {
    function f() public view {
        assembly {
            pop(blobbasefee())
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
