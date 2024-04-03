contract C {
    event Ev();
    error Er();

    error e0();

    function h() public view {
        abi.encodeError(e0, Ev());
        abi.encodeError(e0, Er());
        abi.encodeError(e0, revert());
    }
}
// ----
// TypeError 9063: (122-126): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9063: (157-161): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9063: (192-200): Expected an inline tuple, not an expression of a tuple type.
