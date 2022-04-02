contract test {
        function fun(uint256 a) public {
            uint256 i =0;
            for (;;) {
                uint256 x = i; break; continue;
            }
        }
    }
// ----
// Warning 5740: (144-152='continue'): Unreachable code.
// Warning 5667: (37-46='uint256 a'): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 2072: (122-131='uint256 x'): Unused local variable.
// Warning 2018: (24-177): Function state mutability can be restricted to pure
