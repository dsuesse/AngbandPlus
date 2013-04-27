/* File: bldg.c */

/*
 * Purpose: Building commands
 * Created by Ken Wigle for Kangband - a variant of Angband 2.8.3
 * -KMW-
 *
 * Rewritten for Kangband 2.8.3i using Kamband's version of
 * bldg.c as written by Ivan Tkatchev
 *
 * Changed for ZAngband by Robert Ruehlmann
 * Rewritten yet again by Steven Fuerst to use the fields code.
 */

#include "angband.h"


/* Hack - force exit from building */
static bool force_build_exit = FALSE;


void have_nightmare(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];
	char m_name[80];
	bool happened = FALSE;
	int power = r_ptr->level + 10;
	cptr desc = r_name + r_ptr->name;


	if (!(r_ptr->flags1 & RF1_UNIQUE))
	{
		/* Describe it */
		strnfmt(m_name, 80, "%s %s", (is_a_vowel(desc[0]) ? "an" : "a"), desc);

		if (r_ptr->flags1 & RF1_FRIENDS)
		{
			power /= 2;
		}
	}
	else
	{
		/* Describe it */
		strnfmt(m_name, 80, "%s", desc);

		power *= 2;
	}

	if (saving_throw(p_ptr->skill_sav * 100 / power))
	{
		msgf("%^s chases you through your dreams.", m_name);

		/* Safe */
		return;
	}

	if (p_ptr->image)
	{
		/* Something silly happens... */
		msgf("You behold the %s visage of %s!",
				   funny_desc[randint0(MAX_SAN_FUNNY)], m_name);

		if (one_in_(3))
		{
			msgf(funny_comments[randint0(MAX_SAN_COMMENT)]);
			p_ptr->image = p_ptr->image + randint1(r_ptr->level);
		}

		/* Never mind; we can't see it clearly enough */
		return;
	}

	/* Something frightening happens... */
	msgf("You behold the %s visage of %s!",
			   horror_desc[randint0(MAX_SAN_HORROR)], desc);

	r_ptr->r_flags4 |= RF4_ELDRITCH_HORROR;

	switch (p_ptr->prace)
	{
		case RACE_IMP:
		{
			/* Imps may make a saving throw */
			if (saving_throw(20 + p_ptr->lev)) return;

			break;
		}

		case RACE_SKELETON:
		case RACE_ZOMBIE:
		case RACE_SPECTRE:
		case RACE_VAMPIRE:
		case RACE_GHOUL:
		{
			/* Undead may make a saving throw */
			if (saving_throw(10 + p_ptr->lev)) return;

			break;
		}
	}

	/* Mind blast */
	if (!saving_throw(p_ptr->skill_sav * 100 / power))
	{
		if (!p_ptr->resist_confu)
		{
			(void)set_confused(p_ptr->confused + rand_range(4, 8));
		}
		if (!p_ptr->resist_chaos && one_in_(3))
		{
			(void)set_image(p_ptr->image + rand_range(250, 400));
		}
		return;
	}

	/* Lose int & wis */
	if (!saving_throw(p_ptr->skill_sav * 100 / power))
	{
		(void)do_dec_stat(A_INT);
		(void)do_dec_stat(A_WIS);
		return;
	}

	/* Brain smash */
	if (!saving_throw(p_ptr->skill_sav * 100 / power))
	{
		if (!p_ptr->resist_confu)
		{
			(void)set_confused(p_ptr->confused + rand_range(4, 8));
		}
		if (!p_ptr->free_act)
		{
			(void)set_paralyzed(p_ptr->paralyzed + rand_range(4, 8));
		}
		while (!saving_throw(p_ptr->skill_sav))
		{
			(void)do_dec_stat(A_INT);
		}
		while (!saving_throw(p_ptr->skill_sav))
		{
			(void)do_dec_stat(A_WIS);
		}
		if (!p_ptr->resist_chaos)
		{
			(void)set_image(p_ptr->image + rand_range(250, 400));
		}
		return;
	}

	/* Permanent lose int & wis */
	if (!saving_throw(p_ptr->skill_sav * 100 / power))
	{
		if (dec_stat(A_INT, 10, TRUE)) happened = TRUE;
		if (dec_stat(A_WIS, 10, TRUE)) happened = TRUE;
		if (happened)
		{
			msgf("You feel much less sane than before.");
		}
		return;
	}

	/* Amnesia */
	if (!saving_throw(p_ptr->skill_sav * 100 / power))
	{
		if (lose_all_info())
		{
			msgf("You forget everything in your utmost terror!");
		}
		return;
	}

	/* Else gain permanent insanity */
	if ((p_ptr->muta3 & MUT3_MORONIC) && (p_ptr->muta2 & MUT2_BERS_RAGE) &&
		((p_ptr->muta2 & MUT2_COWARDICE) || (p_ptr->resist_fear)) &&
		((p_ptr->muta2 & MUT2_HALLU) || (p_ptr->resist_chaos)))
	{
		/* The poor bastard already has all possible insanities! */
		return;
	}

	while (!happened)
	{
		switch (randint1(4))
		{
			case 1:
			{
				if (!(p_ptr->muta3 & MUT3_MORONIC))
				{
					msgf("You turn into an utter moron!");
					if (p_ptr->muta3 & MUT3_HYPER_INT)
					{
						msgf("Your brain is no longer a living computer.");
						p_ptr->muta3 &= ~(MUT3_HYPER_INT);
					}
					p_ptr->muta3 |= MUT3_MORONIC;
					happened = TRUE;
				}
				break;
			}
			case 2:
			{
				if (!(p_ptr->muta2 & MUT2_COWARDICE) && !p_ptr->resist_fear)
				{
					msgf("You become paranoid!");

					/* Duh, the following should never happen, but anyway... */
					if (p_ptr->muta3 & MUT3_FEARLESS)
					{
						msgf("You are no longer fearless.");
						p_ptr->muta3 &= ~(MUT3_FEARLESS);
					}

					p_ptr->muta2 |= MUT2_COWARDICE;
					happened = TRUE;
				}
				break;
			}
			case 3:
			{
				if (!(p_ptr->muta2 & MUT2_HALLU) && !p_ptr->resist_chaos)
				{
					msgf("You are afflicted by a hallucinatory insanity!");
					p_ptr->muta2 |= MUT2_HALLU;
					happened = TRUE;
				}
				break;
			}
			default:
			{
				if (!(p_ptr->muta2 & MUT2_BERS_RAGE))
				{
					msgf("You become subject to fits of berserk rage!");
					p_ptr->muta2 |= MUT2_BERS_RAGE;
					happened = TRUE;
				}
				break;
			}
		}
	}

	p_ptr->update |= PU_BONUS;
	handle_stuff();
}


bool get_nightmare(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Require eldritch horrors */
	if (!(r_ptr->flags4 & (RF4_ELDRITCH_HORROR))) return (FALSE);

	/* Require high level */
	if (r_ptr->level <= p_ptr->lev) return (FALSE);

	/* Accept this monster */
	return (TRUE);
}


static void building_prt_gold(void)
{
	prtf(40, 23, "Gold Remaining: %9ld", (long)p_ptr->au);
}

/* Does the player have enough gold for this action? */
bool test_gold(s32b *cost)
{
	if (p_ptr->au < *cost)
	{
		/* Player does not have enough gold */

		msgf("You need %ld gold to do this!", (long)*cost);
		message_flush();

		*cost = 0;

		return (FALSE);
	}

	/* Player has enough gold */
	return (TRUE);
}



/*
 * Display a building.
 */
static void display_build(const field_type *f_ptr, const store_type *b_ptr)
{
	const b_own_type *bo_ptr = &b_owners[f_ptr->data[0]][b_ptr->owner];

	int factor;

	cptr build_name = t_info[f_ptr->t_idx].name;
	cptr owner_name = (bo_ptr->owner_name);
	cptr race_name = race_info[bo_ptr->owner_race].title;

	/* Add in the charisma factor */
	factor = 100 + adj_chr_gold[p_ptr->stat_ind[A_CHR]];

	factor = ((factor + 100) * bo_ptr->inflate) / 400;

	Term_clear();
	prtf(1, 2, "%s (%s) %s", owner_name, race_name, build_name);
	prtf(0, 19, "You may:");


	/* Display building-specific information */
	field_hook(&area(p_ptr->px, p_ptr->py)->fld_idx,
			   FIELD_ACT_STORE_ACT1, factor);

	prtf(0, 23, " ESC) Exit building");

	/* Show your gold */
	building_prt_gold();
}


/*
 * display fruit for dice slots
 */
static void display_fruit(int col, int row, int fruit)
{
	switch (fruit)
	{
		case 0:
		{
			/* lemon */
			put_fstr(col, row, CLR_YELLOW
							"   ####.\n"
							"  #    #\n"
							" #     #\n"
							"#      #\n"
							"#      #\n"
							"#     # \n"
							"#    #  \n"
							".####   \n"
				CLR_WHITE	" Lemon  ");
			break;
		}
		case 1:
		{
			/* orange */
			put_fstr(col, row, CLR_ORANGE
							"   ##   \n"
							"  #..#  \n"
							" #....# \n"
							"#......#\n"
							"#......#\n"
							" #....# \n"
							"  #..#  \n"
							"	##   \n"
				CLR_WHITE	" Orange ");
			break;
		}
		case 2:
		{
			/* sword */
			put_fstr(col, row, CLR_SLATE
							"   /\\   \n"
							"	##   \n"
							"	##   \n"
							"	##   \n"
							"	##   \n"
							"	##   \n"
				CLR_UMBER	" ###### \n"
				CLR_UMBER	"   ##   \n"
				CLR_WHITE	" Sword  ");
			break;
		}
		case 3:
		{
			/* shield */
			put_fstr(col, row, CLR_SLATE
							" ###### \n"
							"#  	#\n"
							"# ++++ #\n"
							"# +==+ #\n"
							"#  ++  #\n"
							" #    # \n"
							"  #  #  \n"
							"	##   \n"
				CLR_WHITE	" Shield ");
			break;
		}
		case 4:
		{
			/* plum */
			put_fstr(col, row, CLR_VIOLET
							"   ##   \n"
							" ###### \n"
							"########\n"
							"########\n"
							"########\n"
							" ###### \n"
							"  ####  \n"
							"	##   \n"
				CLR_WHITE	"  Plum  ");
			break;
		}
		case 5:
		{
			/* cherry */
			put_fstr(col, row, CLR_GREEN
            				"      ##\n"
					CLR_RED	"   ##" CLR_GREEN "#  \n"
					CLR_RED "  #..#  \n"
							"  #..#  \n"
							" ###### \n"
							"#..##..#\n"
							"#..##..#\n"
							" ##  ## \n"
				CLR_WHITE	" Cherry ");
			break;
		}
	}
}


/* The amount of gold you have before gambling */
static s32b gamble_oldgold;


/*
 * Initialize gambling by getting bet.
 * Return a wager of zero, if something goes wrong.
 *
 * Hack - we return with screen still saved if wager is non-zero
 */
static s32b gamble_init(void)
{
	char out_val[160];
	cptr p;

	s32b wager;
	s32b maxbet;

	/* Save gold before gambling */
	gamble_oldgold = p_ptr->au;

	screen_save();

	/* No money */
	if (p_ptr->au < 1)
	{
		msgf("Hey! You don't have gold - get out of here!");

		screen_load();
		return (0);
	}

	clear_region(0, 5,23);

	/* Set maximum bet */
	if (p_ptr->lev < 10)
		maxbet = p_ptr->lev * 100;
	else
		maxbet = p_ptr->lev * 1000;

	/* We can't bet more than we have */
	maxbet = MIN(maxbet, p_ptr->au);

	/* Get the wager */
	out_val[0] = 0;

	/*
	 * Use get_string() because we may need more than
	 * the s16b value returned by get_quantity().
	 */
	if (!get_string(out_val, 33, "Your wager (1-%ld) ? ", maxbet))
	{
		screen_load();
		return (0);
	}

	/* Strip spaces */
	for (p = out_val; *p == ' '; p++) ;

	/* Get the wager */
	wager = atol(p);

	if (wager > p_ptr->au)
	{
		msgf("Hey! You don't have the gold - get out of here!");

		screen_load();
		return (0);
	}
	else if (wager > maxbet)
	{
		msgf("I'll take %ld gold of that. Keep the rest.", maxbet);
		wager = maxbet;
	}
	else if (wager < 1)
	{
		msgf("Ok, we'll start with 1 gold.");
		wager = 1;
	}

	message_flush();

	prtf(2, 20, "Gold before game: %9ld\n"
				"Current Wager:    %9ld", p_ptr->au, wager);

	/* Prevent savefile-scumming of the casino */
	Rand_quick = TRUE;
	Rand_value = time(NULL);

	/* Return the amount of gold bet */
	return (wager);
}


static bool gamble_again(bool win, int odds, s32b wager)
{
	char again;

	if (win)
	{
		prtf(37, 16, "YOU WON\n"
					 "Payoff: %ld\n"
                     "Again(Y/N)?", odds * wager);
        p_ptr->au += odds * wager;
	}
	else
	{
		prtf(37, 16, "You Lost\n"
					 "\n"
                     "Again(Y/N)?");
        p_ptr->au -= wager;
	}

	prtf(2, 22, "Current Gold:     %9ld", p_ptr->au);
	Term_gotoxy(49, 18);
	again = inkey();

	if ((again != 'y') && (again != 'Y')) return (FALSE);

	if (wager > p_ptr->au)
	{
		msgf("Hey! You don't have the gold - get out of here!");
		message_flush();

		/* Get out here */
		return (FALSE);
	}

	/* The player wants another go */
	return (TRUE);
}


/*
 * Finished gambling
 */
static void gamble_done(void)
{
	/* Switch back to complex RNG */
	Rand_quick = FALSE;

	prtf(37, 18, "");
	if (p_ptr->au >= gamble_oldgold)
		msgf("You came out a winner! We'll win next time, I'm sure.");
	else
		msgf("You lost gold! Haha, better head home.");

	message_flush();

	screen_load();
}


void gamble_help(void)
{
	/* Peruse the gambling help file */
	(void)show_file("gambling.txt", NULL, 0, 0);
}


void gamble_in_between(void)
{
	s32b wager = gamble_init();
	int roll1, roll2, choice;

	bool win;

	/* Exit if the player is out of gold */
	if (!wager) return;

	while (TRUE)
	{
		win = FALSE;

		put_fstr(2, 5, CLR_GREEN "In Between");

		roll1 = randint1(10);
		roll2 = randint1(10);
		choice = randint1(10);

		put_fstr(3, 8, "Black die: %d       Black Die: %d", roll1, roll2);
		put_fstr(14, 11, "Red die: %d", choice);

		if (((choice > roll1) && (choice < roll2)) ||
			((choice < roll1) && (choice > roll2)))
			win = TRUE;
		if (!gamble_again(win, 3, wager)) break;
	}

	gamble_done();
}


void gamble_craps(void)
{
	s32b wager = gamble_init();

	int roll1, roll2, roll3, choice;

	bool win;

	/* Exit if the player is out of gold */
	if (!wager) return;

	while (TRUE)
	{
		put_fstr(2, 5, CLR_GREEN "Craps");

		/* Roll the dice */
		roll1 = randint1(6);
		roll2 = randint1(6);
		roll3 = roll1 + roll2;
		choice = roll3;

		put_fstr(5, 7, "First roll: %d %d    Total: %d", roll1, roll2, roll3);

		/* Is it is result straight away? */
		if ((roll3 == 7) || (roll3 == 11))
			win = TRUE;
		else if ((roll3 == 2) || (roll3 == 3) || (roll3 == 12))
			win = FALSE;
		else
			while (TRUE)
			{
				/* Ok - we need to roll a few more times */
				msgf("Hit any key to roll again");
				message_flush();

				roll1 = randint1(6);
				roll2 = randint1(6);
				roll3 = roll1 + roll2;

				prtf(5, 8, "Roll result: %d %d   Total:     %d",
						roll1, roll2, roll3);

				if (roll3 == choice)
					win = TRUE;
				else if (roll3 == 7)
					win = FALSE;
				else
					continue;

				/* We have a result */
				break;
			}
		if (!gamble_again(win, 1, wager)) break;
	}

	gamble_done();
}


void gamble_spin_wheel(void)
{
	s32b wager = gamble_init();

	int roll1, choice;

	bool win;

	/* Exit if the player is out of gold */
	if (!wager) return;

	while (TRUE)
	{
		win = FALSE;

		put_fstr(2, 5, CLR_GREEN "Wheel");
		prtf(5, 7, "1  2  3  4  5  6  7  8  9 10");
		prtf(3, 8, "--------------------------------");

		choice = get_quantity("Pick a number (1-10): ", 10);

		message_flush();
		roll1 = randint1(10);
		prtf(3, 13, "The wheel spins to a stop and the winner is %d",
				roll1);
		clear_row(9);
		prtf((3 * roll1 + 2), 9, "*");

		if (roll1 == choice) win = TRUE;

		if (!gamble_again(win, 8, wager)) break;
	}

	gamble_done();
}


void gamble_dice_slots(void)
{
	static const char *fruit[6] =
	{ "Lemon", "Orange", "Sword", "Shield", "Plum", "Cherry" };

	s32b wager = gamble_init();

	int roll1, roll2, choice;
	int odds;

	bool win;

	/* Exit if the player is out of gold */
	if (!wager) return;

	while (TRUE)
	{
		put_fstr(2, 5, CLR_GREEN "Dice Slots");
		win = FALSE;
		odds = 0;

		/* Roll the dice */
		roll1 = randint1(6);
		roll2 = randint1(6);
		choice = randint1(6);

		/* Show the result */
		prtf(37, 15, "%s %s %s", fruit[roll1 - 1], fruit[roll2 - 1],
				fruit[choice - 1]);
		prtf(2, 7, "/--------------------------\\");
		prtf(2, 17, "\\--------------------------/");

		display_fruit(3, 8, roll1 - 1);
		display_fruit(12, 8, roll2 - 1);
		display_fruit(21, 8, choice - 1);

		/* What did we win? */
		if ((roll1 == roll2) && (roll2 == choice))
		{
			win = TRUE;
			if (roll1 == 1)
				odds = 4;
			else if (roll1 == 2)
				odds = 6;
			else
				odds = roll1 * roll1;
		}
		else if ((roll1 == 6) && (roll2 == 6))
		{
			win = TRUE;
			odds = choice + 1;
		}

		if (!gamble_again(win, odds, wager)) break;
	}

	gamble_done();
}


/*
 * The Inn
 * Note that resting for the night was a perfect way to avoid player
 * ghosts in the town *if* you could only make it to the inn in time (-:
 * Now that the ghosts are temporarily disabled in 2.8.X, this function
 * will not be that useful.
 *
 * (Although food is fairly hard to find elsewhere - so the Inn is quite
 *  useful when you are hungry after crossing the wilderness.)
 *
 * Resting at night is also a quick way to restock stores -KMW-
 */
bool inn_rest(void)
{
	/* Only at night time */
	if ((turn % (10L * TOWN_DAWN)) < 50000)
	{
		msgf("The rooms are available only at night.");
		message_flush();

		return (FALSE);
	}

	/* Hurt? */
	if ((p_ptr->poisoned) || (p_ptr->cut))
	{
		msgf("You need a healer, not a room.");
		message_flush();
		msgf("Sorry, but don't want anyone dying in here.");
		message_flush();

		return (FALSE);
	}

	/* Rest all night */
	turn = ((turn / 50000) + 1) * 50000;
	p_ptr->chp = p_ptr->mhp;

	/*
	 * Nightmare mode has a TY_CURSE at midnight...
	 * and the player may want to avoid that.
	 */
	if (ironman_nightmare)
	{
		msgf("Horrible visions flit through your mind as you sleep.");

		/* Pick a nightmare */
		get_mon_num_prep(get_nightmare, NULL);

		/* Have some nightmares */
		while (TRUE)
		{
			have_nightmare(get_mon_num(MAX_DEPTH));

			if (!one_in_(3)) break;
		}

		/* Remove the monster restriction */
		get_mon_num_prep(NULL, NULL);

		msgf("You awake screaming.");
		message_flush();

		return (TRUE);
	}

	/* Normally heal the player */
	(void)set_blind(0);
	(void)set_confused(0);
	p_ptr->stun = 0;
	p_ptr->csp = p_ptr->msp;

	msgf("You awake refreshed for the new day.");
	message_flush();

	return (TRUE);
}


#define WEP_MAST_COL1	2
#define WEP_MAST_COL2	45

/*
 * Display the damage figure of an object
 * (used by compare_weapon_aux1)
 *
 * Only accurate for the current weapon, because it includes
 * the current +dam of the player.
 */
static void compare_weapon_aux2(const object_type *o_ptr, int numblows,
                                int r, cptr attr, byte slay)
{
	long maxdam, mindam;
	int dambonus;

	int intmaxdam, intmindam;

	dambonus = o_ptr->to_d + p_ptr->to_d;

	/* Include effects of slaying bonus */
	mindam = o_ptr->dd * slay * 10;

	maxdam = (o_ptr->dd * o_ptr->ds * deadliness_calc(dambonus) * slay) / 10;

	/* Include effects of slaying bonus */
	mindam += (slay - 10) * 100;
	maxdam += (slay - 10) * 100;

	/* Number of blows */
	maxdam *= numblows;
	mindam *= numblows;

	/* rescale */
	intmaxdam = maxdam / 100;
	intmindam = mindam / 100;

	/* Print the intro text */
	put_fstr(WEP_MAST_COL2, r, attr);

	/* Print the damage */
	put_fstr(WEP_MAST_COL2 + 8, r, " %d-%d damage", intmindam, intmaxdam);
}


/*
 * Show the damage figures for the various monster types
 *
 * Only accurate for the current weapon, because it includes
 * the current number of blows for the player.
 */
static void compare_weapon_aux1(const object_type *o_ptr)
{
	int r = 10;

	u32b f1, f2, f3;

	/* Get the flags of the weapon */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Print the relevant lines */
	if (f1 & TR1_SLAY_ANIMAL) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												  CLR_YELLOW "Animals:", 17);
	if (f1 & TR1_SLAY_EVIL) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												CLR_YELLOW "Evil:", 15);
	if (f1 & TR1_SLAY_UNDEAD) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												  CLR_YELLOW "Undead:", 20);
	if (f1 & TR1_SLAY_DEMON) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_YELLOW "Demons:", 20);
	if (f1 & TR1_SLAY_ORC) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
											   CLR_YELLOW "Orcs:", 20);
	if (f1 & TR1_SLAY_TROLL) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_YELLOW "Trolls:", 20);
	if (f1 & TR1_SLAY_GIANT) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_YELLOW "Giants:", 20);
	if (f1 & TR1_SLAY_DRAGON) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												  CLR_YELLOW "Dragons:", 20);
	if (f1 & TR1_KILL_DRAGON) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												  CLR_YELLOW "Dragons:", 30);
	if (f1 & TR1_BRAND_ACID) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_RED "Acid:", 20);
	if (f1 & TR1_BRAND_ELEC) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_RED "Elec:", 20);
	if (f1 & TR1_BRAND_FIRE) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_RED "Fire:", 20);
	if (f1 & TR1_BRAND_COLD) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_RED "Cold:", 20);
	if (f1 & TR1_BRAND_POIS) compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++,
												 CLR_RED "Poison:", 20);
}


/*
 * Calculate the probability of successful hit for a weapon and certain AC
 *
 * Only accurate for the current weapon, because it includes
 * player's +to_hit.
 */
static int hit_prob(int to_h, int ac)
{
	int chance = p_ptr->skill_thn + (p_ptr->to_h + to_h) * BTH_PLUS_ADJ;
	int prob = 0;

	if (chance > 0 && ac < chance) prob = (100 * (chance - ac) / chance);
	return (5 + 95 * prob / 100);
}


/*
 * Calculate the probability of critical hit for a weapon 
 *
 * Only accurate for the current weapon, because it includes
 * player's +to_hit.
 */
static int critical_prob(int to_h, int number)
{
	int chance = p_ptr->skill_thn + (p_ptr->to_h + to_h) * BTH_PLUS_ADJ;

	if (chance <= 0) return (0);

	/* Chance we make a critical */
	chance = (chance * 100) / (chance + 240);

	/* Which critical do we want? */
	switch (number)
	{
		case 0:
		{
			/* No critical */
			return (100 - chance);
		}

		case 1:
		{
			/* x 1.5 */
			chance = ((chance * 89) / 90);
			chance = ((chance * 39) / 40);
			chance = ((chance * 11) / 12);

			return (2 * chance / 3);
		}

		case 2:
		{
			/* x 2.0 */
			chance = ((chance * 89) / 90);
			chance = ((chance * 39) / 40);
			chance = ((chance * 11) / 12);

			return (chance / 3);
		}

		case 3:
		{
			/* x 2.7 */
			chance = ((chance * 89) / 90);
			chance = ((chance * 39) / 40);

			return (chance / 12);
		}

		case 4:
		{
			/* x 3.6 */
			chance = ((chance * 89) / 90);

			return (chance / 40);
		}

		case 5:
		{
			/* x 5.0 */
			return (chance / 90);
		}
	}

	/* Paranoia */
	return (0);
}


/*
 * Displays all info about a weapon
 *
 * Only accurate for the current weapon, because it includes
 * various info about the player's +to_dam and number of blows.
 */
static void list_weapon(const object_type *o_ptr)
{
	int dambonus;

	int intmaxdam, intmindam;

	/* Print the weapon name */
	put_fstr(WEP_MAST_COL1, 6, CLR_L_BLUE "%v", OBJECT_FMT(o_ptr, TRUE, 0));

	/* Print to_hit and to_dam of the weapon */
	put_fstr(WEP_MAST_COL1, 8, "To Hit: %d  Deadliness: %d", o_ptr->to_h, o_ptr->to_d);

	/* Print the weapons base damage dice and blows */
	put_fstr(WEP_MAST_COL1, 10, "Dice: %dd%d    Number of Blows: %d",
			(int)o_ptr->dd, (int)o_ptr->ds, p_ptr->num_blow);

	/* Print hit probabilities */
	put_fstr(WEP_MAST_COL1, 12, "Enemy AC:  Low   Medium  High \n"
								"Hit Prob:  %2d%% %2d%% %2d%% %2d%% %2d%%",
			hit_prob(o_ptr->to_h, 25), hit_prob(o_ptr->to_h, 50),
			hit_prob(o_ptr->to_h, 75), hit_prob(o_ptr->to_h, 100),
			hit_prob(o_ptr->to_h, 200));

	/* Print critical hit probabilities */
	put_fstr(WEP_MAST_COL1, 15, CLR_RED "Critical:" CLR_WHITE " 1.0 1.5 2.0 2.7 3.6 5.0\n"
										"          %2d%% %2d%% %2d%% %2d%% %2d%% %2d%%",
			critical_prob(o_ptr->to_h, 0),
			critical_prob(o_ptr->to_h, 1),
			critical_prob(o_ptr->to_h, 2),
			critical_prob(o_ptr->to_h, 3),
			critical_prob(o_ptr->to_h, 4), critical_prob(o_ptr->to_h, 5));

	put_fstr(WEP_MAST_COL2, 6, CLR_L_BLUE "Possible Damage:");

	dambonus = o_ptr->to_d + p_ptr->to_d;

	/* Calculate max and min damage */
	intmindam = o_ptr->dd;
	intmaxdam = (o_ptr->dd * o_ptr->ds * deadliness_calc(dambonus)) / 100;


	/* Damage for one blow (if it hits) */
	put_fstr(WEP_MAST_COL2, 7, "One Strike: %d-%d damage", intmindam, intmaxdam);

	/* Rescale */
	intmindam *= p_ptr->num_blow;
	intmaxdam *= p_ptr->num_blow;

	/* Damage for the complete attack (if all blows hit) */
	put_fstr(WEP_MAST_COL2, 8, "One Attack: %d-%d damage", intmindam, intmaxdam);
}


/*
 * Compare weapons
 *
 * Copies the weapons to compare into the weapon-slot and
 * compares the values for both weapons.
 */
bool compare_weapons(void)
{
	object_type *o_ptr;

	/* Clear the screen */
    clear_region(0, 6, 18);

	/* Point to wielded weapon */
	o_ptr = &p_ptr->equipment[EQUIP_WIELD];

	/* Check to see if we have one */
	if (!o_ptr->k_idx)
	{
		msgf("You need to wield a weapon.");
		return (FALSE);
	}

	put_fstr(2, 4, 
			"Based on your current abilities, here is what your weapon will do:");

	/* Identify the weapon */
	identify_item(o_ptr);
	object_mental(o_ptr);

	/* Save all the known flags */
	o_ptr->kn_flags1 = o_ptr->flags1;
	o_ptr->kn_flags2 = o_ptr->flags2;
	o_ptr->kn_flags3 = o_ptr->flags3;

	/* Erase the "feeling" */
	o_ptr->feeling = FEEL_NONE;


	/* List the new values */
	list_weapon(o_ptr);
	compare_weapon_aux1(o_ptr);

	put_fstr(0, 20,
			"(Only highest damage applies per monster. Special damage not cumulative.)");

	/* Done */
	return (TRUE);
}


/*
 * Enchant item
 */
bool enchant_item(s32b cost, bool to_hit, bool to_dam, bool to_ac)
{
	bool okay = FALSE;
	object_type *o_ptr;
	cptr q, s;
	int maxenchant = (p_ptr->lev / 5);
	int maxenchant_d = (p_ptr->lev / 3);

    clear_region(0, 5, 18);
    
	if (to_dam)
		prtf(0, 5, "  Based on your skill, we can improve up to +%d,+%d%%.", maxenchant, maxenchant_d * 3);
	else
		prtf(0, 5, "  Based on your skill, we can improve up to +%d.", maxenchant);
	prtf(0, 7, "  The price for the service is %d gold per item.", cost);

	/* Get an item */
	q = "Improve which item? ";
	s = "You have nothing to improve.";

	/* Get the item */
	o_ptr = get_item(q, s, (USE_INVEN | USE_EQUIP));

	/* No valid items */
	if (!o_ptr) return (FALSE);

	/* Check if the player has enough money */
	if (p_ptr->au < (cost * o_ptr->number))
	{
		msgf("You do not have the gold to improve %v!",
			OBJECT_FMT(o_ptr, TRUE, 0));
		message_flush();
		return (FALSE);
	}

	/* Note that enchanting something a negative number of times will fail */

	/* Enchant to hit */
	if ((to_hit) && (enchant(o_ptr, maxenchant - o_ptr->to_h,
							 (ENCH_TOHIT | ENCH_FORCE))))
	{
		okay = TRUE;
	}

	/* Enchant to damage */
	if ((to_dam) && (enchant(o_ptr, maxenchant_d - o_ptr->to_d,
							 (ENCH_TODAM | ENCH_FORCE))))
	{
		okay = TRUE;
	}

	/* Enchant to AC */
	if ((to_ac) && (enchant(o_ptr, maxenchant - o_ptr->to_a,
							(ENCH_TOAC | ENCH_FORCE))))
	{
		okay = TRUE;
	}

	/* Failure */
	if (!okay)
	{
		/* Flush */
		if (flush_failure) flush();

		/* Message */
		msgf("The improvement failed.");

		return (FALSE);
	}
	else
	{
		msgf("Improved %v for %d gold.", OBJECT_FMT(o_ptr, TRUE, 1),
			 cost * o_ptr->number);
		message_flush();

		/* Charge the money */
		p_ptr->au -= (cost * o_ptr->number);

		/* Something happened */
		return (TRUE);
	}
}

/*
 * Sell a soul to the soul dealer
 */
void building_sellsoul(void)
{
	object_type *o_ptr;
	object_kind *k_ptr;
	monster_race *r_ptr;
	cptr q, s;
	int delta_level, i;
	obj_theme theme;
	object_type* q_ptr;
	int num_mon_types;

	/* Only accept legal items */
	item_tester_hook = item_tester_hook_soulgem;

	/* Get an item */
	q = "Deposit which soul gem? ";
	s = "You have no soul gems to deposit.";

	o_ptr = get_item(q, s, (USE_INVEN | USE_FLOOR));

	/* No valid item */
	if (!o_ptr) return;

	k_ptr = &k_info[o_ptr->k_idx];

	r_ptr = &r_info[o_ptr->soul_source];

	/* Take the soul gem from the player, describe the result */
	item_increase(o_ptr, -1);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	// we already have a soul of this type
	if (r_ptr->r_xtra1 > 0)
	{
		msgf("Some money awaits you outside.");

		/* Make some gold */
		q_ptr = make_gold(0);

		/* Paranoia */
		if (q_ptr)
		{
			/* Drop it outside */
			drop_near(q_ptr, -1, p_ptr->px, p_ptr->py);
		}

		return;
	}

	//else

	msgf("A trinket awaits you outside.");

	/* Mark the monster race as added to the library */
	r_ptr->r_xtra1 = 1;

	/* Make an object as reward*/
	theme.combat   = 100;
	theme.magic    = 0;
	theme.tools    = 0;
	theme.treasure = 0;

	// the 'level' of the object is based on the number of soul types we have turned in

	/* set the base level */
	object_level = 0;

	//start with a minimum goodness level
	delta_level = 15;

	//count all the souls we have deposited
	for (i=0; i<z_info->r_max; i++)
	{
		if (r_info[i].r_xtra1 > 0) delta_level++;
	}

	if (p_ptr->wizard) msgf("Generating item with delta_level %i", delta_level);

  for (i = 0; i < 1000; i++)
  {
      /* Make an object */
			q_ptr = make_object(delta_level, theme);

      if (!q_ptr) continue;
  }

	/* Paranoia */
	if (q_ptr)
	{
		/* Drop it outside */
		drop_near(q_ptr, -1, p_ptr->px, p_ptr->py);
	}

	/* Finished */
	return;
}

/*
 * View souls sold to the soul dealer
 */
void building_viewsouls(int delta)
{
	monster_race *r_ptr;
	int i, j, row, col, limit;
	static int page = 0;
	int pagesize = 24;
	int base = 0;

	if (delta > 0) page++;
	if (delta < 0) page--;

	if (page < 0) page = 0;

	limit = z_info->r_max;

  clear_region(0, 4, 17);

	if ( page * pagesize > limit ) page--;

	for (j=0; j<pagesize; j++)
	{
		row = j % 12 + 4;
		col = (j / 12) * 40;

		i = (page*pagesize + j);

		if ( i < limit)
		{
			r_ptr = &r_info[i];

	 		if (!(r_ptr->flags1 & RF1_UNIQUE) && !(r_ptr->flags3 & RF3_UNIQUE_7))
			{
				base++;
			}

			/* Label it, clear the line --(-- */
	 		prtf(col, row, "%i) ", i);

			// if we have previously sold a soul of this type
			if (r_ptr->r_xtra1 == 1)	prtf(col + 5, row, "X");
				else prtf(col + 5, row, "-");

			// an unobtainable soul
			if ( (r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags3 & RF3_UNIQUE_7) || (i == 0) )
			{
				/* Print monster name */
				prtf(col + 7, row, "XXX" );
			}
			// if we have previously seen this monster or sold a soul of this type
			else if ( (r_ptr->r_xtra1 == 1) || (r_ptr->r_sights))
			{
				/* Print monster name */
				prtf(col + 7, row, r_name + r_ptr->name);
			}
			else
			{
				prtf(col + 7, row, "???");
			}
		}
	}

	/* Finished */
	return;
}

/*
 * Examine a soul
 */
bool building_evaluatesoul()
{
	object_type* o_ptr;
	soul_type*   s_ptr;
	soul_type*   s_ptr2;
	u32b save_flags1;
	u32b save_flags2;
	u32b save_flags3;
	s16b save_pval;
	s16b save_level;
	s16b save_to_h;
	s16b save_to_d;
	s16b save_to_a;
	int i;
	cptr q, s;

	/* Only accept legal items */
	item_tester_hook = item_tester_hook_hassoul;

	/* Get an item */
	q = "Examine which soul?";
	s = "You have no soul to examine.";

	o_ptr = get_item(q, s, (USE_INVEN | USE_EQUIP | USE_FLOOR));

	/* No valid item */
	if (!o_ptr) return FALSE;

	/* Big hack, save the old flags */
	save_flags1 = o_ptr->kn_flags1;
	save_flags2 = o_ptr->kn_flags2;
	save_flags3 = o_ptr->kn_flags3;
	save_pval   = o_ptr->pval;
	save_level  = o_ptr->level;
	save_to_h   = o_ptr->to_h;
	save_to_d   = o_ptr->to_d;
	save_to_a   = o_ptr->to_a;

	o_ptr->kn_flags1 = 0;
	o_ptr->kn_flags2 = 0;
	o_ptr->kn_flags3 = 0;
	o_ptr->pval      = 6;
	o_ptr->level     = 6;

	s_ptr  = &s_info[o_ptr->soul_type1];
	s_ptr2 = &s_info[o_ptr->soul_type2];

	o_ptr->to_h      = s_ptr->max_to_h;
	o_ptr->to_d      = s_ptr->max_to_d;
	o_ptr->to_a      = s_ptr->max_to_a;

	for (i=0; i<6; i++)
	{
		o_ptr->kn_flags1 |= s_ptr->flags1[i];
		o_ptr->kn_flags2 |= s_ptr->flags2[i];
		o_ptr->kn_flags3 |= s_ptr->flags3[i];

		o_ptr->kn_flags1 |= s_ptr2->flags1[i];
		o_ptr->kn_flags2 |= s_ptr2->flags2[i];
		o_ptr->kn_flags3 |= s_ptr2->flags3[i];
	}

	msgf("When fully powered, the %v will look like:", OBJECT_FMT(o_ptr, TRUE, 3));

	/* Describe it fully */
	(void)identify_fully_aux(o_ptr);

	o_ptr->kn_flags1 = save_flags1;
	o_ptr->kn_flags2 = save_flags2;
	o_ptr->kn_flags3 = save_flags3;
	o_ptr->pval      = save_pval;
	o_ptr->level     = save_level;
	o_ptr->to_h      = save_to_h;
	o_ptr->to_d      = save_to_d;
	o_ptr->to_a      = save_to_a;

	/* Finished */
	return TRUE;
}


/*
 * Imbue Item
 *
 * Imbues a soul into an amulet or ring
 */
void building_imbuesoul(void)
{
	object_type *o_ptr;
	object_type *s_ptr;
	cptr q, s;

	/* Only accept legal items */
	item_tester_hook = item_tester_hook_imbue;

	/* Get an item */
	q = "Imbue which item? ";
	s = "You have no imbueable items";

	o_ptr = get_item(q, s, (USE_INVEN | USE_FLOOR | USE_EQUIP));

	/* No valid item */
	if (!o_ptr) return;

	/* Only accept legal items */
	item_tester_hook = item_tester_hook_soulgem;

	/* Get an item */
	q = "Which soul will be imbued? ";
	s = "You have no soul gems";

	s_ptr = get_item(q, s, (USE_INVEN | USE_FLOOR ));

	/* No valid item */
	if (!s_ptr) return;

	//give it a soul type

	o_ptr->soul_source = s_ptr->soul_source;
	o_ptr->soul_type1  = s_ptr->soul_type1;
	o_ptr->soul_type2  = s_ptr->soul_type2;

	/* Set the first flags */
	o_ptr->flags1 |= s_info[o_ptr->soul_type1].flags1[0];
	o_ptr->flags2 |= s_info[o_ptr->soul_type1].flags2[0];
	o_ptr->flags3 |= s_info[o_ptr->soul_type1].flags3[0];

	o_ptr->flags1 |= s_info[o_ptr->soul_type2].flags1[0];
	o_ptr->flags2 |= s_info[o_ptr->soul_type2].flags2[0];
	o_ptr->flags3 |= s_info[o_ptr->soul_type2].flags3[0];

	o_ptr->pval = 1;

	//start the levels counting
	o_ptr->level = 1;

	/* gain any bonuses to AC / Damage / Accuracy */

	o_ptr->to_h = (o_ptr->level * s_info[o_ptr->soul_type1].max_to_h) / 6;
	o_ptr->to_d = (o_ptr->level * s_info[o_ptr->soul_type1].max_to_d) / 6;
	o_ptr->to_a = (o_ptr->level * s_info[o_ptr->soul_type1].max_to_a) / 6;


	/* Identify the item */
	identify_item(o_ptr);
	object_mental(o_ptr);

	/* Save all the known flags */
	o_ptr->kn_flags1 = o_ptr->flags1;
	o_ptr->kn_flags2 = o_ptr->flags2;
	o_ptr->kn_flags3 = o_ptr->flags3;

	/* if we are imbuing a stack of rings, put a stop to that nonsense */
	if (o_ptr->number > 1)
	{
		msgf("You cannot imbue more than one item.");
		msgf("I have taken the excess %d.", (o_ptr->number) - 1);

		o_ptr->number = 1;

		/* Notice weight changes */
		p_ptr->update |= PU_WEIGHT;
	}

	/* Take the soul away from the player, describe the result */
	item_increase(s_ptr, -1);
}

/*
 * Recharge rods, wands and staves
 *
 * The player can select the number of charges to add
 * (up to a limit), and the recharge never fails.
 *
 * The cost for rods depends on the level of the rod. The prices
 * for recharging wands and staves are dependent on the cost of
 * the base-item.
 */
void building_recharge(s32b cost)
{
	int lev;
	object_type *o_ptr;
	object_kind *k_ptr;
	cptr q, s;
	int price;
	int charges;
	int max_charges;


	/* Display some info */
    clear_region(0, 5, 18);
	prtf(0, 6, "  The prices of recharge depend on the type.");

	/* Only accept legal items */
	item_tester_hook = item_tester_hook_recharge;

	/* Get an item */
	q = "Recharge which item? ";
	s = "You have nothing to recharge.";

	o_ptr = get_item(q, s, (USE_INVEN | USE_FLOOR));

	/* No valid item */
	if (!o_ptr) return;

	k_ptr = &k_info[o_ptr->k_idx];

	/*
	 * We don't want to give the player free info about
	 * the level of the item or the number of charges.
	 */
	/* The item must be "known" */
	if (!object_known_p(o_ptr))
	{
		msgf("The item must be identified first!");
		message_flush();

		if ((p_ptr->au >= 50) && get_check("Identify for 50 gold? "))
		{
			/* Pay the price */
			p_ptr->au -= 50;

			/* Identify it */
			identify_item(o_ptr);

			/* Description */
			msgf("You have: %v.", OBJECT_FMT(o_ptr, TRUE, 3));
		}
		else
		{
			return;
		}
	}

	/* Extract the object "level" */
	lev = get_object_level(o_ptr);

	/* Price for a rod */
	if (o_ptr->tval == TV_ROD)
	{
		if (o_ptr->timeout > 0)
		{
			/* Fully recharge */
			price = (lev * o_ptr->timeout) / k_ptr->pval;
		}
		else
		{
			/* No recharge necessary */
			msgf("That doesn't need to be recharged.");
			message_flush();
			return;
		}
	}
	else if (o_ptr->tval == TV_STAFF)
	{
		/*
		 * Price per charge ( = double the price paid 
		 * by shopkeepers for the charge)
		 */
		price = (o_ptr->cost / 10) * o_ptr->number;

		/* Pay at least 10 gold per charge */
		price = MAX(10, price);
	}
	else
	{
		/*
		 * Price per charge ( = double the price paid
		 * by shopkeepers for the charge)
		 */
		price = (o_ptr->cost / 10);

		/* Pay at least 10 gold per charge */
		price = MAX(10, price);
	}

	/* Limit the number of charges for wands and staves */
	if (((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF)) &&
		(o_ptr->pval / o_ptr->number >= k_ptr->pval))
	{
		if ((o_ptr->tval == TV_WAND) && (o_ptr->number == 1))
			msgf("This wand is already fully charged.");
		else if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1))
			msgf("These wands are already fully charged.");
		else if ((o_ptr->tval == TV_STAFF) && (o_ptr->number == 1))
			msgf("This staff is already fully charged.");
		else if ((o_ptr->tval == TV_STAFF) && (o_ptr->number > 1))
			msgf("These staffs are already fully charged.");

		message_flush();
		return;
	}

	/* Factor in shopkeeper greed */
	price = (price * cost / 100);

	/* Check if the player has enough money */
	if (p_ptr->au < price)
	{
		msgf("You need %d gold to recharge %v!", price,
			 OBJECT_FMT(o_ptr, TRUE, 0));
		message_flush();
		return;
	}

	if (o_ptr->tval == TV_ROD)
	{
		if (get_check("Recharge the %s for %d gold? ",
							 ((o_ptr->number > 1) ? "rods" : "rod"), price))
		{
			/* Recharge fully */
			o_ptr->timeout = 0;
		}
		else
		{
			return;
		}
	}
	else
	{
		char buf[160];
	
		if (o_ptr->tval == TV_STAFF)
			max_charges = k_ptr->pval - o_ptr->pval;
		else
			max_charges = o_ptr->number * k_ptr->pval - o_ptr->pval;
		
		/* Get prompt */
		strnfmt(buf, 160, "Add how many charges for %d gold? ", price);

		/* Get the quantity for staves and wands */
		charges = get_quantity(buf, MIN(p_ptr->au / price, max_charges));

		/* Do nothing */
		if (charges < 1) return;

		/* Get the new price */
		price *= charges;

		/* Recharge */
		o_ptr->pval += charges;

		/* Hack - no "used" charges */
		o_ptr->ac = 0;

		/* We no longer think the item is empty */
		o_ptr->info &= ~(OB_EMPTY);
	}

	/* Give feedback */
	msgf("%^v %s recharged for %d gold.", OBJECT_FMT(o_ptr, TRUE, 3),
			   ((o_ptr->number > 1) ? "were" : "was"), price);
	message_flush();

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	/* Pay the price */
	p_ptr->au -= price;

	/* Finished */
	return;
}


bool building_healer(void)
{
	bool paid = FALSE;

	if (do_res_stat(A_STR)) paid = TRUE;
	if (do_res_stat(A_INT)) paid = TRUE;
	if (do_res_stat(A_WIS)) paid = TRUE;
	if (do_res_stat(A_DEX)) paid = TRUE;
	if (do_res_stat(A_CON)) paid = TRUE;
	if (do_res_stat(A_CHR)) paid = TRUE;

	if (paid)
	{
		msgf("You are infused with magic, and your ailments disappear.");
		message_flush();
	}

	return (paid);
}


static int collect_magetower_links(int n, int *link_p, int *link_w, s32b *cost,
                                   int factor)
{
	place_type *pl_ptr = &place[p_ptr->place_num];

	int i, j;
	int max_link = 0;

	/* Get current town location */
	int x = pl_ptr->x, y = pl_ptr->y;

	/* Find the magetowers we're linked to */
	for (i = 0; i < place_count; i++)
	{
		place_type *pl_ptr = &place[i];

		/* Skip current town */
		if (i == p_ptr->place_num) continue;

		for (j = 0; j < pl_ptr->numstores; j++)
		{
			store_type *st_ptr = &pl_ptr->store[j];

			if (max_link >= n) return (max_link);

			/* Hack - only allow teleportation to known magetowers */
			if (!st_ptr->data) continue;

			/* Is it a mage tower? */
			if ((st_ptr->type == BUILD_MAGETOWER0) ||
				(st_ptr->type == BUILD_MAGETOWER1))
			{
				link_p[max_link] = i;
				link_w[max_link] = j;
				cost[max_link] = distance(x, y, pl_ptr->x, pl_ptr->y) * factor;
				max_link++;

				/* Only collect 1 link per city */
				break;
			}
		}
	}

	return max_link;
}

bool building_magetower(int factor, bool display)
{
	store_type *st_ptr;

	int link_p[24], link_w[24];
	int max_link = 0;
	int i;

	s32b cost[24];

	char out_val[160];


	/* Save the store pointer */
	st_ptr = get_current_store();

	/* Paranoia */
	if (!st_ptr) return (FALSE);

	/* Collect links */
	max_link = collect_magetower_links(24, link_p, link_w, cost, factor);

	if (display)
	{
		for (i = 0; i < max_link; i++)
		{
			int row = i % 12 + 4;
			int col = (i / 12) * 40;

			/* Label it, clear the line --(-- */
			prtf(col, row, "%c) ", I2A(i));

			/* Print place name */
			prtf(col + 3, row, place[link_p[i]].name);

			/* Print cost */
			prtf(col + 30, row, "%ld au", (long)cost[i]);
		}
	}
	else
	{
		char command;

		if (max_link == 0)
		{
			msgf("You do not know any other towns to teleport to.");
			return (FALSE);
		}

		/* Build the prompt */
		strnfmt(out_val, 160, "(Towns %c-%c, ESC to exit)",
					  I2A(0), I2A(max_link - 1));

		while (TRUE)
		{
			int k;

			/* Escape */
			if (!get_com(out_val, &command)) break;

			k = (islower(command) ? A2I(command) : -1);

			if ((k >= 0) && (k < max_link) && test_gold(&cost[k]))
			{
				place_type *pl_ptr2 = &place[link_p[k]];
				store_type *st_ptr2 = &pl_ptr2->store[link_w[k]];

				/* Subtract off cost */
				p_ptr->au -= cost[k];

				/* Move the player */
				p_ptr->px = pl_ptr2->x * 16 + st_ptr2->x;
				p_ptr->py = pl_ptr2->y * 16 + st_ptr2->y;

				p_ptr->wilderness_x = p_ptr->px;
				p_ptr->wilderness_y = p_ptr->py;

				/* Notice player location */
				Term_move_player();

				/* Remove all monster lights */
				lite_n = 0;

				/* Notice the move */
				move_wild();

				/* Check for new panel (redraw map) */
				verify_panel();

				/* Update stuff */
				p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

				/* Update the monsters */
				p_ptr->update |= (PU_DISTANCE);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

				force_build_exit = TRUE;

				return (TRUE);
			}

			/* Oops */
			bell("Illegal choice!");
		}
	}

	return (FALSE);
}

static bool process_build_hook(field_type *f_ptr, store_type *b_ptr)
{
	const b_own_type *bo_ptr = &b_owners[f_ptr->data[0]][b_ptr->owner];

	int factor;

	factor = 100 + adj_chr_gold[p_ptr->stat_ind[A_CHR]];

	factor = ((factor + 100) * bo_ptr->inflate) / 400;

	field_hook(&area(p_ptr->px, p_ptr->py)->fld_idx,
			   FIELD_ACT_STORE_ACT2, &factor);

	/* Hack XXX XXX, factor is returned as 2 if we want a redraw */
	if (factor == 2)
	{
		/* Redraw screen */
		display_build(f_ptr, b_ptr);
	}

	/* Did we do anything? */
	return (factor);
}


/*
 * Process a command in a building
 *
 * Note that we must disable some commands which are allowed
 * in the dungeon / stores but not in the buildings, to prevent chaos,
 * and also to give more free keys in order to have building
 * specific commands.
 *
 * Hack - we buypass macros / keymaps to prevent silliness when
 * people use the roguelike keyset and press a 'direction' key
 * which also corresponds to a building command.
 */
static bool build_process_command(field_type *f_ptr, store_type *b_ptr)
{
	/* Hack - Get a command */
	p_ptr->command_cmd = inkey();

	/* Handle repeating the last command */
	repeat_check();

	/* Process the building-specific commands */
	if (process_build_hook(f_ptr, b_ptr)) return (FALSE);

	/* Parse the command */
	switch (p_ptr->command_cmd)
	{
		case ESCAPE:
		{
			/* Leave */
			return (TRUE);
		}

		case KTRL('R'):
		{
			/* Redraw */
			do_cmd_redraw();
			display_build(f_ptr, b_ptr);
			break;
		}

		case '\r':
		{
			/* Ignore return */
			break;
		}


		/*** Various commands ***/

		case KTRL('I'):
		{
			/* Hack -- toggle windows */
			toggle_inven_equip();
			break;
		}

		/*** Help and Such ***/

		case '?':
		{
			/* Help */
			do_cmd_help();
			break;
		}

		case '/':
		{
			/* Identify symbol */
			do_cmd_query_symbol();
			break;
		}

		case 'C':
		{
			/* Character description */
			do_cmd_character();
			display_build(f_ptr, b_ptr);
			break;
		}


		/*** System Commands ***/

		case '!':
		{
			/* Hack -- User interface */
			(void)Term_user(0);
			break;
		}

		case '"':
		{
			/* Single line from a pref file */
			do_cmd_pref();
			break;
		}

		case '@':
		{
			/* Interact with macros */
			do_cmd_macros();
			break;
		}

		case '%':
		{
			/* Interact with visuals */
			do_cmd_visuals();
			break;
		}

		case '&':
		{
			/* Interact with colors */
			do_cmd_colors();
			break;
		}

		case '=':
		{
			/* Interact with options */
			do_cmd_options(OPT_FLAG_SERVER | OPT_FLAG_PLAYER);
			break;
		}

		/*** Misc Commands ***/

		case ':':
		{
			/* Take notes */
			do_cmd_note();
			break;
		}

		case 'V':
		{
			/* Version info */
			do_cmd_version();
			break;
		}

		case KTRL('F'):
		{
			/* Repeat level feeling */
			do_cmd_feeling();
			break;
		}

		case KTRL('P'):
		{
			/* Show previous messages */
			do_cmd_messages();
			break;
		}

		case '~':
		case '|':
		{
			/* Check artifacts, uniques etc. */
			do_cmd_knowledge();
			break;
		}

		case '(':
		{
			/* Load "screen dump" */
			do_cmd_load_screen();
			break;
		}

		case ')':
		{
			/* Save "screen dump" */
			do_cmd_save_screen();
			break;
		}

		default:
		{
			/* Hack -- Unknown command */
			msgf("That command does not work in buildings.");
			break;
		}
	}

	return (FALSE);
}


/*
 * Do building commands
 */
void do_cmd_bldg(field_type *f_ptr)
{
	int i, which = -1;
	store_type *b_ptr;
	bool leave_build = FALSE;

	place_type *pl_ptr = &place[p_ptr->place_num];

	/* Get the building the player is on */
	for (i = 0; i < pl_ptr->numstores; i++)
	{
		if ((p_ptr->py - pl_ptr->y * 16 == pl_ptr->store[i].y) &&
			(p_ptr->px - pl_ptr->x * 16 == pl_ptr->store[i].x))
		{
			which = i;
		}
	}

	/* Paranoia */
	if (which == -1)
	{
		msgf("Could not locate building!");
		return;
	}


	b_ptr = &pl_ptr->store[which];

	/* Forget the view */
	forget_view();

	/* Hack -- Increase "icky" depth */
	character_icky++;

	/* No command argument */
	p_ptr->command_arg = 0;

	/* No repeated command */
	p_ptr->command_rep = 0;

	/* No automatic command */
	p_ptr->command_new = 0;

	/* Display the building */
	display_build(f_ptr, b_ptr);

	/* Interact with player */
	while (!leave_build)
	{
		clear_row(1);

		/* Clear */
		clear_from(21);

		/* Basic commands */
		prtf(0, 23, " ESC) Exit building");

		/* Show your gold */
		building_prt_gold();

		/* Process the command */
		leave_build = build_process_command(f_ptr, b_ptr);

		if (force_build_exit)
		{
			force_build_exit = FALSE;
			break;
		}

		/* Hack -- Character is still in "icky" mode */
		character_icky = TRUE;

		/* Notice stuff */
		notice_stuff();

		/* Handle stuff */
		handle_stuff();
	}

	/* Free turn XXX XXX XXX */
	p_ptr->energy_use = 0;

	/* Hack -- Character is no longer in "icky" mode */
	character_icky = FALSE;


	/* Hack -- Cancel automatic command */
	p_ptr->command_new = 0;

	/* Flush messages XXX XXX XXX */
	message_flush();


	/* Clear the screen */
	Term_clear();


	/* Update everything */
	p_ptr->update |= (PU_VIEW);
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw entire screen */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_EQUIPPY);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
}


/*
 * Initialize a building
 */
void build_init(int town_num, int build_num, byte build_type)
{
	/* Activate that building */
	store_type *st_ptr = &place[town_num].store[build_num];

	/* Pick an owner */
	st_ptr->owner = (byte)randint0(MAX_B_OWN);

	/* Set the type */
	st_ptr->type = build_type;

	/* Initialize */
	st_ptr->data = 0;
	st_ptr->last_visit = 0;
}
