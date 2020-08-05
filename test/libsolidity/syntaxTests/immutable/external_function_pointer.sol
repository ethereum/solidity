contract C {
    function() external immutable f;
}
// ----
// TypeError 3366: (17-48): Immutable variables of external function type are not yet supported.
