library Hooks {
    type Callback is function(uint256) returns (uint256);
}

contract C {
    Hooks.Callback cb;

    function double(uint256 x) internal pure returns (uint256) {
        return x * 2;
    }

    function triple(uint256 x) internal pure returns (uint256) {
        return x * 3;
    }

    function setCb(Hooks.Callback _cb) internal {
        cb = _cb;
    }

    function setup() public {
        setCb(Hooks.Callback.wrap(double));
    }

    function callCb(uint256 x) public returns (uint256) {
        function(uint256) returns (uint256) fn = Hooks.Callback.unwrap(cb);
        return fn(x);
    }

    function wrapAndCall(uint256 x) public returns (uint256) {
        Hooks.Callback wrapped = Hooks.Callback.wrap(triple);
        function(uint256) returns (uint256) fn = Hooks.Callback.unwrap(wrapped);
        return fn(x);
    }
}
// ----
// setup() ->
// callCb(uint256): 5 -> 10
// wrapAndCall(uint256): 7 -> 21
