// This "v0.5.0" flag is here to avoid a double-warning that got introduced due to the var-keyword removal
pragma experimental "v0.5.0";

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
// TypeError: (280-293): Type tuple() is not implicitly convertible to expected type uint256.
// TypeError: (366-388): Not enough components (0) in value to assign all variables (1).
