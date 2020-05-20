contract C {
    /// @notice example of notice
    /// @dev example of dev
    uint private state;
}
// ----
// DocstringParsingError: (17-74): Documentation tag @notice not valid for non-public state variables.
