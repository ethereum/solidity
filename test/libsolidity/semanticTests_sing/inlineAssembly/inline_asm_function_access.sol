contract C {
    function f() public pure {}
    constructor() public {
        assembly {
            let x := f
        }
    }
}
// ====
// compileViaYul: also
// ----
// constructor() ->
