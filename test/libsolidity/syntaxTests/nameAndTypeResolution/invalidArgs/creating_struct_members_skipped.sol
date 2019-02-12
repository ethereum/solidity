contract C {
    struct S { uint a; uint b; mapping(uint=>uint) c; }

    function f() public {
        S memory s = S({a: 1});
    }
}
// ----
// TypeError: (117-126): Wrong argument count for struct constructor: 1 arguments given but expected 2. Members that have to be skipped in memory: c
