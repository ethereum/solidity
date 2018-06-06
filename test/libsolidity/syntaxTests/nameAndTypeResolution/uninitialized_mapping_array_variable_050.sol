pragma experimental "v0.5.0";
contract C {
    function f() pure public {
        mapping(uint => uint)[] storage x;
        x;
    }
}
// ----
// DeclarationError: (82-115): Uninitialized storage pointer.
