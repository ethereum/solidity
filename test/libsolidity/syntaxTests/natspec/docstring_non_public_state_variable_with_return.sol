contract test {
  /// @return returns something
  uint private state;
}
// ----
// DocstringParsingError 6546: (18-47): Documentation tag @return not valid for non-public state variables.
