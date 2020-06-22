contract C {
   function f1() public pure returns (bytes memory) {
       return abi.encodePacked(0.1, 1);
   }
}
// ----
// TypeError 6090: (98-101): Fractional numbers cannot yet be encoded.
// TypeError 7279: (103-104): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.
