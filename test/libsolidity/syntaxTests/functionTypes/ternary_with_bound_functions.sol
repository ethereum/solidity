library L {
    function f(uint a) internal pure {}
    function g(uint a) internal pure {}
}
contract C {
    using L for *;
    function f(bool x) pure public {
        uint t = 8;
        (x ? t.f : t.g)();
    }
}
// ----
// TypeError 9717: (196-199): Invalid mobile type in true expression.
// TypeError 3703: (202-205): Invalid mobile type in false expression.
