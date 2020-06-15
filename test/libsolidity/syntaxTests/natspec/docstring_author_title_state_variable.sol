contract C {
    /// @title title
    /// @author author
    uint private state;
}
// ----
// DocstringParsingError: (17-56): Documentation tag @author not valid for non-public state variables.
// DocstringParsingError: (17-56): Documentation tag @title not valid for non-public state variables.
