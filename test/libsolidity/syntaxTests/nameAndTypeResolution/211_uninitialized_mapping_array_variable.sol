contract C {
    function f() pure public {
        mapping(uint => uint)[] storage x;
        x;
    }
}
// ----
// DeclarationError: (52-85): Uninitialized storage pointer.
