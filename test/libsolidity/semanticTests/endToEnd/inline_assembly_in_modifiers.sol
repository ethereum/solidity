contract C {
    modifier m {
        uint a = 1;
        assembly {
            a: = 2
        }
        if (a != 2)
            revert();
        _;
    }

    function f() m public returns(bool) {
        return true;
    }
}

// ====
// compileViaYul: also
// ----
// f() -> true
// f():"" -> "1"
