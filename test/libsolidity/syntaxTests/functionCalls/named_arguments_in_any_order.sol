contract C {
    function f(uint u, string memory s, bool b) internal {}

    function call() public {
        f({s: "abc", u: 1,     b: true});
        f({s: "abc", b: true,  u: 1});
        f({u: 1,     s: "abc", b: true});
        f({b: true,  s: "abc", u: 1});
        f({u: 1,     b: true,  s: "abc"});
        f({b: true,  u: 1,     s: "abc"});
    }
}
// ----
