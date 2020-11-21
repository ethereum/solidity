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
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 2
