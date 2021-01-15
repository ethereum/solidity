import "~~~some-long-unlikely-file-name-12";
import "~~~some-long-unlikely-file-name-456";
import "~~~some-long-unlikely-file-name-7890";
// This test is here to verify that the license/pragma preamble added by the test suite does not
// affect source locations in error messages. Positions in messages below should start at 0 and
// there should be no gaps between the ranges.
// ----
// ParserError 6275: (0-44): Source "~~~some-long-unlikely-file-name-12" not found: File not supplied initially.
// ParserError 6275: (45-90): Source "~~~some-long-unlikely-file-name-456" not found: File not supplied initially.
// ParserError 6275: (91-137): Source "~~~some-long-unlikely-file-name-7890" not found: File not supplied initially.
