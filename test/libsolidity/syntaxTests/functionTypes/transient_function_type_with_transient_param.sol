contract C {
    function (uint transient x) external transient y;
}
// ----
// ParserError 2314: (42-43): Expected ',' but got identifier
