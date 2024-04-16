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
