pragma experimental ABIEncoderV2;

contract C {
  struct T { U u; V v; }

  struct U { W w; }

  struct V { W w; }

  struct W { uint x; }

  function f(T memory) public pure { }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
