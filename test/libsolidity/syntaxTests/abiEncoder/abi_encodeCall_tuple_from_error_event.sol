contract C {
    event Ev();
    error Er();

    function f0() public {}

    function h() public view {
        abi.encodeCall(this.f0, Ev());
        abi.encodeCall(this.f0, Er());
        abi.encodeCall(this.f0, revert());
    }
}
// ----
// TypeError 9062: (138-142): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (177-181): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (216-224): Expected an inline tuple, not an expression of a tuple type.
