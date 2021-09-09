type MyFunction is function(MyFunction) external returns(MyFunction);
// ----
// TypeError 8657: (19-69): The underlying type for a user defined value type has to be an elementary value type.
