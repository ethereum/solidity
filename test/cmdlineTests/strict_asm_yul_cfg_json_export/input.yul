/// @use-src 0:"input.sol"
object "C_19" {
  code {
    {
      /// @src 0:124:223  "contract C is I {..."
      let _1 := memoryguard(0x80)
      mstore(64, _1)
      if callvalue() { revert(0, 0) }
      let _2 := datasize("C_19_deployed")
      codecopy(_1, dataoffset("C_19_deployed"), _2)
      return(_1, _2)
    }
  }
  /// @use-src 0:"input.sol"
  object "C_19_deployed" {
    code {
      {
        /// @src 0:124:223  "contract C is I {..."
        let _1 := memoryguard(0x80)
        mstore(64, _1)
        if iszero(lt(calldatasize(), 4))
        {
          if eq(0x26121ff0, shr(224, calldataload(0)))
          {
            if callvalue() { revert(0, 0) }
            if slt(add(calldatasize(), not(3)), 0) { revert(0, 0) }
            mstore(_1, /** @src 0:212:214  "42" */ 0x2a)
            /// @src 0:124:223  "contract C is I {..."
            return(_1, 32)
          }
        }
        revert(0, 0)
      }
    }
  }
}
