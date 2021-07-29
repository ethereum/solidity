fixed constant a = fixed(bytes17(0));
bytes17 constant b = bytes17(fixed(0));
fixed constant c = fixed(bytes15(0));
bytes15 constant d = bytes15(fixed(0));

fixed16x1 constant e = fixed16x1(bytes3(0));
bytes3 constant f = bytes3(fixed16x1(0));
fixed16x1 constant g = fixed16x1(bytes1(0));
bytes1 constant h = bytes1(fixed16x1(0));
// ----
// TypeError 9640: (19-36): Explicit type conversion not allowed from "bytes17" to "fixed128x18".
// TypeError 9640: (59-76): Explicit type conversion not allowed from "fixed128x18" to "bytes17".
// TypeError 9640: (97-114): Explicit type conversion not allowed from "bytes15" to "fixed128x18".
// TypeError 9640: (137-154): Explicit type conversion not allowed from "fixed128x18" to "bytes15".
// TypeError 9640: (180-200): Explicit type conversion not allowed from "bytes3" to "fixed16x1".
// TypeError 9640: (222-242): Explicit type conversion not allowed from "fixed16x1" to "bytes3".
// TypeError 9640: (267-287): Explicit type conversion not allowed from "bytes1" to "fixed16x1".
// TypeError 9640: (309-329): Explicit type conversion not allowed from "fixed16x1" to "bytes1".
