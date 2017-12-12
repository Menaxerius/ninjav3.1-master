insert into migration.Accounts select accountID, first_seen_when, reward_recipient, last_updated_when, account_name, estimated_capacity, null, account_RS_string, has_used_this_pool, null, null from burstpool.Accounts;

insert into migration.Blocks select blockID, first_seen_when, best_nonce_account_id, generator_account_id, block_reward, is_our_block, has_been_shared, base_target, forged_when, scoop, nonce, gen_sig_str, deadline, our_best_deadline, num_potential_miners, 0 from burstpool.Blocks where burstpool.Blocks.blockID >= 350000;

insert into migration.Nonces select accountID, blockID, nonce, submitted_when, deadline, deadline_string, forge_when, miner from burstpool.Nonces where burstpool.Nonces.blockID >= 350000;

insert into migration.Rewards select null, accountID, blockID, null, amount, is_paid, tx_id, is_confirmed, paid_at_block_id from burstpool.Rewards;

insert into migration.Shares select blockID, accountID, share_fraction, deadline, deadline_string, miner from burstpool.Shares where burstpool.Shares.blockID >= 350000;
