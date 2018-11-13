    contract C {
        function f() public {
            // Invalid number
            [1, 78901234567890123456789012345678901234567890123456789345678901234567890012345678012345678901234567];
        }
    }
// ----
// TypeError: (93-191): Invalid rational number.
