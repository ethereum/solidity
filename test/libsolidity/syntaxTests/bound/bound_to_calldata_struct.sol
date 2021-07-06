struct S {
    uint x;
}

library L {
    function f(S calldata) internal pure {}
}

contract C {
    using L for S;

    function run(S calldata _s) external pure {
        _s.f();
    }
}
