pragma experimental ABIEncoderV2;
contract C {
   function f1() public pure returns (bytes memory) {
       return abi.encode(0.1, 1);
   }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (126-129): Fractional numbers cannot yet be encoded.
