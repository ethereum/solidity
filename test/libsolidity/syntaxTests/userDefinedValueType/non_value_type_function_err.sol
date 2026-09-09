type MyFunction is function(uint) external returns (uint);
// ----
// TypeError 8657: (19-58): The underlying type of the user defined value type "MyFunction" has to be an internal function type, but "function (uint256) external returns (uint256)" is an external function type.
