contract C {
    function f() public view returns (uint ret) {
        assembly {
            let prevrandao := sload(0)
            ret := prevrandao
        }
    }
    function g() public pure returns (uint ret) {
        assembly {
            function prevrandao() -> r {
                r := 1000
            }
            ret := prevrandao()
        }
    }
}
// ====
// EVMVersion: <paris
// ----
// f() -> 0
// g() -> 1000
