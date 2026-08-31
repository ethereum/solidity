contract C {
    struct S {
        function() external f;
    }

    function test() public {
        S memory s;
        delete s.f.selector;
        delete s.f.address;
    }
}
// ----
// TypeError 4247: (130-142): Expression has to be an lvalue.
// TypeError 4247: (159-170): Expression has to be an lvalue.
