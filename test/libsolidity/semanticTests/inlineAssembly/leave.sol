contract C {
    function f() public pure returns (uint w) {
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
// compileViaYul: also
// ----
// f() -> 2
