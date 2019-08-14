pragma experimental ABIEncoderV2;
contract Test {
    struct S { int a; }
    function f(S calldata s) external { s = S(2); }
    function g(S calldata s) external { S memory m; s = m; }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (114-115): External function arguments of reference type are read-only.
// TypeError: (118-122): Type struct Test.S memory is not implicitly convertible to expected type struct Test.S calldata.
// TypeError: (178-179): External function arguments of reference type are read-only.
// TypeError: (182-183): Type struct Test.S memory is not implicitly convertible to expected type struct Test.S calldata.
