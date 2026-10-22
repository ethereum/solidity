type Callback is function(uint256) returns (bool);

library Hooks {
    type BeforeSwap is function(uint256) returns (bool);
}

contract C {
    Hooks.BeforeSwap cb;

    function set(Hooks.BeforeSwap _cb) internal {
        cb = _cb;
    }

    function wrapUnwrap() internal pure {
        function(uint256) returns (bool) raw;
        Callback wrapped = Callback.wrap(raw);
        function(uint256) returns (bool) unwrapped = Callback.unwrap(wrapped);
        unwrapped;
    }

    function wrapUnwrapLib() internal pure {
        function(uint256) returns (bool) raw;
        Hooks.BeforeSwap wrapped = Hooks.BeforeSwap.wrap(raw);
        function(uint256) returns (bool) unwrapped = Hooks.BeforeSwap.unwrap(wrapped);
        unwrapped;
    }
}
