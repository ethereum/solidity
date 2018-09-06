contract C {
    function() external returns (function () internal) x;
}
// ----
// TypeError: (46-67): Internal type cannot be used for external function type.
