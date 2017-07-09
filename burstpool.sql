DROP TABLE IF EXISTS Accounts;
CREATE TABLE Accounts (
			accountID					bigint unsigned not null unique,
			first_seen_when				timestamp not null default current_timestamp,
			reward_recipient			bigint unsigned,
			last_updated_when			timestamp null default null,
			account_name				varchar(255),
			mining_capacity				bigint unsigned,		# GiB
			is_capacity_estimated		boolean,
			account_RS_string			varchar(255) not null,
			has_used_this_pool			boolean not null default false,
			last_checked_at_block		bigint unsigned,
			last_nonce_when				timestamp null default null,
			primary key					(accountID)
		);
DROP TABLE IF EXISTS Blocks;
CREATE TABLE Blocks (
			blockID						bigint unsigned not null unique,
			first_seen_when				timestamp not null default current_timestamp,
			best_nonce_account_id		bigint unsigned,
			generator_account_id		bigint unsigned,
			block_reward				bigint unsigned,
			is_our_block				boolean not null default false,
			has_been_shared				boolean not null default false,
			base_target					bigint unsigned,
			forged_when					timestamp null default null,
			scoop						int unsigned,
			nonce						bigint unsigned,
			generation_signature		char(64),
			deadline					int unsigned,
			our_best_deadline			int unsigned,
			num_potential_miners		int unsigned,
			num_rejected_nonces			int unsigned not null default 0,
			primary key					(blockID),
			index						(is_our_block, has_been_shared)
		);
DROP TABLE IF EXISTS Bonuses;
CREATE TABLE Bonuses (
			bonusID						serial,
			tx_id						bigint unsigned not null,
			amount						bigint unsigned not null,
			seen_when					timestamp not null default current_timestamp,
			primary key					(bonusID),
			index						(tx_id)
		);
DROP TABLE IF EXISTS Nonces;
CREATE TABLE Nonces (
			accountID					bigint unsigned not null,
			blockID						bigint unsigned not null,
			nonce						bigint unsigned not null,
			submitted_when				timestamp not null default current_timestamp,
			deadline					bigint unsigned not null,
			deadline_string				varchar(255) not null,
			forge_when					timestamp not null default "0000-00-00 00:00:00",
			miner						varchar(255),
			primary key					(accountID, blockID, nonce),
			index						(blockID, accountID),
			index						(blockID, deadline desc)
		);
DROP TABLE IF EXISTS Rewards;
CREATE TABLE Rewards (
			rewardID					serial,
			accountID					bigint unsigned not null,
			blockID						bigint unsigned not null,
			bonusID						bigint unsigned,						# set if reward based on a bonus payment
			amount						bigint unsigned not null,
			is_paid						boolean not null default false,
			tx_id						bigint unsigned,
			is_confirmed				boolean not null default false,
			paid_at_block_id			bigint unsigned,
			primary key					(rewardID),
			index						(is_paid, is_confirmed, accountID, blockID)
		);
DROP TABLE IF EXISTS Shares;
CREATE TABLE Shares (
			blockID						bigint unsigned not null,
			accountID					bigint unsigned not null,
			share_fraction				double not null,
			deadline					int unsigned not null,
			deadline_string				varchar(255) not null,
			miner						varchar(255),
			primary key					(blockID, accountID),
			index						(blockID, share_fraction)
		);
