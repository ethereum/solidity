contract C {
    function f() pure public {
        mapping(uint => uint)[] storage x;
        x;
    }
}
// ----
// TypeError: (95-96): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
