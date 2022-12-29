contract C {
    uint private internal a;
    uint private public b;
    uint private private c;
    uint internal internal e;
    uint internal public f;
    uint internal private g;
    uint public internal h;
    uint public public i;
    uint public private j;
    function f1() private external {}
    function f2() private internal {}
    function f3() private public {}
    function f4() private private {}
    function f5() external external {}
    function f6() external internal {}
    function f7() external public {}
    function f8() external private {}
    function f9() internal external {}
    function f10() internal internal {}
    function f11() internal public {}
    function f12() internal private {}
    function f13() public external {}
    function f14() public internal {}
    function f15() public public {}
    function f16() public private {}
}
// ----
// ParserError 4110: (30-38): Visibility already specified as "private".
// ParserError 4110: (59-65): Visibility already specified as "private".
// ParserError 4110: (86-93): Visibility already specified as "private".
// ParserError 4110: (115-123): Visibility already specified as "internal".
// ParserError 4110: (145-151): Visibility already specified as "internal".
// ParserError 4110: (173-180): Visibility already specified as "internal".
// ParserError 4110: (200-208): Visibility already specified as "public".
// ParserError 4110: (228-234): Visibility already specified as "public".
// ParserError 4110: (254-261): Visibility already specified as "public".
// ParserError 9439: (291-299): Visibility already specified as "private".
// ParserError 9439: (329-337): Visibility already specified as "private".
// ParserError 9439: (367-373): Visibility already specified as "private".
// ParserError 9439: (403-410): Visibility already specified as "private".
// ParserError 9439: (441-449): Visibility already specified as "external".
// ParserError 9439: (480-488): Visibility already specified as "external".
// ParserError 9439: (519-525): Visibility already specified as "external".
// ParserError 9439: (556-563): Visibility already specified as "external".
// ParserError 9439: (594-602): Visibility already specified as "internal".
// ParserError 9439: (634-642): Visibility already specified as "internal".
// ParserError 9439: (674-680): Visibility already specified as "internal".
// ParserError 9439: (712-719): Visibility already specified as "internal".
// ParserError 9439: (749-757): Visibility already specified as "public".
// ParserError 9439: (787-795): Visibility already specified as "public".
// ParserError 9439: (825-831): Visibility already specified as "public".
// ParserError 9439: (861-868): Visibility already specified as "public".
