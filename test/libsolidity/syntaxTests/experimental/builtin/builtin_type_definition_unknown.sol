pragma experimental solidity;

type someUnknownType = __builtin("someUnknownType");
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 7758: (31-81): Expected the name of a built-in primitive type.
