contract test {
    bool public flag = false;

    function f0() public {
        flag = true;
    }

    function f() public returns (bool) {
        function() internal x = f0;
        x();
        return flag;
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> true
// flag() -> true
