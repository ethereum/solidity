type MyFunction is function(MyFunction) external returns(MyFunction);
// ----
// TypeError 8657: (19-69): The underlying type of the user defined value type "MyFunction" has to be an internal function type, but "function (MyFunction) external returns (MyFunction)" is an external function type.
