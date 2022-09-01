contract Base {
    function f() internal returns (uint256 i) {
        function() internal returns (uint256) ptr = g;
        return ptr();
    }

    function g() internal virtual returns (uint256 i) {
        return 1;
    }
}


contract Derived is Base {
    function g() internal override returns (uint256 i) {
        return 2;
    }

    function h() public returns (uint256 i) {
        return f();
    }
}

// ====
// compileToEwasm: also
// ----
// h() -> 2
