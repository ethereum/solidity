library L {
    function f(uint a) internal pure {}
    function g(uint a) internal pure {}
}
contract C {
    using L for *;
    function f() pure public {
        uint t = 8;
        [t.f, t.g][0]();
    }
}
// ----
// TypeError 9563: (186-189): Invalid mobile type.
