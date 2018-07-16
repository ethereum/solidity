pragma solidity ^0.4.6;

/*
    Copyright 2016, Jordi Baylina

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @title MilestoneTracker Contract
/// @author Jordi Baylina
/// @dev This contract tracks the


/// is rules the relation between a donor and a recipient
///  in order to guaranty to the donor that the job will be done and to guaranty
///  to the recipient that he will be paid


/// @dev We use the RLP library to decode RLP so that the donor can approve one
///  set of milestone changes at a time.
///  https://github.com/androlo/standard-contracts/blob/master/contracts/src/codec/RLP.sol
import "RLP.sol";



/// @dev This contract allows for `recipient` to set and modify milestones
contract MilestoneTracker {
    using RLP for RLP.RLPItem;
    using RLP for RLP.Iterator;
    using RLP for bytes;

    struct Milestone {
        string description;     // Description of this milestone
        string url;             // A link to more information (swarm gateway)
        uint minCompletionDate; // Earliest UNIX time the milestone can be paid
        uint maxCompletionDate; // Latest UNIX time the milestone can be paid
        address milestoneLeadLink;
                                // Similar to `recipient`but for this milestone
        address reviewer;       // Can reject the completion of this milestone
        uint reviewTime;        // How many seconds the reviewer has to review
        address paymentSource;  // Where the milestone payment is sent from
        bytes payData;          // Data defining how much ether is sent where

        MilestoneStatus status; // Current status of the milestone
                                // (Completed, AuthorizedForPayment...)
        uint doneTime;          // UNIX time when the milestone was marked DONE
    }

    // The list of all the milestones.
    Milestone[] public milestones;

    address public recipient;   // Calls functions in the name of the recipient
    address public donor;       // Calls functions in the name of the donor
    address public arbitrator;  // Calls functions in the name of the arbitrator

    enum MilestoneStatus {
        AcceptedAndInProgress,
        Completed,
        AuthorizedForPayment,
        Canceled
    }

    // True if the campaign has been canceled
    bool public campaignCanceled;

    // True if an approval on a change to `milestones` is a pending
    bool public changingMilestones;

    // The pending change to `milestones` encoded in RLP
    bytes public proposedMilestones;


    /// @dev The following modifiers only allow specific roles to call functions
    /// with these modifiers
    modifier onlyRecipient { if (msg.sender !=  recipient) revert(); _; }
    modifier onlyArbitrator { if (msg.sender != arbitrator) revert(); _; }
    modifier onlyDonor { if (msg.sender != donor) revert(); _; }

    /// @dev The following modifiers prevent functions from being called if the
    /// campaign has been canceled or if new milestones are being proposed
    modifier campaignNotCanceled { if (campaignCanceled) revert(); _; }
    modifier notChanging { if (changingMilestones) revert(); _; }

 // @dev Events to make the payment movements easy to find on the blockchain
    event NewMilestoneListProposed();
    event NewMilestoneListUnproposed();
    event NewMilestoneListAccepted();
    event ProposalStatusChanged(uint idProposal, MilestoneStatus newProposal);
    event CampaignCanceled();


///////////
// Constructor
///////////

    /// @notice The Constructor creates the Milestone contract on the blockchain
    /// @param _arbitrator Address assigned to be the arbitrator
    /// @param _donor Address assigned to be the donor
    /// @param _recipient Address assigned to be the recipient
    constructor (
        address _arbitrator,
        address _donor,
        address _recipient
    ) public {
        arbitrator = _arbitrator;
        donor = _donor;
        recipient = _recipient;
    }


/////////
// Helper functions
/////////

    /// @return The number of milestones ever created even if they were canceled
    function numberOfMilestones() public view returns (uint) {
        return milestones.length;
    }


////////
// Change players
////////

    /// @notice `onlyArbitrator` Reassigns the arbitrator to a new address
    /// @param _newArbitrator The new arbitrator
    function changeArbitrator(address _newArbitrator) public onlyArbitrator {
        arbitrator = _newArbitrator;
    }

    /// @notice `onlyDonor` Reassigns the `donor` to a new address
    /// @param _newDonor The new donor
    function changeDonor(address _newDonor) public onlyDonor {
        donor = _newDonor;
    }

    /// @notice `onlyRecipient` Reassigns the `recipient` to a new address
    /// @param _newRecipient The new recipient
    function changeRecipient(address _newRecipient) public onlyRecipient {
        recipient = _newRecipient;
    }


////////////
// Creation and modification of Milestones
////////////

    /// @notice `onlyRecipient` Proposes new milestones or changes old
    ///  milestones, this will require a user interface to be built up to
    ///  support this functionality as asks for RLP encoded bytecode to be
    ///  generated, until this interface is built you can use this script:
    ///  https://github.com/Giveth/milestonetracker/blob/master/js/milestonetracker_helper.js
    ///  the functions milestones2bytes and bytes2milestones will enable the
    ///  recipient to encode and decode a list of milestones, also see
    ///  https://github.com/Giveth/milestonetracker/blob/master/README.md
    /// @param _newMilestones The RLP encoded list of milestones; each milestone
    ///  has these fields:
    ///       string description,
    ///       string url,
    ///       uint minCompletionDate,  // seconds since 1/1/1970 (UNIX time)
    ///       uint maxCompletionDate,  // seconds since 1/1/1970 (UNIX time)
    ///       address milestoneLeadLink,
    ///       address reviewer,
    ///       uint reviewTime
    ///       address paymentSource,
    ///       bytes payData,
    function proposeMilestones(bytes memory _newMilestones
    ) public onlyRecipient campaignNotCanceled {
        proposedMilestones = _newMilestones;
        changingMilestones = true;
        emit NewMilestoneListProposed();
    }


////////////
// Normal actions that will change the state of the milestones
////////////

    /// @notice `onlyRecipient` Cancels the proposed milestones and reactivates
    ///  the previous set of milestones
    function unproposeMilestones() public onlyRecipient campaignNotCanceled {
        delete proposedMilestones;
        changingMilestones = false;
        emit NewMilestoneListUnproposed();
    }

    /// @notice `onlyDonor` Approves the proposed milestone list
    /// @param _hashProposals The keccak256() of the proposed milestone list's
    ///  bytecode; this confirms that the `donor` knows the set of milestones
    ///  they are approving
    function acceptProposedMilestones(bytes32 _hashProposals
    ) public onlyDonor campaignNotCanceled {

        uint i;

        if (!changingMilestones) revert();
        if (keccak256(proposedMilestones) != _hashProposals) revert();

        // Cancel all the unfinished milestones
        for (i=0; i<milestones.length; i++) {
            if (milestones[i].status != MilestoneStatus.AuthorizedForPayment) {
                milestones[i].status = MilestoneStatus.Canceled;
            }
        }
        // Decode the RLP encoded milestones and add them to the milestones list
        bytes memory mProposedMilestones = proposedMilestones;

        RLP.RLPItem memory itmProposals = mProposedMilestones.toRLPItem(true);

        if (!itmProposals.isList()) revert();

        RLP.Iterator memory itrProposals = itmProposals.iterator();

        while(itrProposals.hasNext()) {


            RLP.RLPItem memory itmProposal = itrProposals.next();

            Milestone storage milestone = milestones[milestones.length ++];

            if (!itmProposal.isList()) revert();

            RLP.Iterator memory itrProposal = itmProposal.iterator();

            milestone.description = itrProposal.next().toAscii();
            milestone.url = itrProposal.next().toAscii();
            milestone.minCompletionDate = itrProposal.next().toUint();
            milestone.maxCompletionDate = itrProposal.next().toUint();
            milestone.milestoneLeadLink = itrProposal.next().toAddress();
            milestone.reviewer = itrProposal.next().toAddress();
            milestone.reviewTime = itrProposal.next().toUint();
            milestone.paymentSource = itrProposal.next().toAddress();
            milestone.payData = itrProposal.next().toData();

            milestone.status = MilestoneStatus.AcceptedAndInProgress;

        }

        delete proposedMilestones;
        changingMilestones = false;
        emit NewMilestoneListAccepted();
    }

    /// @notice `onlyRecipientOrLeadLink`Marks a milestone as DONE and
    ///  ready for review
    /// @param _idMilestone ID of the milestone that has been completed
    function markMilestoneComplete(uint _idMilestone)
        public campaignNotCanceled notChanging
    {
        if (_idMilestone >= milestones.length) revert();
        Milestone storage milestone = milestones[_idMilestone];
        if (  (msg.sender != milestone.milestoneLeadLink)
            &&(msg.sender != recipient))
            revert();
        if (milestone.status != MilestoneStatus.AcceptedAndInProgress) revert();
        if (now < milestone.minCompletionDate) revert();
        if (now > milestone.maxCompletionDate) revert();
        milestone.status = MilestoneStatus.Completed;
        milestone.doneTime = now;
        emit ProposalStatusChanged(_idMilestone, milestone.status);
    }

    /// @notice `onlyReviewer` Approves a specific milestone
    /// @param _idMilestone ID of the milestone that is approved
    function approveCompletedMilestone(uint _idMilestone)
        public campaignNotCanceled notChanging
    {
        if (_idMilestone >= milestones.length) revert();
        Milestone storage milestone = milestones[_idMilestone];
        if ((msg.sender != milestone.reviewer) ||
            (milestone.status != MilestoneStatus.Completed)) revert();

        authorizePayment(_idMilestone);
    }

    /// @notice `onlyReviewer` Rejects a specific milestone's completion and
    ///  reverts the `milestone.status` back to the `AcceptedAndInProgress`
    ///  state
    /// @param _idMilestone ID of the milestone that is being rejected
    function rejectMilestone(uint _idMilestone)
        public campaignNotCanceled notChanging
    {
        if (_idMilestone >= milestones.length) revert();
        Milestone storage milestone = milestones[_idMilestone];
        if ((msg.sender != milestone.reviewer) ||
            (milestone.status != MilestoneStatus.Completed)) revert();

        milestone.status = MilestoneStatus.AcceptedAndInProgress;
        emit ProposalStatusChanged(_idMilestone, milestone.status);
    }

    /// @notice `onlyRecipientOrLeadLink` Sends the milestone payment as
    ///  specified in `payData`; the recipient can only call this after the
    ///  `reviewTime` has elapsed
    /// @param _idMilestone ID of the milestone to be paid out
    function requestMilestonePayment(uint _idMilestone
        ) public campaignNotCanceled notChanging {
        if (_idMilestone >= milestones.length) revert();
        Milestone storage milestone = milestones[_idMilestone];
        if (  (msg.sender != milestone.milestoneLeadLink)
            &&(msg.sender != recipient))
            revert();
        if  ((milestone.status != MilestoneStatus.Completed) ||
             (now < milestone.doneTime + milestone.reviewTime))
            revert();

        authorizePayment(_idMilestone);
    }

    /// @notice `onlyRecipient` Cancels a previously accepted milestone
    /// @param _idMilestone ID of the milestone to be canceled
    function cancelMilestone(uint _idMilestone)
        public onlyRecipient campaignNotCanceled notChanging
    {
        if (_idMilestone >= milestones.length) revert();
        Milestone storage milestone = milestones[_idMilestone];
        if  ((milestone.status != MilestoneStatus.AcceptedAndInProgress) &&
             (milestone.status != MilestoneStatus.Completed))
            revert();

        milestone.status = MilestoneStatus.Canceled;
        emit ProposalStatusChanged(_idMilestone, milestone.status);
    }

    /// @notice `onlyArbitrator` Forces a milestone to be paid out as long as it
    /// has not been paid or canceled
    /// @param _idMilestone ID of the milestone to be paid out
    function arbitrateApproveMilestone(uint _idMilestone
    ) public onlyArbitrator campaignNotCanceled notChanging {
        if (_idMilestone >= milestones.length) revert();
        Milestone storage milestone = milestones[_idMilestone];
        if  ((milestone.status != MilestoneStatus.AcceptedAndInProgress) &&
             (milestone.status != MilestoneStatus.Completed))
           revert();
        authorizePayment(_idMilestone);
    }

    /// @notice `onlyArbitrator` Cancels the entire campaign voiding all
    ///  milestones.
    function arbitrateCancelCampaign() public onlyArbitrator campaignNotCanceled {
        campaignCanceled = true;
        emit CampaignCanceled();
    }

    // @dev This internal function is executed when the milestone is paid out
    function authorizePayment(uint _idMilestone) internal {
        if (_idMilestone >= milestones.length) revert();
        Milestone storage milestone = milestones[_idMilestone];
        // Recheck again to not pay twice
        if (milestone.status == MilestoneStatus.AuthorizedForPayment) revert();
        milestone.status = MilestoneStatus.AuthorizedForPayment;
        if (!milestone.paymentSource.call.value(0)(milestone.payData))
            revert();
        emit ProposalStatusChanged(_idMilestone, milestone.status);
    }
}
