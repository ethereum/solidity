pragma experimental solidity;

function f() {}

class Self: C
{
    function g(self: Self, x: f);
}
// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError 2217: (94-95): Attempt to type identifier referring to unexpected node.
