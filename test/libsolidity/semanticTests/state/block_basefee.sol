contract C {
    function f() public view returns (uint) {
        return block.basefee;
    }
    function g() public view returns (uint ret) {
        assembly {
            ret := basefee()
        }
    }
}
// ====
// EVMVersion: >=london
// ----
// f() -> 7
// g() -> 7
// f() -> 7
// g() -> 7
