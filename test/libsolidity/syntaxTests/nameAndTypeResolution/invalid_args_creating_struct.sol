contract C {
    struct S { uint a; uint b; }

    function f() public {
        var s = S({a: 1});
    }
}
// ----
// Warning: (81-86): Use of the "var" keyword is deprecated.
// TypeError: (89-98): Wrong argument count for struct constructor: 1 arguments given but expected 2.
