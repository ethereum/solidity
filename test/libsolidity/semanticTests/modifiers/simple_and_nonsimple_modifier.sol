contract C {
    uint256 public x;
    modifier m1() {
        x = 1;
        _;
    }

    modifier m2(bool abort) {
        if (abort) return;
        _;
    }

    function f(bool abort) public m1 m2(abort) {
        x += 2;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// x() -> 0
// f(bool): false ->
// x() -> 3
// f(bool): true ->
// x() -> 1
