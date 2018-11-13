contract Test
{
  uint y;
  function internalPureFunc(uint256 x) internal pure returns (uint256) { return x; }
  function internalViewFunc(uint256 x) internal view returns (uint256) { return x + y; }
  function internalMutableFunc(uint256 x) internal returns (uint256) { y = x; return x; }

  function externalPureFunc(uint256 x) external pure returns (uint256) { return x; }
  function externalViewFunc(uint256 x) external view returns (uint256) { return x + y; }
  function externalPayableFunc(uint256 x) external payable returns (uint256) { return x + y; }
  function externalMutableFunc(uint256 x) external returns (uint256) { y = x; return x; }

  function funcTakesInternalPure(function(uint256) internal pure returns(uint256) a) internal pure returns (uint256) { return a(4); }
  function funcTakesInternalView(function(uint256) internal view returns(uint256) a) internal view returns (uint256) { return a(4); }
  function funcTakesInternalMutable(function(uint256) internal returns(uint256) a) internal returns (uint256) { return a(4); }

  function funcTakesExternalPure(function(uint256) external pure returns(uint256) a) internal pure returns (uint256) { return a(4); }
  function funcTakesExternalView(function(uint256) external view returns(uint256) a) internal view returns (uint256) { return a(4); }
  function funcTakesExternalPayable(function(uint256) external payable returns(uint256) a) internal returns (uint256) { return a(4); }
  function funcTakesExternalMutable(function(uint256) external returns(uint256) a) internal returns (uint256) { return a(4); }

  function tests() internal
  {
    funcTakesInternalPure(internalPureFunc);

    funcTakesInternalView(internalPureFunc);
    funcTakesInternalView(internalViewFunc);

    funcTakesInternalMutable(internalPureFunc);
    funcTakesInternalMutable(internalViewFunc);
    funcTakesInternalMutable(internalMutableFunc);

    funcTakesExternalPure(this.externalPureFunc);

    funcTakesExternalView(this.externalPureFunc);
    funcTakesExternalView(this.externalViewFunc);

    funcTakesExternalPayable(this.externalPayableFunc);

    funcTakesExternalMutable(this.externalPureFunc);
    funcTakesExternalMutable(this.externalViewFunc);
    funcTakesExternalMutable(this.externalPayableFunc);
    funcTakesExternalMutable(this.externalMutableFunc);
  }
}
// ----
