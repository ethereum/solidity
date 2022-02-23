contract C {
    event e(uint u, string s, bool b);
    event e(uint u, uint s, uint b);

    function call() public {
        emit e({s: "abc", u: 1,     b: true});
        emit e({s: "abc", b: true,  u: 1});
        emit e({u: 1,     s: "abc", b: true});
        emit e({b: true,  s: "abc", u: 1});
        emit e({u: 1,     b: true,  s: "abc"});
        emit e({b: true,  u: 1,     s: "abc"});
    }
}
// ----
