/* File: monattk.c */

/* Monster melee attacks.  Monster critical blows, whether a monster 
 * attack hits, insult messages.  The code used when a monster attacks 
 * an adjacent player, including descriptions and effects.
 *
 * Copyright (c) 2001 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"





/*
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(int dice, int sides, int dam)
{
	int max = 0;
	int total = dice * sides;

	/* Must do at least 90% of perfect */
	if (dam < total * 19 / 20) return (0);

	/* Randomize. */
	if (rand_int(3) == 0) return (0);

	/* Weak blows rarely work */
	if ((dam < 20) && (dam < randint(100))) return (0);

	/* Perfect damage */
	if (dam == total) max++;

	/* Super-charge */
	if (dam > 19)
	{
		while (rand_int(100) < 2) max++;
	}

	/* Critical damage */
	if (dam > 45) return (6 + max);
	if (dam > 33) return (5 + max);
	if (dam > 25) return (4 + max);
	if (dam > 18) return (3 + max);
	if (dam > 11) return (2 + max);
	return (1 + max);
}





/*
 * Determine if a monster attack against the player succeeds.
 * Now incorporates the effects of terrain and penalizes stunned monsters. -LM-
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match monster power against player armor.
 */
static int check_hit(int power, int level, int terrain_bonus, int m_idx)
{
	int i, k, ac;

	monster_type *m_ptr = &m_list[m_idx];

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Always miss or hit */
	if (k < 10) return (k < 5);

	/* Calculate the "attack quality".  Stunned monsters are hindered. */
	i = (power + (m_ptr->stunned ? level * 2 : level * 3));

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a + terrain_bonus;

	/* Power and Level compete against Armor */
	if ((i > 0) && (randint(i) > ((ac * 3) / 4)))
	{
		return (TRUE);
	}

	/* Assume miss */
	return (FALSE);
}



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_insult[] =
{
	"insults you!",
	"insults your mother!",
	"gives you the finger!",
	"humiliates you!",
	"defiles you!",
	"dances around you!",
	"makes obscene gestures!",
	"moons you!!!"
};



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_sneer[] =
{
	"offers you a pony for an outrageous sum.",
	"waits to tell the Black Riders where you've gone.",
	"tells you to clear out, or he'll break your neck.",
	"sneers at the company you keep."
};

/*
 * Mega Hack, but I don't care --- Ask for and take some survival item, 
 * depending on what the player has, then give some money in exchange, 
 * teleport away, and delete the monster.
 */
static void make_request(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	object_type *o_ptr;


	int i;
	int requested_slot = 0;
	int requested_number = 0;
	int sanity_check = 0;
	int offer_price = 0;

	char m_name[80];
	char o_name[120];


	/* Get the monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Decide what and how much to ask for. */
	while(TRUE)
	{
		/* Increment the loop count. */
		sanity_check++;

		/* Select a random object in the pack. */
		i = rand_int(INVEN_PACK);
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Skip anything that isn't a torch, flask of oil, food ration, 
		 * or scroll of recall.
		 */
		switch(o_ptr->tval)
		{
			case TV_LITE:
			{
				if (o_ptr->sval == SV_LITE_TORCH) requested_slot = i;
				break;
			}
			case TV_FLASK:
			{
				requested_slot = i;

				break;
			}
			case TV_FOOD:
			{
				if (o_ptr->sval == SV_FOOD_RATION) 
					requested_slot = i;
				break;
			}
			case TV_SCROLL:
			{
				if (o_ptr->sval == SV_SCROLL_WORD_OF_RECALL) 
					requested_slot = i;
				break;
			}
			default:
			{
				break;
			}
		}

		/* Found an appropriate item. */
		if (requested_slot)
		{
			object_kind *k_ptr = &k_info[o_ptr->k_idx];

			/* I know what I want.  Now I need to figure how much of it I 
			 * can get away with.
			 */
			requested_number = o_ptr->number;
			if (requested_number > 10) 
				requested_number = 9 + (requested_number / 5);

			/* I must offer a fair price, plus some (I'm desperate). */
			offer_price = (k_ptr->cost * requested_number) + 
				((5 + randint(5)) * 100L);

			/* Done. */
			break;
		}

		/* Player doesn't have anything of interest. */
		if (sanity_check > 50)
		{
			/* Let the player know. */
			msg_format("%^s looks doleful.  You don't have anything of interest.", m_name);

			/* Nothing requested. */
			requested_slot = 0;

			/* Done. */
			break;
		}
	}

	if (requested_slot)
	{
		/* Get the object. */
		o_ptr = &inventory[requested_slot];

		/* Acquire the item name. */
		object_desc(o_name, o_ptr, FALSE, 3);


		/* Try to make a deal for the item. */
		msg_format("%^s looks longingly at your abundant supplies:", m_name);
		msg_format("'Kind Sir, I desperately need %d %s.  I will gladly give %d gold in exchange.'", requested_number, o_name, offer_price);

		/* Make the trade. */
		if (get_check("Accept the offer? "))
		{
			/* Be friendly. */
			msg_print(NULL);
			msg_print("May you have all your heart's desire!");

			/* Take the item. */
			inven_item_increase(i, -requested_number);
			inven_item_describe(i);
			inven_item_optimize(i);

			/* Give gold. */
			p_ptr->au += offer_price;

			/* Redraw gold */
			p_ptr->redraw |= (PR_GOLD);
		}

		/* How can you be so stingy! */
		else
		{
			/* Complain bitterly. */
			msg_print(NULL);
			msg_print("You scummy excuse for a kobold crook!  May jackals gnaw your bones!");
		}
	}

	/* Hack -- Teleport the monster away and delete it (to prevent the player 
	 * getting rich).
	 */
	teleport_away(m_idx, 50);
	msg_format("%^s runs off.", m_name);

	/* If the monster is a unique, it will never come back. */
	if (r_ptr->flags1 & (RF1_UNIQUE)) r_ptr->max_num = 0;

	/* Delete the monster. */
	delete_monster_idx(m_idx);
}



/*
 * Attack the player via physical attacks.
 */
bool make_attack_normal(monster_type *m_ptr, int y, int x)
{
	int m_idx = cave_m_idx[m_ptr->fy][m_ptr->fx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	int ap_cnt;

	int i, j, k, tmp, ac, rlev;
	int do_cut, do_stun;

	int terrain_bonus = 0;

	s32b gold;

	object_type *o_ptr;
	object_kind *k_ptr;

	char o_name[120];

	char m_name[80];

	char ddesc[80];

	bool blinked;

	/* Check Bounds */
	if (!in_bounds(y, x)) return (FALSE);

	/* Not allowed to attack */
	if (r_ptr->flags1 & (RF1_NEVER_BLOW)) return (FALSE);

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


	/* Get the monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, m_ptr, 0x88);


	/* Players in rubble can take advantage of cover. */
	if (cave_feat[y][x] == FEAT_RUBBLE)
	{
		terrain_bonus = ac / 8 + 5;
	}
	/* Players in trees can take advantage of cover, especially rangers and druids. */
	if (cave_feat[y][x] == FEAT_TREE)
	{
		if ((p_ptr->pclass == CLASS_RANGER) || (p_ptr->pclass == CLASS_DRUID))
			terrain_bonus = ac / 8 + 10;
		else terrain_bonus = ac / 10 + 2;
	}
	/* Players in water are vulnerable. */
	if (cave_feat[y][x] == FEAT_WATER)
	{
		terrain_bonus = -(ac / 3);
	}


	/* Assume no blink */
	blinked = FALSE;

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		bool visible = FALSE;
		bool obvious = FALSE;

		int power = 0;
		int damage = 0;

		cptr act = NULL;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[ap_cnt].effect;
		int method = r_ptr->blow[ap_cnt].method;
		int d_dice = r_ptr->blow[ap_cnt].d_dice;
		int d_side = r_ptr->blow[ap_cnt].d_side;


		/* Hack -- no more attacks */
		if (!method) break;


		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Extract visibility (before blink) */
		if (m_ptr->ml) visible = TRUE;



		/* Extract the attack "power".  Elemental attacks upgraded. */
		switch (effect)
		{
			case RBE_HURT:		power = 60; break;
			case RBE_POISON:		power =	 25; break;
			case RBE_UN_BONUS:	power = 20; break;
			case RBE_UN_POWER:	power = 15; break;
			case RBE_EAT_GOLD:	power =	 5; break;
			case RBE_EAT_ITEM:	power =	 5; break;
			case RBE_EAT_FOOD:	power =	 5; break;
			case RBE_EAT_LITE:	power =	 5; break;
			case RBE_ACID:		power = 50; break;
			case RBE_ELEC:		power = 50; break;
			case RBE_FIRE:		power = 50; break;
			case RBE_COLD:		power = 50; break;
			case RBE_BLIND:		power =	 2; break;
			case RBE_CONFUSE:		power = 10; break;
			case RBE_TERRIFY:		power = 10; break;
			case RBE_PARALYZE:	power =	 2; break;
			case RBE_LOSE_STR:	power =	 0; break;
			case RBE_LOSE_DEX:	power =	 0; break;
			case RBE_LOSE_CON:	power =	 0; break;
			case RBE_LOSE_INT:	power =	 0; break;
			case RBE_LOSE_WIS:	power =	 0; break;
			case RBE_LOSE_CHR:	power =	 0; break;
			case RBE_LOSE_ALL:	power =	 2; break;
			case RBE_SHATTER:		power = 60; break;
			case RBE_EXP_10:		power =	 5; break;
			case RBE_EXP_20:		power =	 5; break;
			case RBE_EXP_40:		power =	 5; break;
			case RBE_EXP_80:		power =	 5; break;
		}


		/* Monster hits player */
		if (!effect || check_hit(power, rlev, terrain_bonus, m_idx))
		{
			/* Always disturbing */
			disturb(1, 0);

			/* Hack -- Apply "protection from evil".  Somewhat modi-
			 * fied in Oangband. */
			if ((p_ptr->protevil > 0) &&
			    (r_ptr->flags3 & (RF3_EVIL)) &&
			    (3 * p_ptr->lev / 2 >= rlev) &&
			    ((rand_int(100 + p_ptr->lev - 5 * rlev / 4)) > 50))
			{
				/* Remember the Evil-ness */
				if (m_ptr->ml)
				{
					l_ptr->flags3 |= (RF3_EVIL);
				}

				/* Message */
				msg_format("%^s is repelled.", m_name);

				/* Hack -- Next attack */
				continue;
			}


			/* Assume no cut or stun */
			do_cut = do_stun = 0;

			/* Describe the attack method */
			switch (method)
			{
				case RBM_HIT:
				{
					act = "hits you.";
					do_cut = do_stun = 1;
					break;
				}

				case RBM_TOUCH:
				{
					act = "touches you.";
					break;
				}

				case RBM_PUNCH:
				{
					act = "punches you.";
					do_stun = 1;
					break;
				}

				case RBM_KICK:
				{
					act = "kicks you.";
					do_stun = 1;
					break;
				}

				case RBM_CLAW:
				{
					act = "claws you.";
					do_cut = 1;
					break;
				}

				case RBM_BITE:
				{
					act = "bites you.";
					do_cut = 1;
					break;
				}

				case RBM_STING:
				{
					act = "stings you.";
					break;
				}

				case RBM_XXX1:
				{
					act = "XXX1's you.";
					break;
				}

				case RBM_BUTT:
				{
					act = "butts you.";
					do_stun = 1;
					break;
				}

				case RBM_CRUSH:
				{
					act = "crushes you.";
					do_stun = 1;
					break;
				}

				case RBM_ENGULF:
				{
					act = "engulfs you.";
					break;
				}

				case RBM_XXX2:
				{
					act = "XXX2's you.";
					break;
				}

				case RBM_CRAWL:
				{
					act = "crawls on you.";
					break;
				}

				case RBM_DROOL:
				{
					act = "drools on you.";
					break;
				}

				case RBM_SPIT:
				{
					act = "spits on you.";
					break;
				}

				case RBM_XXX3:
				{
					act = "XXX3's on you.";
					break;
				}

				case RBM_GAZE:
				{
					act = "gazes at you.";
					break;
				}

				case RBM_WAIL:
				{
					act = "wails at you.";
					break;
				}

				case RBM_SPORE:
				{
					act = "releases spores at you.";
					break;
				}

				case RBM_XXX4:
				{
					act = "projects XXX4's at you.";
					break;
				}

				case RBM_BEG:
				{
					act = "begs you for money.";
					break;
				}

				case RBM_INSULT:
				{
					act = desc_insult[rand_int(8)];
					break;
				}

				case RBM_SNEER:
				{
					act = desc_sneer[rand_int(4)];
					break;
				}

				case RBM_REQUEST:
				{
					make_request(m_idx);
					break;
				}
			}

			/* Message */
			if (act) msg_format("%^s %s", m_name, act);

			/* The undead can give the player the Black Breath with a 
			 * sucessful blow. Uniques have a much better chance. -LM-
			 */
			if ((r_ptr->level >= 40) && (r_ptr->flags3 & (RF3_UNDEAD)) && 
				(r_ptr->flags1 & (RF1_UNIQUE)) && 
					(randint(250 - r_ptr->level) == 1))

			{
				msg_print("Your foe calls upon your soul!");
				msg_print("You feel the Black Breath slowly draining you of life...");
				p_ptr->black_breath = TRUE;
			}

			else if ((r_ptr->level >= 50) && (r_ptr->flags3 & (RF3_UNDEAD)) && 
				(randint(500 - r_ptr->level) == 1))
			{
				msg_print("Your foe calls upon your soul!");
				msg_print("You feel the Black Breath slowly draining you of life...");
				p_ptr->black_breath = TRUE;
			}

			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage.  Having resistances no longer reduces
			 * elemental attacks quite so much. 
			*/
			damage = damroll(d_dice, d_side);

			/* Apply appropriate damage */
			switch (effect)
			{
				case 0:
				{
					/* Hack -- Assume obvious */
					obvious = TRUE;

					/* Hack -- No damage */
					damage = 0;

					break;
				}

				case RBE_HURT:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Player armor reduces total damage */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					take_hit(damage, ddesc);

					break;
				}

				/* Poison is no longer fully cumulative. */
				case RBE_POISON:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Take "poison" effect */
					if (!(p_ptr->resist_pois && p_ptr->oppose_pois))
					{
						if (p_ptr->resist_pois || p_ptr->oppose_pois) damage /= 3;

						if (p_ptr->poisoned)
						{
							/* 1/3 to 2/3 damage. */
							if (set_poisoned(p_ptr->poisoned + 
								randint((damage + 2) / 3) + 
								(damage / 3)))
								obvious = TRUE;
						}
						else 
						{
							/* 1/2 to whole damage, plus 4. */
							if (set_poisoned(p_ptr->poisoned + 4 + 
								randint((damage + 1) / 2) + 
								(damage / 2)))
								obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_POIS);

					break;
				}

				case RBE_UN_BONUS:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Allow complete resist */
					if (!p_ptr->resist_disen)
					{
						/* Apply disenchantment */
						if (apply_disenchant(0)) obvious = TRUE;
					}

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_DISEN);

					break;
				}

				/* now drains rods too. */
				case RBE_UN_POWER:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Blindly hunt ten times for an item. */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item */
						i = rand_int(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &inventory[i];
						k_ptr = &k_info[o_ptr->k_idx];

						/* use "tmp" to decide if a item can 
						 * be uncharged.  By default, assume it 
						 * can't.
						 */
						tmp = 0;

						/* Skip non-objects */
						if (!o_ptr->k_idx) continue;

						/* Drain charged wands/staffs/rods */
						if ((o_ptr->tval == TV_STAFF) ||
							(o_ptr->tval == TV_WAND) ||
							(o_ptr->tval == TV_ROD))
						{
							/* case of charged wands/staffs. */
							if (((o_ptr->tval == TV_STAFF) ||
								(o_ptr->tval == TV_WAND)) &&
								(o_ptr->pval)) tmp = 1;

							/* case of (at least partially) charged rods. */
							if ((o_ptr->tval == TV_ROD) &&
								(o_ptr->timeout < o_ptr->pval)) tmp = 1;

							if (tmp)
							{
								/* Message */
								msg_print("Energy drains from your pack!");

								/* Obvious */
								obvious = TRUE;

								/* Heal; halved in 0.5.1 for mana gain
								 * Is later doubled if it slips to hps
								 */
								j = 2 + rlev / 20;

								/* Handle new-style wands correctly. */
								if (o_ptr->tval == TV_WAND)
								{
									j *= o_ptr->pval;
								}
								/* Handle new-style rods correctly. */
								else if (o_ptr->tval == TV_ROD)
								{
									j *= (o_ptr->pval - o_ptr->timeout) / 30;
								}
								else
								{
									j *= o_ptr->pval * 
										o_ptr->number;
								}

								/* Replenish monster mana */
								if (m_ptr->mana < r_ptr->mana)
								{
									if ( j > (r_ptr->mana - m_ptr->mana) * 10)
									{
										 j -= (r_ptr->mana - m_ptr->mana) * 10;
										 m_ptr->mana = r_ptr->mana;
									}
									else
									{
										 m_ptr->mana += (j/10) + 1;
										 j = 0;
									} 
								}
								
								/* Add hps with leftover */
								m_ptr->hp += j * 2;

								if (m_ptr->hp > m_ptr->maxhp) 
									m_ptr->hp = m_ptr->maxhp;

								/* Redraw (later) if needed */
								if (p_ptr->health_who == m_idx) 
									p_ptr->redraw |= (PR_HEALTH);


								/* Uncharge */
								if ((o_ptr->tval == TV_STAFF) || 
									(o_ptr->tval == TV_WAND)) 
									o_ptr->pval = 0;

								/* New-style rods. */
								if (o_ptr->tval == TV_ROD) 
									o_ptr->timeout = o_ptr->pval;


								/* Combine / Reorder the pack */
								p_ptr->notice |= 
									(PN_COMBINE | PN_REORDER);

								/* Window stuff */
								p_ptr->window |= (PW_INVEN);

								/* not more than one inventory 
								 * slot effected. */
								break;
							}
						}
					}

					break;
				}

				case RBE_EAT_GOLD:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Confused monsters cannot steal successfully.*/
					if (m_ptr->confused) break;

					/* Obvious */
					obvious = TRUE;

					/* Saving throw (unless paralyzed) based on dex and 
					 * relationship between yours and monster's level.
					 */
					if (!p_ptr->paralyzed && (rand_int(100) < 
						(adj_dex_safe[p_ptr->stat_ind[A_DEX]] + 
						p_ptr->lev - (r_ptr->level / 2))))
					{
						/* Saving throw message */
						msg_print("You quickly protect your money pouch!");

						/* Occasional blink anyway */
						if (rand_int(3)) blinked = TRUE;
					}

					/* Eat gold */
					else
					{
						gold = (p_ptr->au / 12) + randint(25);
						if (gold < 2) gold = 2;
						if (gold > 5000) gold = (p_ptr->au / 20) + randint(2000);
						if (gold > p_ptr->au) gold = p_ptr->au;
						p_ptr->au -= gold;
						if (gold <= 0)
						{
							msg_print("Nothing was stolen.");
						}
						else if (p_ptr->au)
						{
							msg_print("Your purse feels lighter.");
							msg_format("%ld coins were stolen!", (long)gold);
						}
						else
						{
							msg_print("Your purse feels lighter.");
							msg_print("All of your coins were stolen!");
						}

						/* Redraw gold */
						p_ptr->redraw |= (PR_GOLD);

						/* Window stuff */
						p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

						/* Blink away */
						blinked = TRUE;
					}

					break;
				}

				case RBE_EAT_ITEM:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Confused monsters cannot steal successfully.*/
					if (m_ptr->confused) break;

					/* Saving throw (unless paralyzed) based on dex and 
					 * relationship between yours and monster's level.
					 */
					if (!p_ptr->paralyzed && (rand_int(100) < 
						(adj_dex_safe[p_ptr->stat_ind[A_DEX]] + 
						p_ptr->lev - (r_ptr->level / 2))))
					{
						/* Saving throw message */
						msg_print("You grab hold of your backpack!");

						/* Occasional "blink" anyway */
						blinked = TRUE;

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					/* Blindly scrabble in the backpack ten times */
					for (k = 0; k < 10; k++)
					{
						object_type *i_ptr;
						object_type object_type_body;

						/* Pick an item */
						i = rand_int(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &inventory[i];

						/* Skip non-objects */
						if (!o_ptr->k_idx) continue;

						/* Skip artifacts */
						if (artifact_p(o_ptr)) continue;

						/* Get a description */
						object_desc(o_name, o_ptr, FALSE, 3);

						/* Message */
						msg_format("%sour %s (%c) was stolen!",
							   ((o_ptr->number > 1) ? "One of y" : "Y"),
							   o_name, index_to_label(i));

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain local object */
						object_copy(i_ptr, o_ptr);

						/* One item is stolen at a time. */
						i_ptr->number = 1;

						/* Hack -- If a rod or wand, allocate total 
						 * maximum timeouts or charges between those 
						 * stolen and those missed. -LM-
						 */
						if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
						{
							k_ptr = &k_info[o_ptr->k_idx];
							i_ptr->pval = o_ptr->pval / o_ptr->number;
							o_ptr->pval -= i_ptr->pval;
						}

						/* Carry the object */
						(void)monster_carry(m_idx, i_ptr);

						/* Steal the items */
						inven_item_increase(i, -1);
						inven_item_optimize(i);

						/* Obvious */
						obvious = TRUE;

						/* Blink away */
						blinked = TRUE;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_FOOD:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Steal some food */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item from the pack */
						i = rand_int(INVEN_PACK);

						/* Get the item */
						o_ptr = &inventory[i];

						/* Skip non-objects */
						if (!o_ptr->k_idx) continue;

						/* Skip non-food objects */
						if (o_ptr->tval != TV_FOOD) continue;

						/* Get a description */
						object_desc(o_name, o_ptr, FALSE, 0);

						/* Message */
						msg_format("%sour %s (%c) was eaten!",
							   ((o_ptr->number > 1) ? "One of y" : "Y"),
							   o_name, index_to_label(i));

						/* Steal the items */
						inven_item_increase(i, -1);
						inven_item_optimize(i);

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_LITE:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Access the lite */
					o_ptr = &inventory[INVEN_LITE];

					/* Drain fuel */
					if ((o_ptr->pval > 0) && (!artifact_p(o_ptr)))
					{
						/* Reduce fuel */
						o_ptr->pval -= (250 + randint(250));
						if (o_ptr->pval < 1) o_ptr->pval = 1;

						/* Notice */
						if (!p_ptr->blind)
						{
							msg_print("Your light dims.");
							obvious = TRUE;
						}

						/* Window stuff */
						p_ptr->window |= (PW_EQUIP);
					}

					break;
				}

				case RBE_ACID:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print("You are covered in acid!");

					/* Some guaranteed damage. */
					take_hit(damage / 3 + 1, ddesc);

					/* Special damage, reduced greatly by resists. */
					acid_dam(2 * damage / 3, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_ACID);

					break;
				}

				case RBE_ELEC:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print("You are struck by electricity!");

					/* Some guaranteed damage. */
					take_hit(damage / 3 + 1, ddesc);

					/* Special damage, reduced greatly by resists. */
					elec_dam(2 * damage / 3, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_ELEC);

					break;
				}

				case RBE_FIRE:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print("You are enveloped in flames!");

					/* Some guaranteed damage. */
					take_hit(damage / 3 + 1, ddesc);

					/* Special damage, reduced greatly by resists. */
					fire_dam(2 * damage / 3, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_FIRE);

					break;
				}

				case RBE_COLD:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print("You are covered with frost!");

					/* Some guaranteed damage. */
					take_hit(damage / 3 + 1, ddesc);

					/* Special damage, reduced greatly by resists. */
					cold_dam(2 * damage / 3, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_COLD);

					break;
				}

				/* Blindness is no longer fully cumulative. */
				case RBE_BLIND:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "blind" */
					if (!p_ptr->resist_blind)
					{
						if (p_ptr->blind)
						{
							if (set_blind(p_ptr->blind + 6 + randint(rlev / 2)))
							{
								obvious = TRUE;
							}
						}
						else
						{
							if (set_blind(12 + randint(rlev)))
							{
								obvious = TRUE;
							}
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_BLIND);

					break;
				}

				/* Confusion is no longer fully cumulative. */
				case RBE_CONFUSE:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "confused" */
					if (!p_ptr->resist_confu)
					{
						if (p_ptr->confused)
						{
							if (set_confused(p_ptr->confused + 2 + randint(rlev / 2)))
							{
								obvious = TRUE;
							}
						}
						else
						{
							if (set_confused(5 + randint(rlev)))
							{
								obvious = TRUE;
							}
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_CONFU);

					break;
				}

				/* Fear is no longer fully cumulative. */
				case RBE_TERRIFY:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "afraid" */
					if (p_ptr->resist_fear)
					{
						msg_print("You stand your ground!");
						obvious = TRUE;
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You stand your ground!");
						obvious = TRUE;
					}
					else
					{
						if (p_ptr->afraid)
						{
							if (set_afraid(p_ptr->afraid + 2 + randint(rlev / 2)))
							{
								obvious = TRUE;
							}
						}
						else
						{
							if (set_afraid(6 + randint(rlev)))
							{
								obvious = TRUE;
							}
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_FEAR_SAVE);

					break;
				}

				/* Paralyzation is no longer fully cumulative. */
				case RBE_PARALYZE:
				{
					/* Hack -- Prevent perma-paralysis via damage */
					if (p_ptr->paralyzed && (damage < 1)) damage = 1;

					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "paralyzed" */
					if (p_ptr->free_act)
					{
						msg_print("You are unaffected!");
						obvious = TRUE;
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You resist the effects!");
						obvious = TRUE;
					}
					else
					{
						if (p_ptr->paralyzed)
						{
							if (set_paralyzed(p_ptr->paralyzed + 2 + randint(rlev / 6)))
							{
								obvious = TRUE;
							}
						}
						else
						{
							if (set_paralyzed(4 + randint(rlev / 2)))
							{
								obvious = TRUE;
							}
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, LRN_FREE_SAVE);

					break;
				}

				case RBE_LOSE_STR:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_STR)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_INT:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_INT)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_WIS:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_WIS)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_DEX:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_DEX)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CON:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_CON)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CHR:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_CHR)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_ALL:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stats) */
					if (do_dec_stat(A_STR)) obvious = TRUE;
					if (do_dec_stat(A_DEX)) obvious = TRUE;
					if (do_dec_stat(A_CON)) obvious = TRUE;
					if (do_dec_stat(A_INT)) obvious = TRUE;
					if (do_dec_stat(A_WIS)) obvious = TRUE;
					if (do_dec_stat(A_CHR)) obvious = TRUE;

					break;
				}

				case RBE_SHATTER:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Reduce damage based on the player armor class */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					take_hit(damage, ddesc);

					/* Radius 6 earthquake centered at the monster */
					if (damage > 23) earthquake(m_ptr->fy, m_ptr->fx, 6, FALSE);

					break;
				}

				case RBE_EXP_10:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 95))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(10, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d/10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}
					break;
				}

				case RBE_EXP_20:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 90))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(20, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d/10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}
					break;
				}

				case RBE_EXP_40:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 75))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(40, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d/10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}
					break;
				}

				case RBE_EXP_80:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(80, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d/10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}
					break;
				}
			}


			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun)
			{
				/* Cancel cut */
				if (rand_int(100) < 50)
				{
					do_cut = 0;
				}

				/* Cancel stun */
				else
				{
					do_stun = 0;
				}
			}

			/* Handle cut */
			if (do_cut)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint(5); break;
					case 2: k = randint(5) + 5; break;
					case 3: k = randint(20) + 20; break;
					case 4: k = randint(50) + 50; break;
					case 5: k = randint(100) + 100; break;
					case 6: k = 300; break;
					default: k = 500; break;
				}

				/* Apply the cut */
				if (k) (void)set_cut(p_ptr->cut + k);
			}

			/* Handle stun.  Reduced in Oangband */
			if (do_stun)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint(5); break;
					case 2: k = randint(8) + 8; break;
					case 3: k = randint(15) + 15; break;
					case 4: k = randint(25) + 25; break;
					case 5: k = randint(35) + 35; break;
					case 6: k = 60; break;
					default: k = 100; break;
				}

				/* Apply the stun */
				if (k) (void)set_stun(p_ptr->stun + k);
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
				case RBM_HIT:
				case RBM_TOUCH:
				case RBM_PUNCH:
				case RBM_KICK:
				case RBM_CLAW:
				case RBM_BITE:
				case RBM_STING:
				case RBM_XXX1:
				case RBM_BUTT:
				case RBM_CRUSH:
				case RBM_ENGULF:
				case RBM_XXX2:

				/* Visible monsters */
				if (m_ptr->ml)
				{
					/* Disturbing */
					disturb(1, 0);

					/* Message */
					msg_format("%^s misses you.", m_name);
				}

				break;
			}
		}


		/* Analyze "visible" monsters only */
		if (visible)
		{
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (l_ptr->blows[ap_cnt] > 10))
			{
				/* Count attacks of this type */
				if (l_ptr->blows[ap_cnt] < MAX_UCHAR)
				{
					l_ptr->blows[ap_cnt]++;
				}
			}
		}
	}


	/* Blink away */
	if (blinked)
	{
		msg_print("There is a puff of smoke!");
		teleport_away(m_idx, MAX_SIGHT * 2 + 5);
	}


	/* Always notice cause of death */
	if (p_ptr->is_dead && (l_ptr->deaths < MAX_SHORT))
	{
		l_ptr->deaths++;
	}

	/* Assume we attacked */
	return (TRUE);
}





/*********************************************************************/
/*                                                                   */
/*                      Monster Ranged Attacks                       */
/*                                                                   */
/*********************************************************************/


/*
 * Using an input value for average damage, and another that 
 * controls variability, return the actual base damage of a 
 * monster's attack spell.  The larger the value for "dice", the 
 * less likely the damage will vary greatly.
 *
 * Do not return a value greater than 3/2rds or less than half the 
 * average, or that differs from the average by more than 150.
 */
static int get_dam(int av_dam, int dice)
{
	int dam;

	/* Handle extreme values. */
	if (av_dam < 2) return (av_dam == 1 ? 1 : 0);
	if (dice < 2) dice = 2;

	/* Calculate actual damage. */
	if ((av_dam < 6) || (av_dam < dice)) dam = damroll(1, av_dam * 2 - 1);
	else if (av_dam < 12) dam = damroll(2, av_dam - 1);
	else dam = damroll(dice, (av_dam * 2 / dice) - 1) + 
		rand_int(((av_dam * 2) % dice) + 1);

	/* Boundary control (to reduce instadeaths). */
	if (av_dam > 9)
	{
		/* Control proportions. */
		if (dam > 3 * av_dam / 2) dam = 3 * av_dam / 2;
		if (dam < av_dam / 2) dam = av_dam / 2;

		/* Control actual damage. */
		if (dam > av_dam + 150) dam = av_dam + 150;
		if (dam < av_dam - 150) dam = av_dam - 150;
	}

	/* Return the calculated damage. */
	return (dam);
}


/*
 * Cast a bolt at the player
 * Stop if we hit a monster
 * Affect monsters and the player
 */
static void mon_bolt(int m_idx, int typ, int dam)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_PLAY;

	/* Target the player with a bolt attack */
	(void)project(m_idx, 0, py, px, dam, typ, flg, 0, 0);
}

/*
 * Cast a beam at the player, sometimes with limited range.
 * Do not stop if we hit a monster
 * Affect grids, monsters, and the player
 */
static void mon_beam(int m_idx, int typ, int dam, int range)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_PLAY;

	/* Use a arc of degree 0 if range is limited. */
	if (range < MAX_SIGHT) 
	{
		flg |= PROJECT_ARC;

		/* Paranoia */
		if (range > 25) range = 25;

		/* Target the player with a limited-range beam. */
		(void)project(m_idx, range, py, px, dam, typ, flg, 0, 
			      (byte)(range * 10));
	}

	/* Otherwise, use a standard beam. */
	else 
	{
		flg |= PROJECT_BEAM;

		/* Target the player with a standard beam attack. */
		(void)project(m_idx, 0, py, px, dam, typ, flg, 0, 0);

	}
}

/*
 * Cast a ball spell at the player
 * Pass over any monsters that may be in the way
 * Affect grids, objects, monsters, and (specifically) the player
 */
static void mon_ball(int m_idx, int typ, int dam, int rad)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_PLAY;

	/* Target the player with a ball attack */
	(void)project(m_idx, rad, py, px, dam, typ, flg, 0, 0);
}

/*
 * Breathe or cast an arc-shaped spell at the player.
 * Use an arc spell of specified range and width.
 * Optionally, do not harm monsters with the same r_idx.
 * Affect grids, objects, monsters, and (specifically) the player
 */
static void mon_arc(int m_idx, int typ, bool noharm, int dam, int rad, int degrees_of_arc)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int py = p_ptr->py;
	int px = p_ptr->px;

	/* Diameter of source of energy is normally, but not always, 20. */
	int diameter_of_source = 20;

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_ARC | PROJECT_PLAY;

	/* Can optionally ignore monsters with the same r_idx. */
	if (noharm) flg |= PROJECT_SAFE;

	/* Radius of zero means no fixed limit. */
	if (rad == 0) rad = MAX_SIGHT;

	/* Calculate the effective diameter of the energy source, if necessary. */
	if (degrees_of_arc < 60)
	{
		if (degrees_of_arc == 0) diameter_of_source = rad * 10;
		else diameter_of_source = diameter_of_source * 60 / degrees_of_arc;
	}
	/* XXX XXX -- POWERFUL monster breaths lose less damage with range. */
	if (r_ptr->flags2 & (RF2_POWERFUL)) diameter_of_source *= 2;

	/* Max */
	if (diameter_of_source > 250) diameter_of_source = 250;

	/* Target the player with an arc-shaped attack. */
	(void)project(m_idx, rad, py, px, dam, typ, flg, degrees_of_arc, 
		(byte)diameter_of_source);
}


/*
 * Monster attempts to make a ranged (non-melee) attack.
 *
 * Determine if monster can attack at range, then see if it will.  Use 
 * the helper function "choose_attack_spell()" to pick a physical ranged 
 * attack, magic spell, or summon.  Execute the attack chosen.  Process 
 * its effects, and update character knowledge of the monster.
 *
 * Perhaps monsters should breathe at locations *near* the player,
 * since this would allow them to inflict "partial" damage.
 */
bool make_attack_ranged(monster_type *m_ptr, int attack)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int k, rlev, spower, rad, manacost;

	int m_idx = cave_m_idx[m_ptr->fy][m_ptr->fx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	char m_name[80];
	char m_poss[80];

	char ddesc[80];

	/* Target player */
	int x = px;
	int y = py;

	/* Summon count */
	int count = 0;

	/* Is the player blind? */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	/* Can the player see the monster casting the spell? */
	bool seen = (!blind && m_ptr->ml);


	/* Hack - Spend mana */
	if (attack >= 224) return (FALSE);
	else if (attack >= 192) manacost = mana_cost_RF7[attack-192];
	else if (attack >= 160) manacost = mana_cost_RF6[attack-160];
	else if (attack >= 128) manacost = mana_cost_RF5[attack-128];
	else if (attack >= 96)  manacost = mana_cost_RF4[attack-96];
	else return (FALSE);
	m_ptr->mana -= manacost;

	/*** Get some info. ***/

	/* Extract the monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* Extract the monster's spell power.  Must be at least 1. */
	spower = ((r_ptr->spell_power > 1) ? r_ptr->spell_power : 1);

	/* Get the monster name (or "it") */
	monster_desc(m_name, m_ptr, 0x00);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, m_ptr, 0x22);

	/* Hack -- Get the "died from" name */
	monster_desc(ddesc, m_ptr, 0x88);

	/*** Execute the ranged attack chosen. ***/
	switch (attack)
	{
		/* RF4_SHRIEK */
		case 96+0:
		{
			disturb(1, 0);
			if (r_ptr->flags2 & (RF2_SMART))
				msg_format("%^s shouts for help.", m_name);
			else
				msg_format("%^s makes a high pitched shriek.", m_name);
			aggravate_monsters(m_idx, FALSE);
			break;
		}

		/* RF4_LASH  (used for whips, but also for spitting) */
		case 96+1:
		{
			/* Attack type and descriptions. */
			int typ;
			cptr desc;
			cptr add_of;


			/* Get damage and effect of first melee blow. */
			int effect = r_ptr->blow[0].effect;
			int damage = damroll(r_ptr->blow[0].d_dice, 
				r_ptr->blow[0].d_side);

			/* Add some more damage for other melee blows. */
			if (r_ptr->blow[1].d_dice) damage += 
				damroll(r_ptr->blow[1].d_dice, 
				r_ptr->blow[1].d_side) / 2;
			if (r_ptr->blow[2].d_dice) damage += 
				damroll(r_ptr->blow[2].d_dice, 
				r_ptr->blow[2].d_side) / 2;
			if (r_ptr->blow[3].d_dice) damage += 
				damroll(r_ptr->blow[3].d_dice, 
				r_ptr->blow[3].d_side) / 2;

			/* Stop if no damage possible */
			if (!damage) break;

			/* Determine projection type, using effect.
			 * GF_WHIP is used for pure damage with no 
			 * extras -- many attacks need this.
			 */
			switch (effect)
			{
				case RBE_HURT:
				{
					typ = GF_WHIP;
					desc = ""; 
					add_of = "";
					break;
				}
				case RBE_ACID:
				{
					/* Some of attack is pure damage, and so 
					 * resists should not be allowed to reduce 
					 * damage as much as they do normally.
					 * Damage will be reduced later.
					 */
					if (p_ptr->resist_acid) damage *= 2;
					if (p_ptr->oppose_acid) damage *= 2;
					typ = GF_ACID;
					desc = " acid";
					add_of = " of";
					break;
				}
				case RBE_ELEC:
				{
					if (p_ptr->resist_elec) damage *= 2;
					if (p_ptr->oppose_elec) damage *= 2;

					typ = GF_ELEC;
					desc = " lightning";
					add_of = " of";
					break;
				}
				case RBE_FIRE:
				{
					if (p_ptr->resist_fire) damage *= 2;
					if (p_ptr->oppose_fire) damage *= 2;

					typ = GF_FIRE;
					desc = " fire";
					add_of = " of";
					break;
				}
				case RBE_COLD:
				{
					if (p_ptr->resist_cold) damage *= 2;
					if (p_ptr->oppose_cold) damage *= 2;

					typ = GF_COLD;
					desc = " frost";
					add_of = " of";
					break;
				}
				case RBE_POISON:
				{
					if (p_ptr->resist_pois) damage *= 2;
					if (p_ptr->oppose_pois) damage *= 2;

					typ = GF_POIS;
					desc = " venom";
					add_of = " of";
					break;
				}
				case RBE_BLIND:
				{
					typ = GF_DARK;
					desc = " blackness";
					add_of = " of";
					break;
				}
				case RBE_CONFUSE:
				case RBE_PARALYZE:
				{
					typ = GF_DARK;
					desc = " confusion";
					add_of = " of";
					break;
				}

				case RBE_UN_BONUS:
				case RBE_UN_POWER:
				{
					typ = GF_DISENCHANT;
					desc = " unmagic";
					add_of = " of";
					break;
				}
				case RBE_LOSE_STR:
				case RBE_LOSE_DEX:
				case RBE_LOSE_CON:
				case RBE_LOSE_INT:
				case RBE_LOSE_WIS:
				case RBE_LOSE_CHR:
				case RBE_LOSE_ALL:
				{
					typ = GF_TIME;
					desc = " ruination";
					add_of = " of";
					break;
				}
				case RBE_EXP_10:
				case RBE_EXP_20:
				case RBE_EXP_40:
				case RBE_EXP_80:
				{
					typ = GF_NETHER;
					desc = " withering";
					add_of = " of";
					break;
				}
				default:
				{
					typ = GF_WHIP; 
					desc = ""; 
					add_of = "";
					break;
				}
			}
			/* XXX -- Animals spit.   Acid-users spit. */
			if ((r_ptr->flags3 & (RF3_ANIMAL)) || (typ == GF_ACID))
			{
				if (blind) msg_print("You hear a soft sound.");
				else msg_format("%s spits%s at you.", 
					m_name, desc);
			}
			/* All other creatures use a whip. */
			else
			{
				if (blind) msg_print("You hear a crack.");
				else msg_format("%s lashes at you with a whip%s%s.", 
					m_name, add_of, desc);
			}

			/* Crack the whip, or spit - range 3 */
			mon_beam(m_idx, typ, damage, 3);

			break;
		}

		/* RF4_BOULDER */
		case 96+2:
		{
			disturb(1, 0);
			if (blind) msg_print("You hear something grunt with exertion.");
			else msg_format("%^s hurls a boulder at you.", m_name);
			mon_bolt(m_idx, GF_ROCK, get_dam((spower * 10) / 3, 10));
			break;
		}

		/* RF4_SHOT */
		case 96+3:
		{
			disturb(1, 0);
			if (blind) msg_print("You hear something whirl towards you.");
			else if (spower < 5) msg_format("%^s slings a pebble at you.", m_name);
			else if (spower < 15) msg_format("%^s slings a leaden pellet at you.", m_name);
			else msg_format("%^s slings a seeker shot at you.", m_name);

			mon_bolt(m_idx, GF_SHOT, get_dam((spower * 10) / 3, 10));
			break;
		}

		/* RF4_ARROW */
		case 96+4:
		{
			disturb(1, 0);
			if (spower < 8)
			{
				if (blind) msg_print("You hear a soft twang.");
				else msg_format("%^s fires a small arrow.", m_name);
			}
			else if (spower < 15)
			{
				if (blind) msg_format("You hear a twang.");
				else msg_format("%^s fires an arrow.", m_name);
			}
			else
			{
				if (blind) msg_print("You hear a loud thwang.");
				else msg_format("%^s fires a seeker arrow.", m_name);
			}

			mon_bolt(m_idx, GF_ARROW, get_dam((spower * 10) / 3, 10));
			break;
		}

		/* RF4_BOLT */
		case 96+5:
		{
			disturb(1, 0);
			if (spower < 8)
			{
				if (blind) msg_print("You hear a soft twung.");
				else msg_format("%^s fires a little bolt.", m_name);
			}
			else if (spower < 15)
			{
				if (blind) msg_format("You hear a twung.");
				else msg_format("%^s fires a crossbow bolt.", m_name);
			}
			else
			{
				if (blind) msg_print("You hear a loud thwung.");
				else msg_format("%^s fires a seeker bolt.", m_name);
			}

			mon_bolt(m_idx, GF_ARROW, get_dam((spower * 10) / 3, 10));
			break;
		}

		/* RF4_MISSL */
		case 96+6:
		{
			disturb(1, 0);
			if (blind) msg_print("You hear something coming at you.");
			else msg_format("%^s fires a missile.", m_name);
			mon_bolt(m_idx, GF_MISSILE, get_dam(spower * 3, 10));
			break;
		}

		/* RF4_PMISSL */
		case 96+7:
		{
			disturb(1, 0);
			if (blind) msg_print("You hear a soft 'fftt' sound.");
			else if (r_ptr->flags2 & (RF2_MORGUL_MAGIC)) 
				msg_format("%^s hurls a black dart at you!", m_name);
			else msg_format("%^s whips a poisoned dart at you.", m_name);
			mon_bolt(m_idx, GF_PMISSILE, get_dam(spower * 3, 10));
			break;
		}


		/* RF4_BRTH_ACID */
		case 96+8:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes acid.", m_name);

			/* 
			 * Breaths are 40-degree arcs for POWERFUL monsters, 
			 * 20 degrees for others.
			 */
			mon_arc(m_idx, GF_ACID, TRUE, 
			       ((m_ptr->hp / 2) > 1600 ? 1600 : (m_ptr->hp / 2)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_ELEC */
		case 96+9:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes lightning.", m_name);
			mon_arc(m_idx, GF_ELEC, TRUE, 
			       ((m_ptr->hp / 2) > 1600 ? 1600 : (m_ptr->hp / 2)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_FIRE */
		case 96+10:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes fire.", m_name);
			mon_arc(m_idx, GF_FIRE, TRUE, 
			       ((m_ptr->hp / 2) > 1600 ? 1600 : (m_ptr->hp / 2)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_COLD */
		case 96+11:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes frost.", m_name);
			mon_arc(m_idx, GF_COLD, TRUE, 
			       ((m_ptr->hp / 2) > 1600 ? 1600 : (m_ptr->hp / 2)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_POIS */
		case 96+12:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes gas.", m_name);
			mon_arc(m_idx, GF_POIS, TRUE, 
			       ((m_ptr->hp / 3) > 500 ? 500 : (m_ptr->hp / 3)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 50 : 30));
			break;
		}

		/* RF4_BRTH_PLAS */
		case 96+13:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes plasma.", m_name);
			mon_arc(m_idx, GF_PLASMA, TRUE, 
			       ((m_ptr->hp / 2) > 1600 ? 1600 : (m_ptr->hp / 2)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_LITE */
		case 96+14:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes light.", m_name);
			mon_arc(m_idx, GF_LITE, TRUE, 
			       ((3 * m_ptr->hp / 10) > 500 ? 500 : (3 * m_ptr->hp / 10)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_DARK */
		case 96+15:
		{
			disturb(1, 0);

			if (!(r_ptr->flags2 & (RF2_MORGUL_MAGIC)))
			{
				if (blind) msg_format("%^s breathes.", m_name);
				msg_format("%^s breathes darkness.", m_name);
				mon_arc(m_idx, GF_DARK, TRUE, 
				       ((3 * m_ptr->hp / 10) > 500 ? 500 : (3 * m_ptr->hp / 10)), 
				       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			}
			else
			{
				if (blind) msg_format("%^s breathes.", m_name);
				msg_format("%^s breathes Night.", m_name);
				mon_arc(m_idx, GF_MORGUL_DARK, TRUE, 
				       ((3 * m_ptr->hp / 10) > 500 ? 500 : (3 * m_ptr->hp / 10)), 
				       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			}
			break;
		}

		/* RF4_BRTH_CONFU */
		case 96+16:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes confusion.", m_name);
			mon_arc(m_idx, GF_CONFUSION, TRUE, 
			       ((3 * m_ptr->hp / 10) > 500 ? 500 : (3 * m_ptr->hp / 10)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_SOUND */
		case 96+17:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes sound.", m_name);
			mon_arc(m_idx, GF_SOUND, TRUE, 
			       ((m_ptr->hp / 4) > 250 ? 250 : (m_ptr->hp / 4)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 50 : 30));
			break;
		}

		/* RF4_BRTH_SHARD */
		case 96+18:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes shards.", m_name);
			mon_arc(m_idx, GF_SHARD, TRUE, 
			       ((3 * m_ptr->hp / 10) > 400 ? 400 : (3 * m_ptr->hp / 10)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_INER */
		case 96+19:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes inertia.", m_name);
			mon_arc(m_idx, GF_INERTIA, TRUE, 
			       ((m_ptr->hp / 3) > 200 ? 200 : (m_ptr->hp / 3)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_GRAV */
		case 96+20:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes gravity.", m_name);
			mon_arc(m_idx, GF_GRAVITY, TRUE, 
			       ((m_ptr->hp / 3) > 200 ? 200 : (m_ptr->hp / 3)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_FORCE */
		case 96+21:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes force.", m_name);
			mon_arc(m_idx, GF_FORCE, TRUE, 
			       ((m_ptr->hp / 3) > 250 ? 250 : (m_ptr->hp / 3)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 60 : 60));
			break;
		}

		/* RF4_BRTH_NEXUS */
		case 96+22:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes nexus.", m_name);
			mon_arc(m_idx, GF_NEXUS, TRUE, 
			       ((m_ptr->hp / 3) > 300 ? 300 : (m_ptr->hp / 3)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_NETHR */
		case 96+23:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes nether.", m_name);
			mon_arc(m_idx, GF_NETHER, TRUE, 
			       ((3 * m_ptr->hp / 10) > 600 ? 600 : (3 * m_ptr->hp / 10)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_CHAOS */
		case 96+24:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes chaos.", m_name);
			mon_arc(m_idx, GF_CHAOS, TRUE, 
			       ((3 * m_ptr->hp / 10) > 600 ? 600 : (3 * m_ptr->hp / 10)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_DISE */
		case 96+25:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes disenchantment.", m_name);
			mon_arc(m_idx, GF_DISENCHANT, TRUE, 
			       ((3 * m_ptr->hp / 10) > 400 ? 400 : (3 * m_ptr->hp / 10)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_BRTH_TIME */
		case 96+26:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes time.", m_name);
			mon_arc(m_idx, GF_TIME, TRUE, 
			       ((m_ptr->hp / 3) > 200 ? 200 : (m_ptr->hp / 3)), 
			       0, (r_ptr->flags2 & (RF2_POWERFUL) ? 40 : 20));
			break;
		}

		/* RF4_XXX4 */
		case 96+27:
		{
			break;
		}

		/* RF4_XXX5 */
		case 96+28:
		{
			break;
		}

		/* RF4_XXX6 */
		case 96+29:
		{
			break;
		}

		/* RF4_XXX7 */
		case 96+30:
		{
			break;
		}

		/* RF4_XXX8 */
		case 96+31:
		{
			break;
		}

		/* RF5_BALL_ACID */
		case 128+0:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts an acid ball.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts an acid ball.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a storm of acid.", m_name);
				if (spower < 120) rad = 3;
				else rad = 4;
			}
			mon_ball(m_idx, GF_ACID, get_dam(5 * spower, 6), rad);
			break;
		}

		/* RF5_BALL_ELEC */
		case 128+1:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a ball of electricity.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a ball of electricity.", m_name);
				rad = 2;
			}

			/* Electricity is the most variable of all attacks at high level. */
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);

				if (rand_int(3) != 0)
				{
					msg_format("%^s invokes a storm of electricity.", m_name);
					if (spower < 120) rad = 3;
					else rad = 4;
					spower = 3 * spower / 4;
				}
				else
				{
					msg_format("%^s calls a massive stroke of lightning down upon you!", m_name);
					rad = 0;
					spower = 3 * spower / 2;
				}
			}
			mon_ball(m_idx, GF_ELEC, get_dam(5 * spower, 6), rad);
			break;
		}

		/* RF5_BALL_FIRE */
		case 128+2:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a ball of %sfire.", m_name, 
					(r_ptr->flags2 & (RF2_UDUN_MAGIC) ? "hell" : ""));
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a ball of %sfire.", m_name, 
					(r_ptr->flags2 & (RF2_UDUN_MAGIC) ? "hell" : ""));
				rad = 2;
			}
			else if (spower < 110)
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else if (r_ptr->flags2 & (RF2_UDUN_MAGIC)) 
					msg_format("%^s invokes a storm of Udun-fire.", m_name);
				else msg_format("%^s invokes a firestorm.", m_name);
				rad = 3;
			}
			else
			{
				if (blind) msg_format("%^s intones in rising wrath.", m_name);
				else if (r_ptr->flags2 & (RF2_UDUN_MAGIC)) 
					msg_format("%^s calls upon the fires of Udun!", m_name);

				else msg_format("%^s conjures up a maelstrom of fire!", m_name);
				rad = 4;
			}
			if (r_ptr->flags2 & (RF2_UDUN_MAGIC)) mon_ball(m_idx, 
				GF_HELLFIRE, get_dam(spower * 6, 6), rad);
			else mon_ball(m_idx, GF_FIRE, get_dam(5 * spower, 6), rad);
			break;
		}

		/* RF5_BALL_COLD */
		case 128+3:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a frost ball.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a frost ball.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a storm of frost.", m_name);
				if (spower < 120) rad = 3;
				else rad = 4;
			}
			mon_ball(m_idx, GF_COLD, get_dam(5 * spower, 6), rad);
			break;
		}

		/* RF5_BALL_POIS */
		case 128+4:
		{
			disturb(1, 0);
			if (spower < 15)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a stinking cloud.", m_name);
				rad = 2;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a venomous cloud.", m_name);
				rad = 3;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a storm of poison.", m_name);
				if (spower < 120) rad = 4;
				else rad = 5;
			}
			mon_ball(m_idx, GF_POIS, get_dam(3 * spower, 8), rad);
			break;
		}

		/* RF5_BALL_LITE */
		case 128+5:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a ball of light.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s invokes a starburst.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a starburst.", m_name);
				rad = 3;
			}
			mon_ball(m_idx, GF_LITE, get_dam(3 * spower, 12), rad);
			break;
		}

		/* RF5_BALL_DARK */
		case 128+6:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else if (r_ptr->flags2 & (RF2_MORGUL_MAGIC))
					msg_format("%^s casts a ball of Night.", m_name);
				else msg_format("%^s casts a ball of darkness.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else if (r_ptr->flags2 & (RF2_MORGUL_MAGIC))
					msg_format("%^s casts a ball of Night.", m_name);
				else msg_format("%^s casts a ball of darkness.", m_name);
				rad = 2;
			}
			else 
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else if (r_ptr->flags2 & (RF2_MORGUL_MAGIC))
					msg_format("%^s invokes a storm of Night.", m_name);
				else msg_format("%^s invokes a darkness storm.", m_name);
				if (spower < 110) rad = 3;
				else rad = 4;
			}
			if (r_ptr->flags2 & (RF2_MORGUL_MAGIC))
				mon_ball(m_idx, GF_MORGUL_DARK, 
				   get_dam(3 * spower, 12), rad);
			else
				mon_ball(m_idx, GF_DARK, 
				   get_dam(3 * spower, 12), rad);
			break;
		}

		/* RF5_BALL_CONFU */
		case 128+7:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a ball of confusion.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a ball of confusion.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a storm of confusion.", m_name);
				rad = 3;
			}
			mon_ball(m_idx, GF_CONFUSION, get_dam(3 * spower, 12), rad);
			break;
		}

		/* RF5_BALL_SOUND */
		case 128+8:
		{
			disturb(1, 0);
			if (spower < 15)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s calls up a blast of sound.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s invokes a thunderclap.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s unleashes a cacophony of sound.", m_name);
				rad = 3;
			}
			mon_ball(m_idx, GF_SOUND, get_dam(2 * spower, 12), rad);
			break;
		}

		/* RF5_BALL_SHARD */
		case 128+9:
		{
			disturb(1, 0);
			if (spower < 15)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s calls up up a blast of shards.", m_name);
				rad = 1;
			}
			else if (spower < 90)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s calls up a whirlwind of shards.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a storm of knives!", m_name);
				rad = 3;
			}
			mon_ball(m_idx, GF_SHARD, get_dam(3 * spower, 12), rad);
			break;
		}

		/* RF5_BALL_STORM */
		case 128+10:
		{
			disturb(1, 0);

			if (spower < 30)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s gestures fluidly.", m_name);
				msg_print("You are surrounded by a little storm.");
				rad = 2;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s gestures fluidly.", m_name);
				msg_print("You are engulfed in a whirlpool.");
				rad = 3;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s gestures fluidly.", m_name);
				msg_print("You are lost in a raging tempest of wind and water!");
				rad = 5;
			}
			mon_ball(m_idx, GF_STORM, get_dam(4 * spower, 6), rad);
			break;
		}

		/* RF5_BALL_NETHR */
		case 128+11:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s whispers nastily.", m_name);
				else msg_format("%^s casts an orb of nether.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs a deadly word.", m_name);
				else msg_format("%^s casts a nether ball.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s intones with deadly menace.", m_name);
				else msg_format("%^s calls up a storm of nether magics.", m_name);
				rad = 3;
			}
			mon_ball(m_idx, GF_NETHER, get_dam(3 * spower, 12), rad);
			break;
		}

		/* RF5_BALL_CHAOS */
		case 128+12:
		{
			disturb(1, 0);
			if (spower < 20)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a sphere of chaos.", m_name);
				rad = 1;
			}
			else if (spower < 70)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a ball of raw chaos.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a storm of chaos.", m_name);
				rad = 3;
			}
			mon_ball(m_idx, GF_CHAOS, get_dam(3 * spower, 8), rad);
			break;
		}

		/* RF5_BALL_MANA */
		case 128+13:
		{
			disturb(1, 0);
			if (spower < 40)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a mana burst.", m_name);
				rad = 1;
			}
			else if (spower < 90)
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a mana ball.", m_name);
				rad = 2;
			}
			else
			{
				if (blind) msg_format("%^s chants powerfully.", m_name);
				else msg_format("%^s invokes a storm of mana.", m_name);
				rad = 3;
			}
			mon_ball(m_idx, GF_MANA, get_dam(2 * spower, 16), rad);

			break;
		}

		/* RF5_XXX2 */
		case 128+14:
		{
			break;
		}

		/* RF5_XXX3 */
		case 128+15:
		{
			break;
		}

		/* RF5_BOLT_ACID */
		case 128+16:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts an acid bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a bolt of acid.", m_name);
			}
			mon_bolt(m_idx, GF_ACID, get_dam(4 * spower, 6));
			break;
		}

		/* RF5_BOLT_ELEC */
		case 128+17:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a bolt of electricity.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a bolt of lightning.", m_name);
			}
			mon_bolt(m_idx, GF_ELEC, get_dam(4 * spower, 6));
			break;
		}

		/* RF5_BOLT_FIRE */
		case 128+18:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a fire bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s throws a fiery sphere at you.", m_name);
			}
			mon_bolt(m_idx, GF_FIRE, get_dam(4 * spower, 6));
			break;
		}

		/* RF5_BOLT_COLD */
		case 128+19:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a frost bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a frost bolt.", m_name);
			}
			mon_bolt(m_idx, GF_COLD, get_dam(4 * spower, 6));
			break;
		}

		/* RF5_BOLT_POIS */
		case 128+20:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a poison bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a bolt of venom.", m_name);
			}
			mon_bolt(m_idx, GF_POIS, get_dam(12 * spower / 5, 8));
			break;
		}

		/* RF5_BOLT_PLAS */
		case 128+21:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a plasma bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a bolt of plasma.", m_name);
			}
			mon_bolt(m_idx, GF_PLASMA, get_dam(5 * spower, 8));
			break;
		}

		/* RF5_BOLT_ICE */
		case 128+22:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts an ice bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a bolt of ice.", m_name);
			}
			mon_bolt(m_idx, GF_ICE, get_dam(5 * spower, 6));
			break;
		}

		/* RF5_BOLT_WATER */
		case 128+23:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a water bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a water bolt.", m_name);
			}
			mon_bolt(m_idx, GF_WATER, get_dam(2 * spower, 12));
			break;
		}

		/* RF5_BOLT_NETHR */
		case 128+24:
		{
			disturb(1, 0);
			if (spower < 80)
			{
				if (blind) msg_format("%^s whispers nastily.", m_name);
				else msg_format("%^s casts a nether bolt.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs a deadly word.", m_name);
				else msg_format("%^s hurls a black orb at you.", m_name);
			}
			mon_bolt(m_idx, GF_NETHER, get_dam(5 * spower / 2, 12));
			break;
		}

		/* RF5_BOLT_MANA */
		case 128+25:
		{
			disturb(1, 0);
			if ((spower < 5) || (spower <= rlev / 10))
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a magic missile.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts a mana bolt.", m_name);
			}
			mon_bolt(m_idx, GF_MANA, get_dam(2 * spower, 16));
			break;
		}

		/* RF5_BOLT_XXX1 */
		case 128+26:
		{
		}

		/* RF5_BEAM_ELEC */
		case 128+27:
		{
			disturb(1, 0);
			if (blind) msg_print("You feel a crackling in the air.");
			else msg_format("%^s shoots a spark of lightning at you.", m_name);

			mon_beam(m_idx, GF_ELEC, get_dam(5 * spower, 6), 6);
			break;
		}

		/* RF5_BEAM_ICE */
		case 128+28:
		{
			disturb(1, 0);
			if (spower < 50)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts an icy lance.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s murmurs deeply.", m_name);
				else msg_format("%^s casts an icy lance.", m_name);
			}
			mon_beam(m_idx, GF_ICE, get_dam(5 * spower, 6), 12);
			break;
		}

		/* RF5_BEAM_NETHR */
		case 128+29:
		{
			disturb(1, 0);
			if (spower < 30)
			{
				if (blind) msg_format("%^s whispers nastily.", m_name);
				else msg_format("%^s casts a beam of nether.", m_name);
			}
			else if (spower < 90)
			{
				if (blind) msg_format("%^s murmurs a deadly word.", m_name);
				else msg_format("%^s hurls a nether lance.", m_name);
			}
			else
			{
				if (blind) msg_format("%^s intones with deadly menace.", m_name);
				else msg_format("%^s unleashes a ray of death.", m_name);
			}
			mon_beam(m_idx, GF_NETHER, get_dam(3 * spower, 12), 10);
			break;
		}

		/* RF5_ARC__HFIR */
		case 128+30:
		{
			disturb(1, 0);
			/* Must be powerful and have Udun-magic to get an arc. */
			if ((spower > 50) && (r_ptr->flags2 & (RF2_UDUN_MAGIC)))
			{
				if (blind) msg_format("%^s speaks a word of peril.", m_name);
				else msg_format("%^s invokes a storm of hellfire!", m_name);

				/* Absolutely formidable close up, less so at range. */
				mon_arc(m_idx, GF_HELLFIRE, FALSE, get_dam(7 * spower, 8), 6, 60);
			}

			/* For weak Udun-magic casters, a column of hellfire. */
			else if (r_ptr->flags2 & (RF2_UDUN_MAGIC))
			{
				if (blind) msg_format("%^s murmurs darkly.", m_name);
				else msg_format("%^s gestures, and you are enveloped in hellfire.", m_name);
				mon_ball(m_idx, GF_HELLFIRE, get_dam(5 * spower, 8), 0);
			}

			/* Column of fire for everyone else. */
			else
			{
				if (blind) msg_format("%^s mutters.", m_name);
				else msg_format("%^s gestures, and you are enveloped in fire.", m_name);
				mon_ball(m_idx, GF_FIRE, get_dam(5 * spower, 8), 0);

			}
			break;
		}

		/* RF5_ARC__WALL */
		case 128+31:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mutters.", m_name);
			else msg_format("%^s calls up a wall of force.", m_name);
			mon_arc(m_idx, GF_FORCE, FALSE, get_dam(3 * spower, 18), 8, 60);
			break;
		}

		/* RF6_HASTE */
		case 160+0:
		{
			disturb(1, 0);
			if (seen)
			{
				if (blind)	
				{
					msg_format("%^s mumbles.", m_name);
				}
				else
				{
					msg_format("%^s concentrates on %s body.", m_name, m_poss);
				}
			}

			/* Allow a quick speed increase if not already greatly hasted. */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				msg_format("%^s starts moving faster.", m_name);
				m_ptr->mspeed += 10;
			}

			/* Allow small speed increases to base+20 */
			else if (m_ptr->mspeed < r_ptr->speed + 20)
			{
				msg_format("%^s starts moving slightly faster.", m_name);
				m_ptr->mspeed += 2;
			}

			break;
		}

		/* RF6_ADD_MANA */
		case 160+1:
		{
			int j;
		      
			disturb(1, 0);

			if (seen)
			{
				if (blind)
				{
					msg_format("%^s mumbles.", m_name);
				}
				else
				{
					msg_format("%^s gathers %s power.", m_name, m_poss);
				}
			}
			/* Increase current mana */
			j = (spower/20) + 5;
			if (j > r_ptr->mana - m_ptr->mana) m_ptr->mana = r_ptr->mana;
			else m_ptr->mana += j;

			break;
		}

		/* RF6_HEAL */
		case 160+2:
		{
			int cost;
			int hp_gain = 0;

			disturb(1, 0);

			/* Message */
			if (seen)
			{
				if (blind)
				{
					msg_format("%^s mumbles.", m_name);
				}
				else
				{
					msg_format("%^s concentrates on %s wounds.", m_name, m_poss);
				}
			}

			/* Costs mana (and has a limit of spower * 5) */
			if (((m_ptr->maxhp - m_ptr->hp) <= spower * 5) &&
			    ((m_ptr->maxhp - m_ptr->hp) <= m_ptr->mana * 50))
			{
				hp_gain = m_ptr->maxhp - m_ptr->hp;
				cost = 1 + ((m_ptr->maxhp - m_ptr->hp) / 50);
			}
			else if (spower <= m_ptr->mana * 10)
			{
				hp_gain += spower * 5;
				cost = (spower + 9)/10;
			}
			else
			{
				hp_gain += m_ptr->mana * 50;
				cost = m_ptr->mana;
			}

			/* Black Breath slows healing */
			if (m_ptr->black_breath)
			{
				msg_format("%^s is hindered by the Black Breath!", m_name);
				hp_gain = 4 * hp_gain / 10;
			}

			/* Spend mana, gain hps */
			if (cost > m_ptr->mana) m_ptr->mana=0;
			else m_ptr->mana -= cost;
			m_ptr->hp += hp_gain;

			/* Fully healed */
			if (m_ptr->hp >= m_ptr->maxhp)
			{
				/* Fully healed */
				m_ptr->hp = m_ptr->maxhp;

				/* Message */
				if (seen)
				{
					msg_format("%^s looks very healthy!", m_name);
				}
				else
				{
					msg_format("%^s sounds very healthy!", m_name);
				}
			}

			/* Partially healed */
			else
			{
				/* Message */
				if (seen)
				{
					msg_format("%^s looks healthier.", m_name);
				}
				else
				{
					msg_format("%^s sounds healthier.", m_name);
				}
			}

			/* Redraw (later) if needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Cancel fear */
			if (m_ptr->monfear)
			{
				/* Cancel fear */
				m_ptr->monfear = 0;

				/* Message */
				msg_format("%^s recovers %s courage.", m_name, m_poss);
			}

			break;
		}

		/* RF6_CURE */
		case 160+3:
		{
			if (seen) msg_format("%^s concentrates on %s ailments.", m_name, m_poss);

			/*
			 * Cancel Black Breath sometimes.
			 * If we don't cure it there will be no further curing.
			 */
			if ((m_ptr->black_breath) && (rlev + 20 > rand_int(120)))
			{
				/* Cancel Black Breath */
				m_ptr->black_breath = 0;

				/* Message */
				if (seen) msg_format("The hold of the Black Breath on %s is broken.", m_name);
			}

			/* Cancel stunning */
			if ((m_ptr->stunned) && !(m_ptr->black_breath))
			{
				/* Cancel stunning */
				m_ptr->stunned = 0;

				/* Message */
				if (seen) msg_format("%^s is no longer stunned.", m_name);
			}

			/* Cancel fear */
			if ((m_ptr->monfear) && !(m_ptr->black_breath))
			{
				/* Cancel fear */
				m_ptr->monfear = 0;

				/* Message */
				if (seen) msg_format("%^s recovers %s courage.", m_name, m_poss);

			}

			/* Cancel (major) slowing */
			if ((m_ptr->mspeed < r_ptr->speed - 5) && !(m_ptr->black_breath))
			{
				/* Cancel slowing */
				m_ptr->mspeed = r_ptr->speed;

				/* Message */
				if (seen) msg_format("%^s is no longer slowed.", m_name);
			}

			/* Redraw (later) if needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			break;
		}

		/* RF6_BLINK */
		case 160+4:
		{
			disturb(1, 0);
			if (seen) msg_format("%^s blinks away.", m_name);
			teleport_away(m_idx, 10);
			/* 
			 * if it comes into view from around a corner (unlikely)
			 * give a message and learn about the casting
			 */
			if (!seen && m_ptr->ml)
			{
                                monster_desc(ddesc, m_ptr, 0x08);
				msg_format("%^s blinks.", ddesc);
				seen = TRUE;
			}
			break;
		}

		/* RF6_TPORT */
		case 160+5:
		{
			disturb(1, 0);
			if (seen) msg_format("%^s teleports away.", m_name);
			teleport_away(m_idx, MAX_SIGHT * 2 + 5);
			/* Hack - 'teleport stress' fatigues the monster */
			/* m_ptr->mana -= m_ptr->mana/3; */
			
			/* 
			 * if it comes into view from around a corner (VERY unlikely)
			 * give a message and learn about the casting
			 */
			if (!seen && m_ptr->ml)
			{
				monster_desc(ddesc, m_ptr, 0x08);
				msg_format("%^s teleports.", ddesc);
				seen = TRUE;
			}
			break;
		}

		/* RF6_XXX3 */
		case 160+6:
		{
			break;
		}

		/* RF6_TELE_SELF_TO */
		case 160+7:
		{
			/* Move monster near player (also updates "m_ptr->ml"). */
			teleport_towards(m_ptr->fy, m_ptr->fx, py, px);

			/* Monster is now visible, but wasn't before. */
			if ((!seen) && (m_ptr->ml))
			{
				/* Get the name (using "A"/"An") again. */
				monster_desc(ddesc, m_ptr, 0x08);

				/* Message */
				msg_format("%^s suddenly appears.", ddesc);
			}
			/* Monster was visible before, but isn't now. */
			else if ((seen) && (!m_ptr->ml))
			{
				/* Message */
				msg_format("%^s blinks away.", m_name);
			}
			/* Monster is visible both before and after. */
			else if ((seen) && (m_ptr->ml))
			{
				/* Message */
				msg_format("%^s blinks toward you.", m_name);
			}

			/* Have we seen them at any point?  If so, we will learn about the spell. */
			if (seen || m_ptr->ml) seen = TRUE;

			break;
		}
		/* RF6_TELE_TO */
		case 160+8:
		{
			disturb(1, 0);
			msg_format("%^s commands you to return.", m_name);
			if ((check_specialty(SP_TELEPORT_RESIST)) && (rand_int(100) < p_ptr->skill_sav))
			{
				msg_print("Teleport Resistance!");
			}
			else
			{
				teleport_player_to(m_ptr->fy, m_ptr->fx);
			}
			break;
		}

		/* RF6_TELE_AWAY */
		case 160+9:
		{
			disturb(1, 0);
			msg_format("%^s teleports you away.", m_name);
			if ((check_specialty(SP_TELEPORT_RESIST)) && (rand_int(100) < p_ptr->skill_sav))
			{
				msg_print("Teleport Resistance!");
			}
			else
			{
				teleport_player(100, TRUE);
			}
			break;
		}

		/* RF6_TELE_LEVEL */
		case 160+10:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles strangely.", m_name);
			else msg_format("%^s gestures at your feet.", m_name);
			if (check_specialty(SP_TELEPORT_RESIST))
			{
				msg_print("Teleport Resistance!");
			}
			else if (p_ptr->resist_nexus)
			{
				msg_print("You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
			}
			else
			{
				teleport_player_level();
			}
			break;
		}

		/* RF6_XXX5 */
		case 160+11:
		{
			break;
		}

		/* RF6_DARKNESS */
		case 160+12:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s gestures in shadow.", m_name);
			(void)unlite_area(0, 3);
			break;
		}

		/* RF6_TRAPS */
		case 160+13:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles, and then cackles evilly.", m_name);
			else msg_format("%^s casts a spell and cackles evilly.", m_name);
			(void)trap_creation();
			break;
		}

		/* RF6_FORGET */
		case 160+14:
		{
			disturb(1, 0);
			msg_format("%^s tries to blank your mind.", m_name);

			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
			}
				else if (lose_all_info())
			{
				msg_print("Your memories fade away.");
			}
			break;
		}

		/* RF6_DRAIN_MANA */
		case 160+15:
		{
			if (p_ptr->csp)
			{
				int r1;

				/* Disturb if legal */
				disturb(1, 0);

				/* Basic message */
				msg_format("%^s draws psychic energy from you!", m_name);

				/* Attack power */
				r1 = (randint(spower) / 20) + 1;

				/* Full drain */
				if (r1 >= p_ptr->csp)
				{
					r1 = p_ptr->csp;
					p_ptr->csp = 0;
					p_ptr->csp_frac = 0;
				}

				/* Partial drain */
				else
				{
					p_ptr->csp -= r1;
				}

				/* Redraw mana */
				p_ptr->redraw |= (PR_MANA);

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

				/* Replenish monster mana */
				if (m_ptr->mana < r_ptr->mana)
				{
					if ( r1 > r_ptr->mana - m_ptr->mana)
					{
						 r1 -= r_ptr->mana - m_ptr->mana;
						 m_ptr->mana = r_ptr->mana;
					}
					else
					{
						 m_ptr->mana += r1;
						 r1 = 0;
					} 
				}

				/* Heal the monster with remaining energy */
				if ((m_ptr->hp < m_ptr->maxhp) && (r1))
				{
					/* Heal */
					m_ptr->hp += (30 * (r1 + 1));
					if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

					/* Redraw (later) if needed */
					if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

					/* Special message */
					if (seen)
					{
						msg_format("%^s appears healthier.", m_name);
					}
				}
			}
			break;
		}

		/* RF4_DISPEL */
		case 160+16:
		{
			int r1 = 0;

			disturb(1, 0);

			if (!blind) msg_format("%^s dispels magic.", m_name);
			else if (spower < 40) msg_format("%^s mumbles.", m_name);
			else if (spower < 90) msg_format("%^s murmurs deeply.", m_name);
			else msg_format("%^s chants powerfully.", m_name);

			if (p_ptr->fast && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_fast(0);
				r1 += 2;
			}
			if (p_ptr->protevil && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_protevil(0);
				r1 += 2;
			}
			if (p_ptr->magicdef && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_extra_defences(0);
				r1 += 2;
			}
			if (p_ptr->hero && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_hero(0);
				r1 += 2;
			}
			if (p_ptr->shero && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_shero(0);
				r1 += 2;
			}
			if (p_ptr->shield && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_shield(0);
				r1 += 2;
			}
			if (p_ptr->blessed && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_blessed(0);
				r1 += 2;
			}
			if (p_ptr->tim_invis && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_tim_invis(0);
				r1 += 2;
			}
			if (p_ptr->tim_infra && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_tim_infra(0);
				r1 += 2;
			}
			if (p_ptr->tim_esp && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_tim_esp(0);
				r1 += 2;
			}
			if (p_ptr->superstealth && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_superstealth(0);
				r1 += 2;
			}
			if (p_ptr->ele_attack && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_ele_attack(0, 0);
				r1 += 2;
			}
			if (p_ptr->oppose_acid && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_oppose_acid(0);
				r1 += 2;
			}
			if (p_ptr->oppose_elec && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_oppose_elec(0);
				r1 += 2;
			}
			if (p_ptr->oppose_fire && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_oppose_fire(0);
				r1 += 2;
			}
			if (p_ptr->oppose_cold && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_oppose_cold(0);
				r1 += 2;
			}
			if (p_ptr->oppose_pois && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_oppose_pois(0);
				r1 += 2;
			}
			if (p_ptr->word_recall && (rand_int(spower) > p_ptr->skill_sav))
			{
				set_recall(0);
				r1 += 2;
			}
			if ((p_ptr->special_attack & (ATTACK_CONFUSE)) && (rand_int(spower) > p_ptr->skill_sav))
			{
				p_ptr->special_attack &= ~(ATTACK_CONFUSE);
				msg_print("Your hands stop glowing.");
				r1 += 2;
			}
			if ((p_ptr->special_attack & (ATTACK_BLKBRTH)) && (rand_int(spower) > p_ptr->skill_sav))
			{
				p_ptr->special_attack &= ~(ATTACK_BLKBRTH);
				msg_print("Your hands stop radiating Night.");
				r1 += 2;
			}
			if ((p_ptr->special_attack & (ATTACK_FLEE)) && (rand_int(spower) > p_ptr->skill_sav))
			{
				p_ptr->special_attack &= ~(ATTACK_FLEE);
				msg_print("You forget your escape plan.");
				r1 += 2;
			}
			if ((p_ptr->special_attack & (ATTACK_SUPERSHOT)) && (rand_int(spower) > p_ptr->skill_sav))
			{
				p_ptr->special_attack &= ~(ATTACK_SUPERSHOT);
				msg_print("Your ready crossbow bolt seems less dangerous.");
				r1 += 2;
			}
			if ((p_ptr->special_attack & (ATTACK_HOLY)) && (rand_int(spower) > p_ptr->skill_sav))
			{
				p_ptr->special_attack &= ~(ATTACK_HOLY);
				msg_print("Your Holy attack dissipates.");
				r1 += 2;
			}
			if (SCHANGE && (rand_int(spower) > p_ptr->skill_sav))
			{
				shapechange(SHAPE_NORMAL);
				r1 += 2;
			}

			/* Replenish monster mana */
			if ( r1 > r_ptr->mana - m_ptr->mana) m_ptr->mana = r_ptr->mana;
			else m_ptr->mana += r1;

			break;
		}

		/* RF6_XXX7 */
		case 160+17:
		{
			break;
		}

		/* RF6_MIND_BLAST */
		case 160+18:
		{
			disturb(1, 0);
			if (!seen)
			{
				msg_print("You feel something focusing on your mind.");
			}
			else
			{
				msg_format("%^s gazes deep into your eyes.", m_name);
			}

			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
			}
			else
			{
				msg_print("Your mind is blasted by psionic energy.");
				if (!p_ptr->resist_confu)
				{
					(void)set_confused(p_ptr->confused + rand_int(4) + 4);
				}
				take_hit(damroll(8, 8), ddesc);
			}
			break;
		}

		/* RF6_BRAIN_SMASH */
		case 160+19:
		{
			disturb(1, 0);
			if (!seen)
			{
				msg_print("You feel something focusing on your mind.");

			}
			else
			{
				msg_format("%^s looks deep into your eyes.", m_name);
			}
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
			}
			else
			{
				msg_print("Your mind is blasted by psionic energy.");
				take_hit(damroll(12, 15), ddesc);
				if (!p_ptr->resist_blind)
				{
					(void)set_blind(p_ptr->blind + 8 + rand_int(8));
				}
				if (!p_ptr->resist_confu)
				{
					(void)set_confused(p_ptr->confused + rand_int(4) + 4);
				}
				if (!p_ptr->free_act)
				{
					(void)set_paralyzed(p_ptr->paralyzed + rand_int(4) + 4);
				}
				(void)set_slow(p_ptr->slow + rand_int(4) + 4);
			}
			break;
		}

		/* RF6_WOUND */
		case 160+20:
		{
			disturb(1, 0);

			if (spower < 7)
			{
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s points at you and curses.", m_name);
				k = 1;
			}
			else if (spower < 15)
			{
				if (blind) msg_format("%^s mumbles deeply.", m_name);
				else msg_format("%^s points at you and curses horribly.", m_name);
				k = 2;
			}
			else if (spower < 30)
			{
				if (blind) msg_format("%^s murmurs loudly.", m_name);
				else msg_format("%^s points at you, incanting terribly.", m_name);
				k = 3;
			}
			else if (spower < 55)
			{
				if (blind) msg_format("%^s cries out wrathfully.", m_name);
				else msg_format("%^s points at you, screaming words of peril!", m_name);
				k = 4;
			}
			else
			{
				if (blind) msg_format("%^s screams the word 'DIE!'", m_name);
				else msg_format("%^s points at you, screaming the word DIE!", m_name);
				k = 5;
			}

			if (rand_int(rlev / 2 + 70) < p_ptr->skill_sav)
			{
				msg_format("You resist the effects%c", 
				      (spower < 30 ?  '.' : '!'));
			}
			else
			{
				/* Inflict damage. */
				if (spower < 75) take_hit(get_dam(spower * 4, 6), ddesc);
				else take_hit(get_dam(300, 6) + (spower - 75), ddesc);

				/* Cut the player depending on strength of spell. */
				if (k == 1) (void)set_cut(p_ptr->cut + 8 + damroll(2, 4));
				if (k == 2) (void)set_cut(p_ptr->cut + 23 + damroll(3, 8));
				if (k == 3) (void)set_cut(p_ptr->cut + 46 + damroll(4, 12));
				if (k == 4) (void)set_cut(p_ptr->cut + 95 + damroll(8, 15));
				if (k == 5) (void)set_cut(1200);
			}
			break;
		}

		/* RF6_XXX6 */
		case 160+21:
		{
			break;
		}

		/* RF6_XXX7 */
		case 160+22:
		{
			break;
		}

		/* RF6_XXX8 */
		case 160+23:
		{
			break;
		}

		/* RF6_XXX9 */
		case 160+24:
		{
			break;
		}

		/* RF6_HUNGER */
		case 160+25:
		{
			if (blind) msg_print("You suddenly feel hungry.");
			else msg_format("%^s gestures at you, and you suddenly feel hungry.", m_name);

			if (randint(100) > p_ptr->skill_sav)
			{
				/* Reduce food abruptly.  */
				(void)set_food(p_ptr->food - (p_ptr->food/3));
			}

			else msg_print ("You resist the effects!");

			break;
		}

		/* RF6_XX11 */
		case 160+26:
		{
			break;
		}

		/* RF6_SCARE */
		case 160+27:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles, and you hear scary noises.", m_name);
			else msg_format("%^s casts a fearful illusion.", m_name);
			if (p_ptr->resist_fear)
			{
				msg_print("You refuse to be frightened.");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You refuse to be frightened.");
			}
			else
			{
				(void)set_afraid(p_ptr->afraid + rand_int(3) + 3);
			}
			break;
		}

		/* RF6_BLIND */
		case 160+28:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a spell, burning your eyes!", m_name);
			if (p_ptr->resist_blind)
			{
				msg_print("You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
			}
			else
			{
				(void)set_blind(12 + rand_int(4));
			}
			break;
		}

		/* RF6_CONF */
		case 160+29:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles, and you hear puzzling noises.", m_name);
			else msg_format("%^s creates a mesmerising illusion.", m_name);
			if (p_ptr->resist_confu)
			{
				msg_print("You disbelieve the feeble spell.");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You disbelieve the feeble spell.");
			}
			else
			{
				(void)set_confused(p_ptr->confused + rand_int(4) + 4);
			}
			break;
		}

		/* RF6_SLOW */
		case 160+30:
		{
			disturb(1, 0);
			msg_format("%^s drains power from your muscles!", m_name);
			if (p_ptr->free_act)
			{
				msg_print("You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
			}
			else
			{
				(void)set_slow(p_ptr->slow + rand_int(6) + 6);
			}
			break;
		}	
	
		/* RF6_HOLD */
		case 160+31:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s stares deep into your eyes!", m_name);
			if (p_ptr->free_act)
			{
					msg_print("You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
					msg_format("You resist the effects!");
			}
				else
			{
				(void)set_paralyzed(p_ptr->paralyzed + rand_int(4) + 4);
			}
			break;
		}


		/* RF7_S_KIN */
		case 192 + 0:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons %s %s.", m_name, 
				m_poss, ((r_ptr->flags1) & RF1_UNIQUE ?
				"minions" : "kin"));

			/* Hack -- Set the letter of the monsters to summon */
			summon_kin_type = r_ptr->d_char;
			for (k = 0; k < (r_ptr->level > 40 ? 4 : 3); k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_KIN);
			}

			if (blind && count)
			{
				msg_print("You hear many things appear nearby.");
			}
			break;
		}

		/* RF7_XXX */
		case 192 + 1:  break;
		case 192 + 2:  break;


		/* RF7_S_MONSTER */
		case 192 + 3:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons help!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(y, x, FALSE, rlev, 0);
			}
			if (blind && count) msg_print("You hear something appear nearby.");
			break;
		}

		/* RF7_S_MONSTERS */
		case 192 + 4:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons monsters!", m_name);
			for (k = 0; k < 4; k++)
			{
				count += summon_specific(y, x, FALSE, rlev, 0);
			}
			if (blind && count) msg_print("You hear many things appear nearby.");
			break;
		}

		/* RF7_XXX */
		case 192 + 5:  break;
		case 192 + 6:  break;
		case 192 + 7:  break;

		/* RF7_S_ANT */
		case 192 + 8:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons ants.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_ANT);
			}
			if (blind && count) msg_print("You hear many things appear nearby.");
			break;
		}

		/* RF7_S_SPIDER */
		case 192 + 9:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons spiders.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_SPIDER);
			}
			if (blind && count) msg_print("You hear many things appear nearby.");
			break;
		}

		/* RF7_S_HOUND */
		case 192 + 10:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons hounds.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_HOUND);
			}
			if (blind && count) msg_print("You hear many things appear nearby.");
			break;
		}

		/* RF7_S_ANIMAL */
		case 192 + 11:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons natural creatures.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_ANIMAL);
			}
			if (blind && count) msg_print("You hear many things appear nearby.");
			break;
		}

		/* RF7_XXX */
		case 192 + 12:	break;
		case 192 + 13:	break;

		/* RF7_S_THIEF */
		case 192 + 14:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s whistles.", m_name);
			else msg_format("%^s whistles up a den of thieves!", m_name);
			for (k = 0; k < 4; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_THIEF);
			}
			if (blind && count) msg_print("You hear many sneaky things appear nearby.");
			break;
		}

		/* Summon Bert, Bill, and Tom */
		/* No messages unless sucessful */
		/* RF7_S_BERTBILLTOM */
		case 192 + 15:
		{
			for (k = 0; k < 2; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_BERTBILLTOM);
			}
			if (blind && count) 
				msg_print("You hear heavy footsteps approaching.");
			else if (count) 
				msg_format("%^s calls up his friends!", m_name);
			break;
		}

		/* RF7_XXX */
		case 192 + 16:	break;
		case 192 + 17:	break;
		case 192 + 18:	break;
		case 192 + 19:	break;

		/* RF7_S_DRAGON */
		case 192 + 20:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons a dragon!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_DRAGON);
			}
			if (blind && count) msg_print("You hear something appear nearby.");
			break;
		}

		/* RF7_S_HI_DRAGON */
		case 192 + 21:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons ancient dragons!", m_name);
			for (k = 0; k < 4; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_HI_DRAGON);
			}
			if (blind && count)
			{
				msg_print("You hear many powerful things appear nearby.");
			}
			break;
		}

		/* RF7_XXX */
		case 192 + 22:	break;
		case 192 + 23:	break;

		/* RF7_S_DEMON */
		case 192 + 24:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			{
				if (!(blind)) msg_format
					("%^s magically summons a hellish adversary!", m_name);
				for (k = 0; k < 1; k++)
				{
					count += summon_specific(y, x, FALSE, rlev, SUMMON_DEMON);
				}
				if (blind && count) msg_print
					("You hear something appear nearby.");
			}
			break;
		}

		/* RF7_S_HI_DEMON */
		case 192 + 25:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons greater demons!", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(y, x, FALSE, rlev, SUMMON_HI_DEMON);
			}
			if (blind && count)
			{
				msg_print("You hear many evil things appear nearby.");
			}
			break;
		}

		/* RF7_XXX */
		case 192 + 26:	break;
		case 192 + 27:	break;

		/* RF7_S_UNDEAD */
		case 192 + 28:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons an undead adversary!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_UNDEAD);
			}
			if (blind && count) msg_print("You hear something appear nearby.");
			break;
		}

		/* RF7_S_HI_UNDEAD */
		case 192 + 29:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons greater undead!", m_name);
			for (k = 0; k < 4; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_HI_UNDEAD);
			}
			if (blind && count)
			{
				msg_print("You hear many creepy things appear nearby.");
			}
			break;
		}

		/* Summon the Ringwraiths */
		/* RF7_S_WRAITH */
		case 192 + 30:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons mighty undead opponents!", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_WRAITH);
			}
			for (k = 0; (k < 6) && (count < 6); k++)
			{
				count += summon_specific(y, x, FALSE, 
							 rlev, SUMMON_HI_UNDEAD);
			}
			if (blind && count)
			{
				msg_print("You hear many creepy things appear nearby.");
			}
			break;
		}

		/* Summon Uniques */
		/* RF7_S_UNIQUE */
		case 192 + 31:
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons special opponents!", m_name);
			for (k = 0; k < 3; k++)
			{
				count += summon_specific(y, x, FALSE, 
					rlev, SUMMON_UNIQUE);
			}
			for (k = 0; (k < 6) && (count < 6); k++)
			{
				count += summon_specific(y, x, FALSE, 
							 rlev, SUMMON_HI_UNDEAD);
			}
			if (blind && count)
			{
				msg_print("You hear many powerful things appear nearby.");
			}
			break;
		}

		/* Paranoia */
		default:
		{
			msg_print("A monster tried to cast a spell that has not yet been defined.");
		}
	}

	/* Power Siphon Specialty Ability */
	if ((check_specialty(SP_POWER_SIPHON)) && (manacost) && m_ptr->ml)
	{
		p_ptr->mana_gain += manacost;
	}

	/* Learn Player Resists */
	if (attack < 128)
	{
		  update_smart_learn(m_idx, spell_desire_RF4[attack-96][D_RES]);
	}
	else if (attack < 160)
	{
		  update_smart_learn(m_idx, spell_desire_RF5[attack-128][D_RES]);
	}
	else if (attack < 192)
	{
		  update_smart_learn(m_idx, spell_desire_RF6[attack-160][D_RES]);
	}
	else if (attack < 224)
	{
		  update_smart_learn(m_idx, spell_desire_RF7[attack-192][D_RES]);
	}
	
	/* Mark minimum desired range for recalculation */
	m_ptr->min_range = 0;

	/* Remember what the monster did to us */
	if (seen)
	{
		/* Innate spell */
		if (attack < 32*4)
		{
		l_ptr->flags4 |= (1L << (attack - 32*3));
		if (l_ptr->cast_inate < MAX_UCHAR) l_ptr->cast_inate++;
		}

		/* Bolt or Ball */
		else if (attack < 32*5)
		{
		l_ptr->flags5 |= (1L << (attack - 32*4));
		if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
		}

		/* Special spell */
		else if (attack < 32*6)
		{
		l_ptr->flags6 |= (1L << (attack - 32*5));
		if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
		}

		/* Summon spell */
		else if (attack < 32*7)
		{
		l_ptr->flags7 |= (1L << (attack - 32*6));
		if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
		}

		/* Remember special flags */
		if (r_ptr->flags2 & (RF2_ARCHER)) l_ptr->flags2 |= RF2_ARCHER;
		if (r_ptr->flags2 & (RF2_MORGUL_MAGIC)) l_ptr->flags2 |= RF2_MORGUL_MAGIC;
		if (r_ptr->flags2 & (RF2_UDUN_MAGIC)) l_ptr->flags2 |= RF2_UDUN_MAGIC;

	}

	if (seen && p_ptr->wizard) 
		msg_format("%^s has %i mana remaining.", m_name, m_ptr->mana);

	/* Always take note of monsters that kill you */
	if (p_ptr->is_dead && (l_ptr->deaths < MAX_SHORT))
	{
		l_ptr->deaths++;
	}


	/* A spell was cast */
	return (TRUE);
}
