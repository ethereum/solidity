********************
Secure Crowd Funding
********************

In this example We will see how to create an crowdfunding smart contract which will quite secure by moving the power of campaign's admin to those who donated money.

Let's first design all the things:

1. The Admin will start a campaign for crowdfunding with a specefic monetary goal and deadline.

2. Contributors will contribute to that project by sending eth.

3. The admin has to create a spending request to spend money for the campaign.

4. Once the spending request was created,the contributors can start voting for that spending request.

5. If more than 50% of the total contributors voted for the request,then the admin would have the permission to spend the amount specefic in the spending request.

6. The powe is moved from the campaign's admin to those that donated money.

7.The contributors can request a refund if the monetary goal was not reached within the deadline. 

The full contract
-----------------

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0

pragma solidity 0.8.8;
 
 error DeadlinePassed();
 error MinimumContributionNotMet();
 error DeadlineNotPassed();
 error TheGoalWasMet();
 error Unauthorized();
 error YouAreNotaContributor();
 error YouAlreadyVoted();
 error RequestAlreadyCompleted();
 error NeedMoreThan50percentContributors();
contract CrowdFunding {
    mapping(address => uint) public contributors;
    address public immutable admin;
    uint public noOfContributors;
    uint public minimumContribution;
    uint public deadline; //timestamp
    uint public goal;
    uint public raisedAmount;
  
    // Spending Request
    struct Request {
        string description;
        address payable recipient;
        uint value;
        bool completed;
        uint noOfVoters;
        mapping(address => bool) voters;
    }
    
    // mapping of spending requests
    // the key is the spending request number (index) - starts from zero
    // the value is a Request struct
    mapping(uint => Request) public requests;
    uint public numRequests;
    
    // events to emit
    event ContributeEvent(address _sender, uint _value);
    event CreateRequestEvent(string _description, address _recipient, uint _value);
    event MakePaymentEvent(address _recipient, uint _value);
    
    
    constructor(uint _goal, uint _deadline) {
        goal = _goal;
        deadline = block.timestamp + _deadline;
        admin = msg.sender;
        minimumContribution = 100 wei;
    }
    
   
    modifier onlyAdmin() {
        require(msg.sender == admin, "Only admin can execute this");
        _;
    }
    
    
    function contribute() public payable {
        if(block.timestamp >= deadline){
            revert DeadlinePassed();
        }
        if(msg.value < minimumContribution){
            revert MinimumContributionNotMet();
        }
     
        
        // incrementing the no. of contributors the first time when 
        // someone sends eth to the contract
        if(contributors[msg.sender] == 0) {
            noOfContributors++;
        }
        
        contributors[msg.sender] += msg.value;
        raisedAmount += msg.value;
        
        emit ContributeEvent(msg.sender, msg.value);
    }
    

 
    // a contributor can get a refund if goal was not reached within the deadline
    function getRefund() public {
        if(block.timestamp <= deadline){
            revert DeadlineNotPassed();
        }
       if(raisedAmount >= goal){
           revert TheGoalWasMet();
       }
       if(contributors[msg.sender] <= 0){
           revert Unauthorized();
       }
     
        
         payable(msg.sender).transfer(contributors[msg.sender]);
        
        contributors[msg.sender] = 0;
 
    }
    
    
    function createRequest(string calldata _description, address payable _recipient, uint _value) public onlyAdmin {
        //numRequests starts from zero
        Request storage newRequest = requests[numRequests];
        numRequests++;
        
        newRequest.description = _description;
        newRequest.recipient = _recipient;
        newRequest.value = _value;
        newRequest.completed = false;
        newRequest.noOfVoters = 0;
        
        emit CreateRequestEvent(_description, _recipient, _value);
    }
    
    
    function voteRequest(uint _requestNo) public {
        if(contributors[msg.sender]<=0){
            revert YouAreNotaContributor();
        }
     
        
        Request storage thisRequest = requests[_requestNo];
        if(thisRequest.voters[msg.sender] == true){
            revert YouAlreadyVoted();
        }
        
        
        thisRequest.voters[msg.sender] = true;
        thisRequest.noOfVoters++;
    }
    
    
    function makePayment(uint _requestNo) public onlyAdmin {
        Request storage thisRequest = requests[_requestNo];
        if(thisRequest.completed == true){
            revert RequestAlreadyCompleted();
        }
        
        if(thisRequest.noOfVoters <= noOfContributors / 2){
            revert NeedMoreThan50percentContributors();
        }
        
        
        thisRequest.recipient.transfer(thisRequest.value);
        thisRequest.completed = true;
        
        emit MakePaymentEvent(thisRequest.recipient, thisRequest.value);
    }
    
}