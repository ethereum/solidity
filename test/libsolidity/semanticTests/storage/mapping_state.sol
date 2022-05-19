contract Ballot {
	mapping(address => bool) canVote;
	mapping(address => uint) voteCount;
	mapping(address => bool) voted;
	function getVoteCount(address addr) public returns (uint retVoteCount) {
		return voteCount[addr];
	}
	function grantVoteRight(address addr) public {
		canVote[addr] = true;
	}
	function vote(address voter, address vote) public returns (bool success) {
		if (!canVote[voter] || voted[voter]) return false;
		voted[voter] = true;
		voteCount[vote] = voteCount[vote] + 1;
		return true;
	}
}
// ----
// getVoteCount(address): 0 -> 0
// getVoteCount(address): 1 -> 0
// getVoteCount(address): 2 -> 0
// vote(address,address): 0, 2 -> false
// getVoteCount(address): 0 -> 0
// getVoteCount(address): 1 -> 0
// getVoteCount(address): 2 -> 0
// grantVoteRight(address): 0 ->
// grantVoteRight(address): 1 ->
// vote(address,address): 0, 2 -> true
// getVoteCount(address): 0 -> 0
// getVoteCount(address): 1 -> 0
// getVoteCount(address): 2 -> 1
// vote(address,address): 0, 1 -> false
// getVoteCount(address): 0 -> 0
// getVoteCount(address): 1 -> 0
// getVoteCount(address): 2 -> 1
// vote(address,address): 2, 1 -> false
// getVoteCount(address): 0 -> 0
// getVoteCount(address): 1 -> 0
// getVoteCount(address): 2 -> 1
// grantVoteRight(address): 2 ->
// vote(address,address): 2, 1 -> true
// getVoteCount(address): 0 -> 0
// getVoteCount(address): 1 -> 1
// getVoteCount(address): 2 -> 1
