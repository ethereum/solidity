contract Test
{
  function internalPureFunc(uint256 x) internal pure returns (uint256) { return x; }
  function internalViewFunc(uint256 x) internal view returns (uint256) { return x; }
  function internalMutableFunc(uint256 x) internal returns (uint256) { return x; }

  function externalPureFunc(uint256 x) external pure returns (uint256) { return x; }
  function externalViewFunc(uint256 x) external view returns (uint256) { return x; }
  function externalPayableFunc(uint256 x) external payable returns (uint256) { return x; }
  function externalMutableFunc(uint256 x) external returns (uint256) { return x; }

  function funcTakesInternalPure(function(uint256) internal pure returns(uint256) a) internal returns (uint256) { return a(4); }
  function funcTakesInternalView(function(uint256) internal view returns(uint256) a) internal returns (uint256) { return a(4); }
  function funcTakesInternalMutable(function(uint256) internal returns(uint256) a) internal returns (uint256) { return a(4); }

  function funcTakesExternalPure(function(uint256) external pure returns(uint256) a) internal returns (uint256) { return a(4); }
  function funcTakesExternalView(function(uint256) external view returns(uint256) a) internal returns (uint256) { return a(4); }
  function funcTakesExternalPayable(function(uint256) external payable returns(uint256) a) internal returns (uint256) { return a(4); }
  function funcTakesExternalMutable(function(uint256) external returns(uint256) a) internal returns (uint256) { return a(4); }

  function tests() internal
  {
    funcTakesInternalPure(internalViewFunc); // view -> pure should fail
    funcTakesInternalPure(internalMutableFunc); // mutable -> pure should fail

    funcTakesInternalView(internalMutableFunc); // mutable -> view should fail

    funcTakesExternalPure(this.externalViewFunc); // view -> pure should fail
    funcTakesExternalPure(this.externalPayableFunc); // payable -> pure should fail
    funcTakesExternalPure(this.externalMutableFunc); // mutable -> pure should fail

    funcTakesExternalView(this.externalPayableFunc); // payable -> view should fail
    funcTakesExternalView(this.externalMutableFunc); // mutable -> view should fail

    funcTakesExternalPayable(this.externalPureFunc); // pure -> payable should fail
    funcTakesExternalPayable(this.externalViewFunc); // view -> payable should fail
    funcTakesExternalPayable(this.externalMutableFunc); // mutable -> payable should fail
  }
}
// ----
// TypeError: (1580-1596): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) view returns (uint256) to function (uint256) pure returns (uint256) requested.
// TypeError: (1653-1672): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) returns (uint256) to function (uint256) pure returns (uint256) requested.
// TypeError: (1733-1752): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) returns (uint256) to function (uint256) view returns (uint256) requested.
// TypeError: (1813-1834): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) view external returns (uint256) to function (uint256) pure external returns (uint256) requested.
// TypeError: (1891-1915): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) payable external returns (uint256) to function (uint256) pure external returns (uint256) requested.
// TypeError: (1975-1999): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) external returns (uint256) to function (uint256) pure external returns (uint256) requested.
// TypeError: (2060-2084): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) payable external returns (uint256) to function (uint256) view external returns (uint256) requested.
// TypeError: (2144-2168): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) external returns (uint256) to function (uint256) view external returns (uint256) requested.
// TypeError: (2232-2253): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) pure external returns (uint256) to function (uint256) payable external returns (uint256) requested.
// TypeError: (2316-2337): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) view external returns (uint256) to function (uint256) payable external returns (uint256) requested.
// TypeError: (2400-2424): Invalid type for argument in function call. Invalid implicit conversion from function (uint256) external returns (uint256) to function (uint256) payable external returns (uint256) requested.
