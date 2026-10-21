uint constant fileLevelConstant = 1;

contract C {
    uint stateVariableBefore;

    modifier m() {
        { uint modifierVariable; } // No shadowing (due to order)
        uint modifierVariable;
        _;
    }

    function f(uint parameter) public pure m returns (uint returnParameter) {
        { uint stateVariableBefore; } // Shadows the state variable, not the later local
        uint stateVariableBefore;

        { uint stateVariableAfter; } // Shadows the state variable, not the later local
        uint stateVariableAfter;

        { uint fileLevelConstant; } // Shadows the file-level constant, not the later local
        uint fileLevelConstant;

        { uint msg; } // Shadows the builtin, not the later local
        uint msg;

        { uint parameter; } // Shadows the parameter
        { uint returnParameter; } // Shadows the return parameter
    }

    uint stateVariableAfter;
}
// ----
// Warning 2519: (304-328): This declaration shadows an existing declaration.
// Warning 2519: (428-451): This declaration shadows an existing declaration.
// Warning 2519: (550-572): This declaration shadows an existing declaration.
// Warning 2319: (675-683): This declaration shadows a builtin symbol.
// Warning 2519: (760-774): This declaration shadows an existing declaration.
// Warning 2519: (813-833): This declaration shadows an existing declaration.
// Warning 2519: (391-415): This declaration shadows an existing declaration.
// Warning 2519: (514-537): This declaration shadows an existing declaration.
// Warning 2519: (640-662): This declaration shadows an existing declaration.
// Warning 2319: (739-747): This declaration shadows a builtin symbol.
// Warning 5667: (231-245): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (270-290): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 2072: (304-328): Unused local variable.
// Warning 2072: (391-415): Unused local variable.
// Warning 2072: (428-451): Unused local variable.
// Warning 2072: (514-537): Unused local variable.
// Warning 2072: (550-572): Unused local variable.
// Warning 2072: (640-662): Unused local variable.
// Warning 2072: (675-683): Unused local variable.
// Warning 2072: (739-747): Unused local variable.
// Warning 2072: (760-774): Unused local variable.
// Warning 2072: (813-833): Unused local variable.
