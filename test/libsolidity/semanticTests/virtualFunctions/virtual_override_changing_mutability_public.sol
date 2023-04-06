contract A {
    function f() internal virtual {
        mutableWithViewOverride();
        mutableWithPureOverride();
        viewWithPureOverride();
    }

    function mutableWithViewOverride() public virtual {}
    function mutableWithPureOverride() public virtual {}
    function viewWithPureOverride() public view virtual {}
}

contract C is A {
    function run() public {
        f();
    }

    function mutableWithViewOverride() public view override {}
    function mutableWithPureOverride() public pure override {}
    function viewWithPureOverride() public pure override {}
}
// ----
// run() ->
