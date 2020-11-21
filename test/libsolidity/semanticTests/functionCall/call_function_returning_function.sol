contract test {
    function f0() public returns (uint) {
        return 2;
    }

    function f1() internal returns (function() internal returns (uint)) {
        return f0;
    }

    function f2() internal returns (function() internal returns (function () internal returns (uint))) {
        return f1;
    }

    function f3() internal returns (function() internal returns (function () internal returns (function () internal returns (uint)))) {
        return f2;
    }

    function f() public returns (uint) {
        function() internal returns(function() internal returns(function() internal returns(function() internal returns(uint)))) x;
        x = f3;
        return x()()()();
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 2
