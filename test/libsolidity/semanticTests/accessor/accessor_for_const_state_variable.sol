contract Lotto {
    uint256 public constant ticketPrice = 555;
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// ticketPrice() -> 555
