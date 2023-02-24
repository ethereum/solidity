function suffix1(uint) pure suffix suffix returns (uint) {}
function suffix2(uint) suffix pure suffix returns (uint) {}
function suffix3(uint) suffix pure suffix suffix suffix returns (uint) {}
// ----
// ParserError 2878: (35-41): Suffix already specified.
// ParserError 2878: (95-101): Suffix already specified.
// ParserError 2878: (155-161): Suffix already specified.
// ParserError 2878: (162-168): Suffix already specified.
// ParserError 2878: (169-175): Suffix already specified.
