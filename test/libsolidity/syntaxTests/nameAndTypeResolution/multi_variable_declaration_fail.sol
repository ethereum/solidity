contract C { function f() public { var (x,y); x = 1; y = 1;} }
// ----
// Warning: (40-41): Use of the "var" keyword is deprecated.
// Warning: (42-43): Use of the "var" keyword is deprecated.
// TypeError: (35-44): Assignment necessary for type detection.
