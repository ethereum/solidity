contract C {
    function g() public pure returns (uint w) {
        assembly {
            function f() -> t {
                t := 2
                leave
                t := 9
            }
            w := f()
        }
    }
}
// ====
// compileToEwasm: also
// ----
// g() -> 2
