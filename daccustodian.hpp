#include <eosiolib/eosio.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/eosio.hpp>

using namespace eosio;
using namespace std;

// This is a reference to the member struct as used in the eosdactoken contract.
// @abi table members
struct member {
    name sender;
    /// Hash of agreed terms
    uint64_t agreedterms;

    name primary_key() const { return sender; }

    EOSLIB_SERIALIZE(member, (sender)(agreedterms))
};

// This is a reference to the termsinfo struct as used in the eosdactoken contract.
struct termsinfo {
  string terms;
  string hash;
  uint64_t version;

  uint64_t primary_key() const { return version; }
  EOSLIB_SERIALIZE(termsinfo, (terms)(hash)(version))
};

typedef multi_index<N(memberterms), termsinfo> memterms;

struct account {
    asset    balance;

    uint64_t primary_key()const { return balance.symbol.name(); }
};

typedef multi_index<N(members), member> regmembers;
typedef eosio::multi_index<N(accounts), account> accounts;

// @abi table configs
struct contr_config {
//    The amount of assets that are locked up by each candidate applying for election.
    asset lockupasset;
//    The maximum number of votes that each member can make for a candidate.
    uint8_t maxvotes = 5;
//    Number of custodians to be elected for each election count.
    uint8_t numelected = 3;
//    Length of a period in seconds.
//     - used for pay calculations if an eary election is called and to trigger deferred `newperiod` calls.
    uint32_t periodlength = 7 * 24 * 60 * 60;
    //The eosdac compatible token contract this contract should call to for member reg info
    name tokencontr;

    EOSLIB_SERIALIZE(contr_config, (lockupasset)(maxvotes)(numelected)(periodlength)(tokencontr))
};

typedef singleton<N(config), contr_config> configscontainer;

struct contr_state {
    uint32_t lastperiodtime = 0;

    EOSLIB_SERIALIZE(contr_state, (lastperiodtime))
};

typedef singleton<N(state), contr_state> statecontainer;

// Uitility to combine ids to help with indexing.
uint128_t combine_ids(const uint8_t &boolvalue, const uint64_t &longValue) {
    return (uint128_t{boolvalue} << 8) | longValue;
}

struct candidate {
    name candidate_name;
    string bio;
    // Active requested pay used for payment calculations.
    asset requestedpay;
    // Requested pay that would be pending until the new period begins. Then it should be moved to requestedpay.
    asset pendreqpay;
    asset locked_tokens;
    uint64_t total_votes;

    name primary_key() const { return candidate_name; }

    uint64_t by_number_votes() const { return static_cast<uint64_t>(total_votes); }
    uint64_t by_votes_rank() const { return static_cast<uint64_t>(UINT64_MAX - total_votes); }
    uint64_t by_pending_pay() const { return static_cast<uint64_t>(pendreqpay.amount); }

    EOSLIB_SERIALIZE(candidate,
                     (candidate_name)(bio)(requestedpay)(pendreqpay)(locked_tokens)(total_votes))
};

typedef multi_index<N(candidates), candidate,
        indexed_by<N(byvotes), const_mem_fun<candidate, uint64_t, &candidate::by_number_votes> >,
        indexed_by<N(byvotesrank), const_mem_fun<candidate, uint64_t, &candidate::by_votes_rank> >,
        indexed_by<N(bypendingpay), const_mem_fun<candidate, uint64_t, &candidate::by_pending_pay> >
> candidates_table;

// @abi table votes
struct vote {
    name voter;
    name proxy;
    int64_t weight;
    vector<name> candidates;

    account_name primary_key() const { return voter; }

    account_name by_proxy() const { return static_cast<uint64_t>(proxy); }

    EOSLIB_SERIALIZE(vote, (voter)(proxy)(weight)(candidates))
};

typedef eosio::multi_index<N(votes), vote,
        indexed_by<N(byproxy), const_mem_fun<vote, account_name, &vote::by_proxy> >
> votes_table;

// @abi table pendingpay
struct pay {
    uint64_t key;
    name receiver;
    asset quantity;
    string memo;

    account_name primary_key() const { return key; }

    EOSLIB_SERIALIZE(pay, (key)(receiver)(quantity)(memo))
};

typedef multi_index<N(pendingpay), pay> pending_pay_table;

// @abi table pendingstake
struct tempstake {
    account_name sender;
    asset quantity;
    string memo;

    account_name primary_key() const { return sender; }

    EOSLIB_SERIALIZE(tempstake, (sender)(quantity)(memo))
};

typedef multi_index<N(pendingstake), tempstake> pendingstake_table_t;
