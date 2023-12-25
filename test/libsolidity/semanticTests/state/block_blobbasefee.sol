contract C {
    function f() public view returns (uint) {
        return block.blobbasefee;
    }
    function g() public view returns (uint ret) {
        assembly {
            ret := blobbasefee()
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 1
// g() -> 1
// f() -> 1
// g() -> 1
