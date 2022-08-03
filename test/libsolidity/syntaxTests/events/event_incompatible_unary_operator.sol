contract C {
    event MyCustomEvent(uint);
    function f() pure public {
        MyCustomEvent++;
    }
}

// ----
// TypeError 4247: (83-96): Expression has to be an lvalue.
// TypeError 9767: (83-98): Unary operator ++ cannot be applied to type event MyCustomEvent(uint256).
