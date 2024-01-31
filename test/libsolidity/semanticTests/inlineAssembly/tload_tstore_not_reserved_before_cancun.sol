contract C {
    function f() public view returns (uint ret) {
        assembly {
            let tload := sload(0)
            let tstore := add(tload, 1)
            ret := tstore
        }
    }
    function g() public view returns (uint ret) {
        assembly {
            function tstore() -> a {
                a := 2
            }
            function tload() -> b {
                b := 3
            }
            ret := add(tstore(), tload())
        }
    }
}
// ====
// EVMVersion: <cancun
// ----
// f() -> 1
// g() -> 5
