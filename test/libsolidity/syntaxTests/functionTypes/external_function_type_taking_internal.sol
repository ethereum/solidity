contract C {
    function(function () internal) external x;
}
// ----
// TypeError 2582: (26-47='function () internal)'): Internal type cannot be used for external function type.
