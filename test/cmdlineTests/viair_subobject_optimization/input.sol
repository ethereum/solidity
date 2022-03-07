// SPDX-License-Identifier: GPL-3.0
pragma solidity >0.0.0;

contract C {
  constructor(uint x) {
    // In earlier versions of the compiler, the resulting assembly pushed the constant
    // 0xFFFFFFFFFFFFFFFF42 directly in the subassembly of D, while it was optimized to
    // ``sub(shl(0x48, 0x01), 0xbe)`` when C was compiled in isolation.
    // Now the assembly is expected to contain two instances of ``sub(shl(0x48, 0x01), 0xbe)``,
    // one in the creation code of ``C`` directly, one in a subassembly of ``D``.
    // The constant 0xFFFFFFFFFFFFFFFF42 should not occur in the assembly output at all.
    if (x == 0xFFFFFFFFFFFFFFFF42)
      revert();
  }
}
contract D {
  function f() public pure returns (bytes memory) {
    return type(C).creationCode;
  }
}
