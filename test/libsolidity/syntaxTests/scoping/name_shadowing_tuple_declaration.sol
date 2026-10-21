contract C {
    function f() public pure {
        { (uint a, uint b) = (1, 2); } // No shadowing (due to order)
        (uint a, uint b) = (3, 4);

        { uint c; } // No shadowing (due to order)
        (uint c, , uint d) = (5, 6, 7);

        { (, uint e) = (8, 9); } // No shadowing (due to order)
        uint e;
    }

    function g() public pure {
        (uint a, uint b) = (1, 2);
        { (uint a, uint b) = (3, 4); } // Shadows a and b

        uint c;
        { (uint c, , uint d) = (5, 6, 7); } // Shadows c

        (, uint e) = (8, 9);
        { uint e; } // Shadows e
    }
}
// ----
// Warning 2519: (406-412): This declaration shadows an existing declaration.
// Warning 2519: (414-420): This declaration shadows an existing declaration.
// Warning 2519: (481-487): This declaration shadows an existing declaration.
// Warning 2519: (567-573): This declaration shadows an existing declaration.
// Warning 2072: (55-61): Unused local variable.
// Warning 2072: (63-69): Unused local variable.
// Warning 2072: (123-129): Unused local variable.
// Warning 2072: (131-137): Unused local variable.
// Warning 2072: (160-166): Unused local variable.
// Warning 2072: (210-216): Unused local variable.
// Warning 2072: (220-226): Unused local variable.
// Warning 2072: (255-261): Unused local variable.
// Warning 2072: (314-320): Unused local variable.
// Warning 2072: (369-375): Unused local variable.
// Warning 2072: (377-383): Unused local variable.
// Warning 2072: (406-412): Unused local variable.
// Warning 2072: (414-420): Unused local variable.
// Warning 2072: (462-468): Unused local variable.
// Warning 2072: (481-487): Unused local variable.
// Warning 2072: (491-497): Unused local variable.
// Warning 2072: (539-545): Unused local variable.
// Warning 2072: (567-573): Unused local variable.
