interface I {
    event E();
}

library L {
    function f() internal {
        emit I.E();
    }
}

contract C {
    function g() public {
        L.f();
    }
}

// ====
// targetContract: C
// ----
// g() ->
// ~ emit E()
