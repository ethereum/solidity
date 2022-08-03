error MyCustomError(uint, bool);

contract C {
    function f() pure public {
        uint a;
        MyCustomError += 1;
        MyCustomError -= 1;
        a += MyCustomError;
        a -= MyCustomError;
    }
}

// ----
// TypeError 4247: (102-115): Expression has to be an lvalue.
// TypeError 7366: (102-120): Operator += not compatible with types error MyCustomError(uint256,bool) and int_const 1.
// TypeError 4247: (130-143): Expression has to be an lvalue.
// TypeError 7366: (130-148): Operator -= not compatible with types error MyCustomError(uint256,bool) and int_const 1.
// TypeError 7366: (158-176): Operator += not compatible with types uint256 and error MyCustomError(uint256,bool).
// TypeError 7366: (186-204): Operator -= not compatible with types uint256 and error MyCustomError(uint256,bool).
