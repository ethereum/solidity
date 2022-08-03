contract C {
    event MyCustomEvent(uint);
    function f() pure public {
        uint a;
        MyCustomEvent += 1;
        MyCustomEvent -= 1;
        a += MyCustomEvent;
        a -= MyCustomEvent;
    }
}

// ----
// TypeError 4247: (99-112): Expression has to be an lvalue.
// TypeError 7366: (99-117): Operator += not compatible with types event MyCustomEvent(uint256) and int_const 1.
// TypeError 4247: (127-140): Expression has to be an lvalue.
// TypeError 7366: (127-145): Operator -= not compatible with types event MyCustomEvent(uint256) and int_const 1.
// TypeError 7366: (155-173): Operator += not compatible with types uint256 and event MyCustomEvent(uint256).
// TypeError 7366: (183-201): Operator -= not compatible with types uint256 and event MyCustomEvent(uint256).
