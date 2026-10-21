contract C {
    function g() external returns (uint) {}

    function f() public {
        try this.g() returns (uint a) {} // No shadowing (due to order)
        catch Error(string memory b) {} // No shadowing (due to order)
        catch Panic(uint c) {} // No shadowing (due to order)
        catch (bytes memory d) {} // No shadowing (due to order)

        uint256 a;
        uint256 b;
        uint256 c;
        uint256 d;
    }

    function h() public {
        uint256 a;
        uint256 b;
        uint256 c;
        uint256 d;

        try this.g() returns (uint a) {} // Shadows a
        catch Error(string memory b) {} // Shadows b
        catch Panic(uint c) {} // Shadows c
        catch (bytes memory d) {} // Shadows d
    }

    function i() public {
        try this.g() returns (uint e) { uint e; } // Shadows the try parameter
        catch (bytes memory p) { uint p; } // Shadows the catch parameter
    }
}
// ----
// Warning 2519: (571-577): This declaration shadows an existing declaration.
// Warning 2519: (615-630): This declaration shadows an existing declaration.
// Warning 2519: (668-674): This declaration shadows an existing declaration.
// Warning 2519: (707-721): This declaration shadows an existing declaration.
// Warning 2519: (812-818): This declaration shadows an existing declaration.
// Warning 2519: (884-890): This declaration shadows an existing declaration.
// Warning 5667: (114-120): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (176-191): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (247-253): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (304-318): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 2072: (363-372): Unused local variable.
// Warning 2072: (382-391): Unused local variable.
// Warning 2072: (401-410): Unused local variable.
// Warning 2072: (420-429): Unused local variable.
// Warning 2072: (472-481): Unused local variable.
// Warning 2072: (491-500): Unused local variable.
// Warning 2072: (510-519): Unused local variable.
// Warning 2072: (529-538): Unused local variable.
// Warning 5667: (571-577): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (615-630): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (668-674): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (707-721): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (802-808): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 2072: (812-818): Unused local variable.
// Warning 5667: (866-880): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 2072: (884-890): Unused local variable.
