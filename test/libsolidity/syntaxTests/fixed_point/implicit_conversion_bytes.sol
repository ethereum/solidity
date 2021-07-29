bytes32 constant x = ufixed256x5(0);
bytes16 constant y = fixed(0);
// ----
// TypeError 7407: (21-35): Type ufixed256x5 is not implicitly convertible to expected type bytes32.
// TypeError 7407: (58-66): Type fixed128x18 is not implicitly convertible to expected type bytes16.
