contract C {
    function f() public {
        function() external returns (function() internal) getCallback;
        getCallback();
    }
}
// ----
// TypeError 2582: (76-96): Internal type cannot be used for external function type.
