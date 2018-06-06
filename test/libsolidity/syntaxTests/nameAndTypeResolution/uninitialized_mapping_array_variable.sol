contract C {
    function f() pure public {
        mapping(uint => uint)[] storage x;
        x;
    }
}
// ----
// Warning: (52-85): Uninitialized storage pointer.
