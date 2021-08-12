contract C {
    function f() public returns (uint x) {
        assembly {
            function g() -> f { f := 2 }
            x := g()
        }
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 2
