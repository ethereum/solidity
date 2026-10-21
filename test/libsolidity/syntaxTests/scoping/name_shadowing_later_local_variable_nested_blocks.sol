contract C {
    function f() public pure {
        {
            { uint x; } // No shadowing (due to order)
            uint x; // No shadowing (due to order)
        }
        uint x;
    }

    function g() public pure {
        uint y;
        {
            { uint y; } // Shadows y from the function body, despite the later y in the enclosing block
            uint y; // Shadows y from the function body
        }
    }

    function h() public pure {
        {
            uint z;
            { uint z; } // Shadows z from the enclosing block
        }
        uint z;
    }
}
// ----
// Warning 2519: (264-270): This declaration shadows an existing declaration.
// Warning 2519: (366-372): This declaration shadows an existing declaration.
// Warning 2519: (502-508): This declaration shadows an existing declaration.
// Warning 2072: (68-74): Unused local variable.
// Warning 2072: (121-127): Unused local variable.
// Warning 2072: (178-184): Unused local variable.
// Warning 2072: (232-238): Unused local variable.
// Warning 2072: (264-270): Unused local variable.
// Warning 2072: (366-372): Unused local variable.
// Warning 2072: (480-486): Unused local variable.
// Warning 2072: (502-508): Unused local variable.
// Warning 2072: (568-574): Unused local variable.
