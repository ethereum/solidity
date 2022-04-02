contract C {
   function f1() public pure returns (bytes memory) {
       return abi.encode(0.1, 1);
   }
}
// ----
// TypeError 6090: (92-95='0.1'): Fractional numbers cannot yet be encoded.
