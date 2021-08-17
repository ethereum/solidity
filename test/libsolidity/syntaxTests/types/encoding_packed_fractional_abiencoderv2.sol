pragma abicoder               v2;
contract C {
   function f1() public pure returns (bytes memory) {
       return abi.encodePacked(0.1, 1);
   }
}
// ----
// TypeError 7279: (132-135): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.
// TypeError 7279: (137-138): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.
