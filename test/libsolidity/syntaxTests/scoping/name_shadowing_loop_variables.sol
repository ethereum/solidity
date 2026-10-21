contract C {
    function f() public pure {
        for (uint i = 0; i < 10; ++i) {} // No shadowing (due to order)
        uint i;

        for (uint j = 0; j < 10; ++j) { uint k; } // No shadowing (due to order)
        uint k;

        while (true) { uint w; break; } // No shadowing (due to order)
        uint w;

        do { uint d; } while (false); // No shadowing (due to order)
        uint d;
    }

    function g() public pure {
        uint i;
        for (uint i = 0; i < 10; ++i) {} // Shadows i

        uint k;
        for (uint j = 0; j < 10; ++j) { uint k; } // Shadows k

        uint w;
        while (true) { uint w; break; } // Shadows w

        uint d;
        do { uint d; } while (false); // Shadows d

        for (uint n = 0; n < 10; ++n) { uint n; } // Shadows the loop variable
    }
}
// ----
// Warning 2519: (471-477): This declaration shadows an existing declaration.
// Warning 2519: (569-575): This declaration shadows an existing declaration.
// Warning 2519: (632-638): This declaration shadows an existing declaration.
// Warning 2519: (692-698): This declaration shadows an existing declaration.
// Warning 2519: (771-777): This declaration shadows an existing declaration.
// Warning 2072: (124-130): Unused local variable.
// Warning 2072: (173-179): Unused local variable.
// Warning 2072: (222-228): Unused local variable.
// Warning 2072: (254-260): Unused local variable.
// Warning 2072: (310-316): Unused local variable.
// Warning 2072: (332-338): Unused local variable.
// Warning 2072: (396-402): Unused local variable.
// Warning 2072: (450-456): Unused local variable.
// Warning 2072: (521-527): Unused local variable.
// Warning 2072: (569-575): Unused local variable.
// Warning 2072: (601-607): Unused local variable.
// Warning 2072: (632-638): Unused local variable.
// Warning 2072: (671-677): Unused local variable.
// Warning 2072: (692-698): Unused local variable.
// Warning 2072: (771-777): Unused local variable.
