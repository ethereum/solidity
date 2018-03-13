library L {
    function f(uint) pure external {}
}

contract C {
    using L for *;

    function f() public pure {
        L.f(2);
        uint x;
        x.f();
    }
}
// ----
