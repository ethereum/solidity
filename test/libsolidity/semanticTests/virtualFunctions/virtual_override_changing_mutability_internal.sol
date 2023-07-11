contract A {
    function f() internal virtual {
        mutableWithViewOverride();
        mutableWithPureOverride();
        viewWithPureOverride();
    }

    function mutableWithViewOverride() internal virtual {}
    function mutableWithPureOverride() internal virtual {}
    function viewWithPureOverride() internal view virtual {}
}

contract C is A {
    function run() public {
        f();
    }

    function mutableWithViewOverride() internal view override {}
    function mutableWithPureOverride() internal pure override {}
    function viewWithPureOverride() internal pure override {}
}
// ----
// run() ->
