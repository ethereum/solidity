contract test {
  /// @return returns something
  uint private state;
}
// ----
// DocstringParsingError 6546: (18-47='/// @return returns something'): Documentation tag @return not valid for non-public state variables.
