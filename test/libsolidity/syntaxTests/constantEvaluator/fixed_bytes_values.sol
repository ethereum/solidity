contract C {
    bytes32 constant fromShortLiteral = "abc";
    bytes32 constant fromFullLiteral = "01234567890123456789012345678901";
    bytes32 constant fromHexLiteral = hex"00ff";
    bytes32 constant fromEmptyLiteral = "";
    bytes32 constant fromOtherConstant = fromShortLiteral;
    bytes32 constant negated = ~fromHexLiteral;
    bytes32 constant bitwise = (fromShortLiteral & fromHexLiteral) | (fromShortLiteral ^ fromHexLiteral);
    bytes32 constant shifted = (fromShortLiteral << 8) >> 4;
    bytes32 constant shiftedOut = fromShortLiteral << 300;
    bytes32 constant fromUnicode = unicode"日本語";

}
