contract C {
    function f() public returns (uint) {
        return block.basefee;
    }
    function g() public returns (uint ret) {
        assembly {
            ret := basefee()
        }
    }
}
// ====
// EVMVersion: >=london
// compileViaYul: also
// ----
// f() -> FAILURE
// f() -> FAILURE
// f() -> FAILURE
