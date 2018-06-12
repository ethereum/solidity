contract C {
    function returnsVoid() pure public { }

    function testVoidAssignmentStmt() pure public {
        uint i = 42;
        i = returnsVoid();
    }

    function testVoidExprInitialization() pure public {
        uint i = returnsVoid();
    }
}
// ----
// TypeError: (142-155): Type tuple() is not implicitly convertible to expected type uint256.
// Warning: (228-250): Different number of components on the left hand side (1) than on the right hand side (0).
// TypeError: (228-250): Not enough components (0) in value to assign all variables (1).
