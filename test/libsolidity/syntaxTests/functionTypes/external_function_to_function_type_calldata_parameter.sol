// This is a test that checks that the type of the `bytes` parameter is
// correctly changed from its own type `bytes calldata` to `bytes memory`
// when converting to a function type.
contract C {
    function f(function(bytes memory) pure external /*g*/) pure public { }
    function callback(bytes calldata) pure external {}
    function g() view public {
        f(this.callback);
    }
}
