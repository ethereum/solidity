contract C {
    struct S { uint a; uint b; }

    function f() public {
        S memory s = S({a: 1});
    }
}
// ----
// TypeError: (94-103): Wrong argument count for struct constructor: 1 arguments given but expected 2.
