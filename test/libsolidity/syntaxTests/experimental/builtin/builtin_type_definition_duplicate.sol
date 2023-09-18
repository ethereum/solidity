pragma experimental solidity;

type void1 = __builtin("void");
type void2 = __builtin("void");

type word1 = __builtin("word");
type word2 = __builtin("word");

type fun1(T, U) = __builtin("fun");
type fun2(T, U) = __builtin("fun");
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 9609: (63-94): Duplicate builtin type definition.
// TypeError 9609: (128-159): Duplicate builtin type definition.
// TypeError 9609: (197-232): Duplicate builtin type definition.
