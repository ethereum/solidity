contract C {
    function f() public view returns (uint ret) {
        assembly {
            let basefee := sload(0)
            ret := basefee
        }
    }
    function g() public pure returns (uint ret) {
        assembly {
            function basefee() -> r {
                r := 1000
            }
            ret := basefee()
        }
    }
}
// ====
// compileViaYul: also
// EVMVersion: <=berlin
// ----
// f() -> 0
// g() -> 1000
