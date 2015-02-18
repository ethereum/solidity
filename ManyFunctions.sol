
// Based on input param calls ~100 functions from ~200, random algorithm is really bad.
contract ManyFunctions {
    
    function start(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**78)
            return left1(r);
        return right1(r);
    }
    
    function finish(uint seed) returns (uint) {
        return seed;
    }
     
    function nextRand(uint seed) returns (uint) {
        var a = 39948330534945941795786356397633709378407037920056054402537049186942880579585;
        return a * seed + 1;
    }

    function right100(uint seed) returns (uint) {
        return finish(seed);
    }

    function left100(uint seed) returns (uint) {
        return finish(nextRand(seed));
    }

    function right1(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**79)
            return right2(r);
        return left2(r);
    }
    
    function left1(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**79)
            return left2(r);
        return right2(r);
    }


    function right2(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**80)
            return right3(r);
        return left3(r);
    }
    
    function left2(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**80)
            return left3(r);
        return right3(r);
    }


    function right3(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**81)
            return right4(r);
        return left4(r);
    }
    
    function left3(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**81)
            return left4(r);
        return right4(r);
    }


    function right4(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**82)
            return right5(r);
        return left5(r);
    }
    
    function left4(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**82)
            return left5(r);
        return right5(r);
    }


    function right5(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**83)
            return right6(r);
        return left6(r);
    }
    
    function left5(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**83)
            return left6(r);
        return right6(r);
    }


    function right6(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**84)
            return right7(r);
        return left7(r);
    }
    
    function left6(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**84)
            return left7(r);
        return right7(r);
    }


    function right7(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**85)
            return right8(r);
        return left8(r);
    }
    
    function left7(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**85)
            return left8(r);
        return right8(r);
    }


    function right8(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**86)
            return right9(r);
        return left9(r);
    }
    
    function left8(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**86)
            return left9(r);
        return right9(r);
    }


    function right9(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**87)
            return right10(r);
        return left10(r);
    }
    
    function left9(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**87)
            return left10(r);
        return right10(r);
    }


    function right10(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**88)
            return right11(r);
        return left11(r);
    }
    
    function left10(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**88)
            return left11(r);
        return right11(r);
    }


    function right11(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**89)
            return right12(r);
        return left12(r);
    }
    
    function left11(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**89)
            return left12(r);
        return right12(r);
    }


    function right12(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**90)
            return right13(r);
        return left13(r);
    }
    
    function left12(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**90)
            return left13(r);
        return right13(r);
    }


    function right13(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**91)
            return right14(r);
        return left14(r);
    }
    
    function left13(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**91)
            return left14(r);
        return right14(r);
    }


    function right14(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**92)
            return right15(r);
        return left15(r);
    }
    
    function left14(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**92)
            return left15(r);
        return right15(r);
    }


    function right15(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**93)
            return right16(r);
        return left16(r);
    }
    
    function left15(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**93)
            return left16(r);
        return right16(r);
    }


    function right16(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**94)
            return right17(r);
        return left17(r);
    }
    
    function left16(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**94)
            return left17(r);
        return right17(r);
    }


    function right17(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**95)
            return right18(r);
        return left18(r);
    }
    
    function left17(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**95)
            return left18(r);
        return right18(r);
    }


    function right18(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**96)
            return right19(r);
        return left19(r);
    }
    
    function left18(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**96)
            return left19(r);
        return right19(r);
    }


    function right19(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**97)
            return right20(r);
        return left20(r);
    }
    
    function left19(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**97)
            return left20(r);
        return right20(r);
    }


    function right20(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**98)
            return right21(r);
        return left21(r);
    }
    
    function left20(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**98)
            return left21(r);
        return right21(r);
    }


    function right21(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**99)
            return right22(r);
        return left22(r);
    }
    
    function left21(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**99)
            return left22(r);
        return right22(r);
    }


    function right22(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**100)
            return right23(r);
        return left23(r);
    }
    
    function left22(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**100)
            return left23(r);
        return right23(r);
    }


    function right23(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**101)
            return right24(r);
        return left24(r);
    }
    
    function left23(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**101)
            return left24(r);
        return right24(r);
    }


    function right24(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**102)
            return right25(r);
        return left25(r);
    }
    
    function left24(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**102)
            return left25(r);
        return right25(r);
    }


    function right25(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**103)
            return right26(r);
        return left26(r);
    }
    
    function left25(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**103)
            return left26(r);
        return right26(r);
    }


    function right26(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**104)
            return right27(r);
        return left27(r);
    }
    
    function left26(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**104)
            return left27(r);
        return right27(r);
    }


    function right27(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**105)
            return right28(r);
        return left28(r);
    }
    
    function left27(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**105)
            return left28(r);
        return right28(r);
    }


    function right28(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**106)
            return right29(r);
        return left29(r);
    }
    
    function left28(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**106)
            return left29(r);
        return right29(r);
    }


    function right29(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**107)
            return right30(r);
        return left30(r);
    }
    
    function left29(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**107)
            return left30(r);
        return right30(r);
    }


    function right30(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**108)
            return right31(r);
        return left31(r);
    }
    
    function left30(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**108)
            return left31(r);
        return right31(r);
    }


    function right31(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**109)
            return right32(r);
        return left32(r);
    }
    
    function left31(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**109)
            return left32(r);
        return right32(r);
    }


    function right32(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**110)
            return right33(r);
        return left33(r);
    }
    
    function left32(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**110)
            return left33(r);
        return right33(r);
    }


    function right33(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**111)
            return right34(r);
        return left34(r);
    }
    
    function left33(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**111)
            return left34(r);
        return right34(r);
    }


    function right34(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**112)
            return right35(r);
        return left35(r);
    }
    
    function left34(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**112)
            return left35(r);
        return right35(r);
    }


    function right35(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**113)
            return right36(r);
        return left36(r);
    }
    
    function left35(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**113)
            return left36(r);
        return right36(r);
    }


    function right36(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**114)
            return right37(r);
        return left37(r);
    }
    
    function left36(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**114)
            return left37(r);
        return right37(r);
    }


    function right37(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**115)
            return right38(r);
        return left38(r);
    }
    
    function left37(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**115)
            return left38(r);
        return right38(r);
    }


    function right38(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**116)
            return right39(r);
        return left39(r);
    }
    
    function left38(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**116)
            return left39(r);
        return right39(r);
    }


    function right39(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**117)
            return right40(r);
        return left40(r);
    }
    
    function left39(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**117)
            return left40(r);
        return right40(r);
    }


    function right40(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**118)
            return right41(r);
        return left41(r);
    }
    
    function left40(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**118)
            return left41(r);
        return right41(r);
    }


    function right41(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**119)
            return right42(r);
        return left42(r);
    }
    
    function left41(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**119)
            return left42(r);
        return right42(r);
    }


    function right42(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**120)
            return right43(r);
        return left43(r);
    }
    
    function left42(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**120)
            return left43(r);
        return right43(r);
    }


    function right43(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**121)
            return right44(r);
        return left44(r);
    }
    
    function left43(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**121)
            return left44(r);
        return right44(r);
    }


    function right44(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**122)
            return right45(r);
        return left45(r);
    }
    
    function left44(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**122)
            return left45(r);
        return right45(r);
    }


    function right45(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**123)
            return right46(r);
        return left46(r);
    }
    
    function left45(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**123)
            return left46(r);
        return right46(r);
    }


    function right46(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**124)
            return right47(r);
        return left47(r);
    }
    
    function left46(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**124)
            return left47(r);
        return right47(r);
    }


    function right47(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**125)
            return right48(r);
        return left48(r);
    }
    
    function left47(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**125)
            return left48(r);
        return right48(r);
    }


    function right48(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**126)
            return right49(r);
        return left49(r);
    }
    
    function left48(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**126)
            return left49(r);
        return right49(r);
    }


    function right49(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**127)
            return right50(r);
        return left50(r);
    }
    
    function left49(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**127)
            return left50(r);
        return right50(r);
    }


    function right50(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**128)
            return right51(r);
        return left51(r);
    }
    
    function left50(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**128)
            return left51(r);
        return right51(r);
    }


    function right51(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**129)
            return right52(r);
        return left52(r);
    }
    
    function left51(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**129)
            return left52(r);
        return right52(r);
    }


    function right52(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**130)
            return right53(r);
        return left53(r);
    }
    
    function left52(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**130)
            return left53(r);
        return right53(r);
    }


    function right53(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**131)
            return right54(r);
        return left54(r);
    }
    
    function left53(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**131)
            return left54(r);
        return right54(r);
    }


    function right54(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**132)
            return right55(r);
        return left55(r);
    }
    
    function left54(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**132)
            return left55(r);
        return right55(r);
    }


    function right55(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**133)
            return right56(r);
        return left56(r);
    }
    
    function left55(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**133)
            return left56(r);
        return right56(r);
    }


    function right56(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**134)
            return right57(r);
        return left57(r);
    }
    
    function left56(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**134)
            return left57(r);
        return right57(r);
    }


    function right57(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**135)
            return right58(r);
        return left58(r);
    }
    
    function left57(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**135)
            return left58(r);
        return right58(r);
    }


    function right58(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**136)
            return right59(r);
        return left59(r);
    }
    
    function left58(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**136)
            return left59(r);
        return right59(r);
    }


    function right59(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**137)
            return right60(r);
        return left60(r);
    }
    
    function left59(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**137)
            return left60(r);
        return right60(r);
    }


    function right60(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**138)
            return right61(r);
        return left61(r);
    }
    
    function left60(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**138)
            return left61(r);
        return right61(r);
    }


    function right61(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**139)
            return right62(r);
        return left62(r);
    }
    
    function left61(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**139)
            return left62(r);
        return right62(r);
    }


    function right62(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**140)
            return right63(r);
        return left63(r);
    }
    
    function left62(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**140)
            return left63(r);
        return right63(r);
    }


    function right63(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**141)
            return right64(r);
        return left64(r);
    }
    
    function left63(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**141)
            return left64(r);
        return right64(r);
    }


    function right64(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**142)
            return right65(r);
        return left65(r);
    }
    
    function left64(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**142)
            return left65(r);
        return right65(r);
    }


    function right65(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**143)
            return right66(r);
        return left66(r);
    }
    
    function left65(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**143)
            return left66(r);
        return right66(r);
    }


    function right66(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**144)
            return right67(r);
        return left67(r);
    }
    
    function left66(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**144)
            return left67(r);
        return right67(r);
    }


    function right67(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**145)
            return right68(r);
        return left68(r);
    }
    
    function left67(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**145)
            return left68(r);
        return right68(r);
    }


    function right68(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**146)
            return right69(r);
        return left69(r);
    }
    
    function left68(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**146)
            return left69(r);
        return right69(r);
    }


    function right69(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**147)
            return right70(r);
        return left70(r);
    }
    
    function left69(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**147)
            return left70(r);
        return right70(r);
    }


    function right70(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**148)
            return right71(r);
        return left71(r);
    }
    
    function left70(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**148)
            return left71(r);
        return right71(r);
    }


    function right71(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**149)
            return right72(r);
        return left72(r);
    }
    
    function left71(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**149)
            return left72(r);
        return right72(r);
    }


    function right72(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**150)
            return right73(r);
        return left73(r);
    }
    
    function left72(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**150)
            return left73(r);
        return right73(r);
    }


    function right73(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**151)
            return right74(r);
        return left74(r);
    }
    
    function left73(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**151)
            return left74(r);
        return right74(r);
    }


    function right74(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**152)
            return right75(r);
        return left75(r);
    }
    
    function left74(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**152)
            return left75(r);
        return right75(r);
    }


    function right75(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**153)
            return right76(r);
        return left76(r);
    }
    
    function left75(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**153)
            return left76(r);
        return right76(r);
    }


    function right76(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**154)
            return right77(r);
        return left77(r);
    }
    
    function left76(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**154)
            return left77(r);
        return right77(r);
    }


    function right77(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**155)
            return right78(r);
        return left78(r);
    }
    
    function left77(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**155)
            return left78(r);
        return right78(r);
    }


    function right78(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**156)
            return right79(r);
        return left79(r);
    }
    
    function left78(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**156)
            return left79(r);
        return right79(r);
    }


    function right79(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**157)
            return right80(r);
        return left80(r);
    }
    
    function left79(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**157)
            return left80(r);
        return right80(r);
    }


    function right80(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**158)
            return right81(r);
        return left81(r);
    }
    
    function left80(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**158)
            return left81(r);
        return right81(r);
    }


    function right81(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**159)
            return right82(r);
        return left82(r);
    }
    
    function left81(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**159)
            return left82(r);
        return right82(r);
    }


    function right82(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**160)
            return right83(r);
        return left83(r);
    }
    
    function left82(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**160)
            return left83(r);
        return right83(r);
    }


    function right83(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**161)
            return right84(r);
        return left84(r);
    }
    
    function left83(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**161)
            return left84(r);
        return right84(r);
    }


    function right84(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**162)
            return right85(r);
        return left85(r);
    }
    
    function left84(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**162)
            return left85(r);
        return right85(r);
    }


    function right85(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**163)
            return right86(r);
        return left86(r);
    }
    
    function left85(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**163)
            return left86(r);
        return right86(r);
    }


    function right86(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**164)
            return right87(r);
        return left87(r);
    }
    
    function left86(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**164)
            return left87(r);
        return right87(r);
    }


    function right87(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**165)
            return right88(r);
        return left88(r);
    }
    
    function left87(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**165)
            return left88(r);
        return right88(r);
    }


    function right88(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**166)
            return right89(r);
        return left89(r);
    }
    
    function left88(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**166)
            return left89(r);
        return right89(r);
    }


    function right89(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**167)
            return right90(r);
        return left90(r);
    }
    
    function left89(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**167)
            return left90(r);
        return right90(r);
    }


    function right90(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**168)
            return right91(r);
        return left91(r);
    }
    
    function left90(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**168)
            return left91(r);
        return right91(r);
    }


    function right91(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**169)
            return right92(r);
        return left92(r);
    }
    
    function left91(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**169)
            return left92(r);
        return right92(r);
    }


    function right92(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**170)
            return right93(r);
        return left93(r);
    }
    
    function left92(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**170)
            return left93(r);
        return right93(r);
    }


    function right93(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**171)
            return right94(r);
        return left94(r);
    }
    
    function left93(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**171)
            return left94(r);
        return right94(r);
    }


    function right94(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**172)
            return right95(r);
        return left95(r);
    }
    
    function left94(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**172)
            return left95(r);
        return right95(r);
    }


    function right95(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**173)
            return right96(r);
        return left96(r);
    }
    
    function left95(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**173)
            return left96(r);
        return right96(r);
    }


    function right96(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**174)
            return right97(r);
        return left97(r);
    }
    
    function left96(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**174)
            return left97(r);
        return right97(r);
    }


    function right97(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**175)
            return right98(r);
        return left98(r);
    }
    
    function left97(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**175)
            return left98(r);
        return right98(r);
    }


    function right98(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**176)
            return right99(r);
        return left99(r);
    }
    
    function left98(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**176)
            return left99(r);
        return right99(r);
    }


    function right99(uint seed) returns (uint) {
        var r = nextRand(seed);
        if (r >= 2**177)
            return right100(r);
        return left100(r);
    }
    
    function left99(uint seed) returns (uint) {
        var r = nextRand(nextRand(seed));
        if (r >= 2**177)
            return left100(r);
        return right100(r);
    }

}