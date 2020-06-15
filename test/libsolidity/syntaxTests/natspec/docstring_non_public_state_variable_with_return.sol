contract test {
  /// @return returns something
  uint private state;
}
// ----
// DocstringParsingError: (18-47): Documentation tag @return not valid for non-public state variables.
