contract C {
    uint public a = 42;

    function f() public {
        a = 1;
        revert();
    }

    function g() public {
        a = 1;
        assembly {
            revert(0, 0)
        }
    }
}

// ----
// f() -> 
// f():"" -> ""
// a() -> 42
// a():"" -> "42"
// g() -> 
// g():"" -> ""
// a() -> 42
// a():"" -> "42"
