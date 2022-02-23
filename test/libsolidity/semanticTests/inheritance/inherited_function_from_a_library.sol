library A {
    function f() internal returns (uint256) {
        return 1;
    }
}


contract B {
    function f() internal returns (uint256) {
        return 2;
    }

    function g() public returns (uint256) {
        return A.f();
    }
}

// ====
// compileViaYul: also
// ----
// g() -> 1
