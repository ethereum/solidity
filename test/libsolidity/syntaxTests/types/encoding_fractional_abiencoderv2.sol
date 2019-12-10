pragma experimental ABIEncoderV2;
contract C {
   function f1() public pure returns (bytes memory) {
       return abi.encode(0.1, 1);
   }
}
// ----
// TypeError: (126-129): Fractional numbers cannot yet be encoded.
