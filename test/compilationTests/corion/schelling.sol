pragma solidity ^0.4.11;

import "./announcementTypes.sol";
import "./module.sol";
import "./moduleHandler.sol";
import "./safeMath.sol";

contract schellingVars {
    /*
        Common enumerations and structures of the Schelling and Database contract.
    */
    enum voterStatus {
        base,
        afterPrepareVote,
        afterSendVoteOk,
        afterSendVoteBad
    }
    struct _rounds {
        uint256 totalAboveWeight;
        uint256 totalBelowWeight;
        uint256 reward;
        uint256 blockHeight;
        bool voted;
    }
    struct _voter {
        uint256 roundID;
        bytes32 hash;
        voterStatus status;
        bool voteResult;
        uint256 rewards;
    }
}

contract schellingDB is safeMath, schellingVars {
    /*
        Schelling database contract.
    */
    address private owner;
    function replaceOwner(address newOwner) external returns(bool) {
        require( owner == address(0x00) || msg.sender == owner );
        owner = newOwner;
        return true;
    }
    modifier isOwner { require( msg.sender == owner ); _; }
    /*
        Constructor
    */
    constructor() {
        rounds.length = 2;
        rounds[0].blockHeight = block.number;
        currentSchellingRound = 1;
    }
    /*
        Funds
    */
    mapping(address => uint256) private funds;
    function getFunds(address _owner) constant returns(bool, uint256) {
        return (true, funds[_owner]);
    }
    function setFunds(address _owner, uint256 _amount) isOwner external returns(bool) {
        funds[_owner] = _amount;
        return true;
    }
    /*
        Rounds
    */
    _rounds[] private rounds;
    function getRound(uint256 _id) constant returns(bool, uint256, uint256, uint256, uint256, bool) {
        if ( rounds.length <= _id ) { return (false, 0, 0, 0, 0, false); }
        else { return (true, rounds[_id].totalAboveWeight, rounds[_id].totalBelowWeight, rounds[_id].reward, rounds[_id].blockHeight, rounds[_id].voted); }
    }
    function pushRound(uint256 _totalAboveWeight, uint256 _totalBelowWeight, uint256 _reward, uint256 _blockHeight, bool _voted) isOwner external returns(bool, uint256) {
        return (true, rounds.push(_rounds(_totalAboveWeight, _totalBelowWeight, _reward, _blockHeight, _voted)));
    }
    function setRound(uint256 _id, uint256 _totalAboveWeight, uint256 _totalBelowWeight, uint256 _reward, uint256 _blockHeight, bool _voted) isOwner external returns(bool) {
        rounds[_id] = _rounds(_totalAboveWeight, _totalBelowWeight, _reward, _blockHeight, _voted);
        return true;
    }
    function getCurrentRound() constant returns(bool, uint256) {
        return (true, rounds.length-1);
    }
    /*
        Voter
    */
    mapping(address => _voter) private voter;
    function getVoter(address _owner) constant returns(bool success, uint256 roundID,
        bytes32 hash, voterStatus status, bool voteResult, uint256 rewards) {
        roundID         = voter[_owner].roundID;
        hash            = voter[_owner].hash;
        status          = voter[_owner].status;
        voteResult      = voter[_owner].voteResult;
        rewards         = voter[_owner].rewards;
        success         = true;
    }
    function setVoter(address _owner, uint256 _roundID, bytes32 _hash, voterStatus _status, bool _voteResult, uint256 _rewards) isOwner external returns(bool) {
        voter[_owner] = _voter(
            _roundID,
            _hash,
            _status,
            _voteResult,
            _rewards
            );
        return true;
    }
    /*
        Schelling Token emission
    */
    mapping(uint256 => uint256) private schellingExpansion;
    function getSchellingExpansion(uint256 _id) constant returns(bool, uint256) {
        return (true, schellingExpansion[_id]);
    }
    function setSchellingExpansion(uint256 _id, uint256 _expansion) isOwner external returns(bool) {
        schellingExpansion[_id] = _expansion;
        return true;
    }
    /*
        Current Schelling Round
    */
    uint256 private currentSchellingRound;
    function setCurrentSchellingRound(uint256 _id) isOwner external returns(bool) {
        currentSchellingRound = _id;
        return true;
    }
    function getCurrentSchellingRound() constant returns(bool, uint256) {
        return (true, currentSchellingRound);
    }
}

contract schelling is module, announcementTypes, schellingVars {
    /*
        Schelling contract
    */
    /*
        module callbacks
    */
    function replaceModule(address addr) external returns (bool) {
        require( super.isModuleHandler(msg.sender) );
        require( db.replaceOwner(addr) );
        super._replaceModule(addr);
        return true;
    }
    function transferEvent(address from, address to, uint256 value) external returns (bool) {
        /*
            Transaction completed. This function can be called only by the ModuleHandler. 
            If this contract is the receiver, the amount will be added to the prize pool of the current round.
            
            @from      From who
            @to        To who
            @value     Amount
            @bool      Was the transaction succesfull?
        */
        require( super.isModuleHandler(msg.sender) );
        if ( to == address(this) ) {
            uint256 currentRound = getCurrentRound();
            schellingVars._rounds memory round = getRound(currentRound);
            round.reward += value;
            setRound(currentRound, round);
        }
        return true;
    }
    modifier isReady {
        (bool _success, bool _active) = super.isActive();
        require( _success && _active ); 
        _;
    }
    /*
        Schelling database functions.
    */
    function getFunds(address addr) internal returns (uint256) {
        (bool a, uint256 b) = db.getFunds(addr);
        require( a );
        return b;
    }
    function setFunds(address addr, uint256 amount) internal {
        require( db.setFunds(addr, amount) );
    }
    function setVoter(address owner, _voter voter) internal {
        require( db.setVoter(owner, 
            voter.roundID,
            voter.hash,
            voter.status,
            voter.voteResult,
            voter.rewards
            ) );
    }    
    function getVoter(address addr) internal returns (_voter) {
        (bool a, uint256 b, bytes32 c, schellingVars.voterStatus d, bool e, uint256 f) = db.getVoter(addr);
        require( a );
        return _voter(b, c, d, e, f);
    }
    function setRound(uint256 id, _rounds round) internal {
        require( db.setRound(id, 
            round.totalAboveWeight,
            round.totalBelowWeight,
            round.reward,
            round.blockHeight,
            round.voted
            ) );
    }
    function pushRound(_rounds round) internal returns (uint256) {
        (bool a, uint256 b) = db.pushRound( 
            round.totalAboveWeight,
            round.totalBelowWeight,
            round.reward,
            round.blockHeight,
            round.voted
            );
        require( a );
        return b;
    }
    function getRound(uint256 id) internal returns (_rounds) {
        (bool a, uint256 b, uint256 c, uint256 d, uint256 e, bool f) = db.getRound(id);
        require( a );
        return _rounds(b, c, d, e, f);
    }
    function getCurrentRound() internal returns (uint256) {
        (bool a, uint256 b) = db.getCurrentRound();
        require( a );
        return b;
    }
    function setCurrentSchellingRound(uint256 id) internal {
        require( db.setCurrentSchellingRound(id) );
    }
    function getCurrentSchellingRound() internal returns(uint256) {
        (bool a, uint256 b) = db.getCurrentSchellingRound();
        require( a );
        return b;
    }
    function setSchellingExpansion(uint256 id, uint256 amount) internal {
        require( db.setSchellingExpansion(id, amount) );
    }
    function getSchellingExpansion(uint256 id) internal returns(uint256) {
        (bool a, uint256 b) = db.getSchellingExpansion(id);
        require( a );
        return b;
    }
    /*
        Schelling module
    */
    uint256 private roundBlockDelay     = 720;
    uint8 private interestCheckRounds   = 7;
    uint8 private interestCheckAboves   = 4;
    uint256 private interestRate        = 300;
    uint256 private interestRateM       = 1e3;

    bytes1 public aboveChar = 0x31;
    bytes1 public belowChar = 0x30;
    schellingDB private db;
    
    constructor(address _moduleHandler, address _db, bool _forReplace) {
        /*
            Installation function.
            
            @_moduleHandler         Address of ModuleHandler.
            @_db                    Address of the database.
            @_forReplace            This address will be replaced with the old one or not.
            @_icoExpansionAddress   This address can turn schelling runds during ICO.
        */
        db = schellingDB(_db);
        super.registerModuleHandler(_moduleHandler);
        if ( ! _forReplace ) {
            require( db.replaceOwner(this) );
        }
    }
    function configure(announcementType a, uint256 b) external returns(bool) {
        /*
            Can be called only by the ModuleHandler.
            
            @a      Sort of configuration
            @b      Value
        */
        require( super.isModuleHandler(msg.sender) );
        if      ( a == announcementType.schellingRoundBlockDelay )     { roundBlockDelay = b; }
        else if ( a == announcementType.schellingCheckRounds )         { interestCheckRounds = uint8(b); }
        else if ( a == announcementType.schellingCheckAboves )         { interestCheckAboves = uint8(b); }
        else if ( a == announcementType.schellingRate )                { interestRate = b; }
        else { return false; }
        return true;
    }
    function prepareVote(bytes32 votehash, uint256 roundID) isReady noContract external {
        /*
            Initializing manual vote.
            Only the hash of vote will be sent. (Envelope sending). 
            The address must be in default state, that is there are no vote in progress. 
            Votes can be sent only on the actually Schelling round.
            
            @votehash               Hash of the vote
            @roundID                Number of Schelling round
        */
        nextRound();
        
        uint256 currentRound = getCurrentRound();
        schellingVars._rounds memory round = getRound(currentRound);
        _voter memory voter;
        uint256 funds;
        
        require( roundID == currentRound );
        
        voter = getVoter(msg.sender);
        funds = getFunds(msg.sender);

        require( funds > 0 );
        require( voter.status == voterStatus.base );
        voter.roundID = currentRound;
        voter.hash = votehash;
        voter.status = voterStatus.afterPrepareVote;
        
        setVoter(msg.sender, voter);
        round.voted = true;
        
        setRound(currentRound, round);
    }
    function sendVote(string vote) isReady noContract external {
        /*
            Check vote (Envelope opening)
            Only the sent “envelopes” can be opened.
            Envelope opening only in the next Schelling round.
            If the vote invalid, the deposit will be lost.
            If the “envelope” was opened later than 1,5 Schelling round, the vote is automatically invalid, and deposit can be lost.
            Lost deposits will be 100% burned.
            
            @vote      Hash of the content of the vote.
        */
        nextRound();
        
        uint256 currentRound = getCurrentRound();
        _rounds memory round;
        _voter memory voter;
        uint256 funds;
        
        bool lostEverything;
        voter = getVoter(msg.sender);
        round = getRound(voter.roundID);
        funds = getFunds(msg.sender);
        
        require( voter.status == voterStatus.afterPrepareVote );
        require( voter.roundID < currentRound );
        if ( keccak256(bytes(vote)) == voter.hash ) {
            delete voter.hash;
            if (round.blockHeight+roundBlockDelay/2 >= block.number) {
                if ( bytes(vote)[0] == aboveChar ) {
                    voter.status = voterStatus.afterSendVoteOk;
                    round.totalAboveWeight += funds;
                    voter.voteResult = true;
                } else if ( bytes(vote)[0] == belowChar ) {
                    voter.status = voterStatus.afterSendVoteOk;
                    round.totalBelowWeight += funds;
                } else { lostEverything = true; }
            } else {
                voter.status = voterStatus.afterSendVoteBad;
            }
        } else { lostEverything = true; }
        if ( lostEverything ) {
            require( moduleHandler(moduleHandlerAddress).burn(address(this), funds) );
            delete funds;
            delete voter.status;
        }
        
        setVoter(msg.sender, voter);
        setRound(voter.roundID, round);
        setFunds(msg.sender, funds);
    }
    function checkVote() isReady noContract external {
        /*
            Checking votes.
            Vote checking only after the envelope opening Schelling round.
            Deposit will be lost, if the vote wrong, or invalid.
            The right votes take share of deposits.
        */
        nextRound();
        
        uint256 currentRound = getCurrentRound();
        _rounds memory round;
        _voter memory voter;
        uint256 funds;
        
        voter = getVoter(msg.sender);
        round = getRound(voter.roundID);
        funds = getFunds(msg.sender);
        
        require( voter.status == voterStatus.afterSendVoteOk || 
            voter.status == voterStatus.afterSendVoteBad );
        if ( round.blockHeight+roundBlockDelay/2 <= block.number ) {
            if ( isWinner(round, voter.voteResult) && voter.status == voterStatus.afterSendVoteOk ) {
                voter.rewards += funds * round.reward / getRoundWeight(round.totalAboveWeight, round.totalBelowWeight);
            } else {
                require( moduleHandler(moduleHandlerAddress).burn(address(this), funds) );
                delete funds;
            }
            delete voter.status;
            delete voter.roundID;
        } else { throw; }
        
        setVoter(msg.sender, voter);
        setFunds(msg.sender, funds);
    }
    function getRewards(address beneficiary) isReady noContract external {
        /*
            Redeem of prize.
            The prizes will be collected here, and with this function can be transferred to the account of the user.
            Optionally there can be an address of a beneficiary added, which address the prize will be sent to. Without beneficiary, the owner is the default address.
            Prize will be sent from the Schelling address without any transaction fee.
            
            @beneficiary        Address of the beneficiary
        */
        schellingVars._voter memory voter = getVoter(msg.sender);
        uint256 funds = getFunds(msg.sender);
        
        address _beneficiary = msg.sender;
        if (beneficiary != address(0x00)) { _beneficiary = beneficiary; }
        uint256 reward;
        require( voter.rewards > 0 );
        require( voter.status == voterStatus.base );
        reward = voter.rewards;
        delete voter.rewards;
        require( moduleHandler(moduleHandlerAddress).transfer(address(this), _beneficiary, reward, false) );
            
        setVoter(msg.sender, voter);
    }
    function checkReward() public constant returns (uint256 reward) {
        /*
            Withdraw of the amount of the prize (it’s only information).
            
            @reward         Prize
        */
        schellingVars._voter memory voter = getVoter(msg.sender);
        return voter.rewards;
    }
    function nextRound() internal returns (bool) {
        /*
            Inside function, checks the time of the Schelling round and if its needed, creates a new Schelling round.
        */
        uint256 currentRound = getCurrentRound();
        _rounds memory round = getRound(currentRound);
        _rounds memory newRound;
        _rounds memory prevRound;
        uint256 currentSchellingRound = getCurrentSchellingRound();
        
        if ( round.blockHeight+roundBlockDelay > block.number) { return false; }
        
        newRound.blockHeight = block.number;
        if ( ! round.voted ) {
            newRound.reward = round.reward;
        }
        uint256 aboves;
        for ( uint256 a=currentRound ; a>=currentRound-interestCheckRounds ; a-- ) {
            if (a == 0) { break; }
            prevRound = getRound(a);
            if ( prevRound.totalAboveWeight > prevRound.totalBelowWeight ) { aboves++; }
        }
        uint256 expansion;
        if ( aboves >= interestCheckAboves ) {
            expansion = getTotalSupply() * interestRate / interestRateM / 100;
        }
        
        currentSchellingRound++;
        
        pushRound(newRound);
        setSchellingExpansion(currentSchellingRound, expansion);
        require( moduleHandler(moduleHandlerAddress).broadcastSchellingRound(currentSchellingRound, expansion) );
        return true;
    }
    function addFunds(uint256 amount) isReady noContract external {
        /*
            Deposit taking.
            Every participant entry with his own deposit.
            In case of wrong vote only this amount of deposit will be burn.
            The deposit will be sent to the address of Schelling, charged with transaction fee.
            
            @amount          Amount of deposit.
        */
        _voter memory voter = getVoter(msg.sender);
        uint256 funds = getFunds(msg.sender);
        
        (bool a, bool b) = moduleHandler(moduleHandlerAddress).isICO();
        require( b && b );
        require( voter.status == voterStatus.base );
        require( amount > 0 );
        require( moduleHandler(moduleHandlerAddress).transfer(msg.sender, address(this), amount, true) );
        funds += amount;
        
        setFunds(msg.sender, funds);
    }
    function getFunds() isReady noContract external {
        /*
            Deposit withdrawn.
            If the deposit isn’t lost, it can be withdrawn.
            By withdrawn, the deposit will be sent from Schelling address to the users address, charged with transaction fee..
        */
        _voter memory voter = getVoter(msg.sender);
        uint256 funds = getFunds(msg.sender);
        
        require( funds > 0 );
        require( voter.status == voterStatus.base );
        setFunds(msg.sender, 0);
        
        require( moduleHandler(moduleHandlerAddress).transfer(address(this), msg.sender, funds, true) );
    }
    function getCurrentSchellingRoundID() public constant returns (uint256) {
        /*
            Number of actual Schelling round.
            
            @uint256        Schelling round.
        */
        return getCurrentSchellingRound();
    }
    function getSchellingRound(uint256 id) public constant returns (uint256 expansion) {
        /*
            Amount of token emission of the Schelling round.
            
            @id             Number of Schelling round
            @expansion      Amount of token emission
        */
        return getSchellingExpansion(id);
    }
    function getRoundWeight(uint256 aboveW, uint256 belowW) internal returns (uint256) {
        /*
            Inside function for calculating the weights of the votes.
            
            @aboveW     Weight of votes: ABOVE
            @belowW     Weight of votes: BELOW
            @uint256    Calculatet weight
        */
        if ( aboveW == belowW ) {
            return aboveW + belowW;
        } else if ( aboveW > belowW ) {
            return aboveW;
        } else if ( aboveW < belowW) {
            return belowW;
        }
    }
    function isWinner(_rounds round, bool aboveVote) internal returns (bool) {
        /*
            Inside function for calculating the result of the game.
            
            @round      Structure of Schelling round.
            @aboveVote  Is the vote = ABOVE or not
            @bool       Result
        */
        if ( round.totalAboveWeight == round.totalBelowWeight ||
            ( round.totalAboveWeight > round.totalBelowWeight && aboveVote ) ) {
            return true;
        }
        return false;
    }
    
    function getTotalSupply() internal returns (uint256 amount) {
        /*
            Inside function for querying the whole amount of the tokens.
            
            @uint256        Whole token amount
        */
        (bool _success, uint256 _amount) = moduleHandler(moduleHandlerAddress).totalSupply();
        require( _success );
        return _amount;
    }
    
    function getTokenBalance(address addr) internal returns (uint256 balance) {
        /*
            Inner function in order to poll the token balance of the address.
            
            @addr       Address
            
            @balance    Balance of the address.
        */
        (bool _success, uint256 _balance) = moduleHandler(moduleHandlerAddress).balanceOf(addr);
        require( _success );
        return _balance;
    }
    
    modifier noContract {
        /*
            Contract can’t call this function, only a natural address.
        */
        require( msg.sender == tx.origin ); _;
    }
}
