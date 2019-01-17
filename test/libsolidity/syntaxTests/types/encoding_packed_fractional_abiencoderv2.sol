pragma experimental ABIEncoderV2;
contract C {
   function f1() public pure returns (bytes memory) {
       return abi.encodePacked(0.1, 1);
   }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (132-135): Fractional numbers cannot yet be encoded.
// TypeError: (137-138): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.
