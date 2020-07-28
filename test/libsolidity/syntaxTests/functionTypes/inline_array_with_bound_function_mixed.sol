library L {
    function f(uint a) internal pure {}
}
contract C {
    using L for *;
    function f() pure public {
        uint t;
        function() pure x;
        [t.f, x][0]({a: 8});
    }
}
// ----
// TypeError 9563: (169-172): Invalid mobile type.
