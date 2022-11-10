error MyCustomError(uint, bool);

contract C {
    function f() pure public {
        MyCustomError++;
    }
}

// ----
// TypeError 4247: (86-99): Expression has to be an lvalue.
// TypeError 9767: (86-101): Built-in unary operator ++ cannot be applied to type error MyCustomError(uint256,bool).
