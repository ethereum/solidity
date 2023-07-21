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

// ----
// g() ->
// ~ emit E()
