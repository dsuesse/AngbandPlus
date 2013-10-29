/* File: monster2.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"



/*
 * Delete a monster by index.
 *
 * When a monster is deleted, all of its objects are deleted.
 */
void delete_monster_idx(int i)
{
	int x, y;

	monster_type *m_ptr = &mon_list[i];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	s16b this_o_idx, next_o_idx = 0;


	/* Get location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* remove CAVE_WALL from dead BLOCK_LOS monsters */
	if (r_ptr->flags7 & (RF7_BLOCK_LOS))
	{
		cave_info[y][x] &= ~(CAVE_WALL);
	}

	/* Hack -- Reduce the racial counter */
	r_ptr->cur_num--;

	/* Hack -- count the number of "reproducers" */
	if (r_ptr->flags2 & (RF2_MULTIPLY)) num_repro--;


	/* Hack -- remove target monster */
	if (p_ptr->target_who == i) target_set_monster(0);

	/* Hack -- remove tracked monster */
	if (p_ptr->health_who == i) health_track(0);

	/* monster is no longer holding the PC */
	if (p_ptr->held_m_idx == i)
	{
		p_ptr->held_m_idx = 0;
		clear_timed(TMD_BEAR_HOLD);
	}

	/* Monster is gone */
	cave_m_idx[y][x] = 0;


	/* Delete objects */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Hack -- efficiency */
		o_ptr->held_m_idx = 0;

		/* Delete the object */
		delete_object_idx(this_o_idx);
	}


	/* Wipe the Monster */
	(void)WIPE(m_ptr, monster_type);

	/* Count monsters */
	mon_cnt--;


	/* Visual update */
	lite_spot(y, x);
}


/*
 * Delete the monster, if any, at a given location
 */
void delete_monster(int y, int x)
{
	/* Paranoia */
	if (!in_bounds(y, x)) return;

	/* Delete the monster (if any) */
	if (cave_m_idx[y][x] > 0) delete_monster_idx(cave_m_idx[y][x]);
}


/*
 * Move an object from index i1 to index i2 in the object list
 */
static void compact_monsters_aux(int i1, int i2)
{
	int y, x;

	monster_type *m_ptr;

	s16b this_o_idx, next_o_idx = 0;


	/* Do nothing */
	if (i1 == i2) return;


	/* Old monster */
	m_ptr = &mon_list[i1];

	/* Location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Update the cave */
	cave_m_idx[y][x] = i2;

	/* Repair objects being carried by monster */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Reset monster pointer */
		o_ptr->held_m_idx = i2;
	}

	/* Hack -- Update the target */
	if (p_ptr->target_who == i1) p_ptr->target_who = i2;

	/* Hack -- Update the health bar */
	if (p_ptr->health_who == i1) p_ptr->health_who = i2;

	/* Hack -- move monster */
	COPY(&mon_list[i2], &mon_list[i1], monster_type);

	/* Hack -- wipe hole */
	(void)WIPE(&mon_list[i1], monster_type);
}


/*
 * Compact and Reorder the monster list
 *
 * This function can be very dangerous, use with caution!
 *
 * When actually "compacting" monsters, we base the saving throw
 * on a combination of monster level, distance from player, and
 * current "desperation".
 *
 * After "compacting" (if needed), we "reorder" the monsters into a more
 * compact order, and we reset the allocation info, and the "live" array.
 */
void compact_monsters(int size)
{
	int i, num, cnt;

	int cur_lev, cur_dis, chance;


	/* Message (only if compacting) */
	if (size) msg_print("Compacting monsters...");


	/* Compact at least 'size' objects */
	for (num = 0, cnt = 1; num < size; cnt++)
	{
		/* Get more vicious each iteration */
		cur_lev = 5 * cnt;

		/* Get closer each iteration */
		cur_dis = 5 * (20 - cnt);

		/* Check all the monsters */
		for (i = 1; i < mon_max; i++)
		{
			monster_type *m_ptr = &mon_list[i];

			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Paranoia -- skip "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Hack -- High level monsters start out "immune" */
			if (r_ptr->level > cur_lev) continue;

			/* Ignore nearby monsters */
			if ((cur_dis > 0) && (m_ptr->cdis < cur_dis)) continue;

			/* Saving throw chance */
			chance = 90;

			/* Only compact "Quest" Monsters in emergencies */
			if ((r_ptr->flags1 & (RF1_QUESTOR)) && (cnt < 1000)) chance = 100;

			/* Try not to compact Unique Monsters */
			if (r_ptr->flags1 & (RF1_UNIQUE)) chance = 99;

			/* All monsters get a saving throw */
			if (rand_int(100) < chance) continue;

			/* Delete the monster */
			delete_monster_idx(i);

			/* Count the monster */
			num++;
		}
	}


	/* Excise dead monsters (backwards!) */
	for (i = mon_max - 1; i >= 1; i--)
	{
		/* Get the i'th monster */
		monster_type *m_ptr = &mon_list[i];

		/* Skip real monsters */
		if (m_ptr->r_idx) continue;

		/* Move last monster into open hole */
		compact_monsters_aux(mon_max - 1, i);

		/* Compress "mon_max" */
		mon_max--;
	}
}


/*
 * Delete/Remove all the monsters when the player leaves the level
 *
 * This is an efficient method of simulating multiple calls to the
 * "delete_monster()" function, with no visual effects.
 */
void wipe_mon_list(void)
{
	int i;

	/* Delete all the monsters */
	for (i = mon_max - 1; i >= 1; i--)
	{
		monster_type *m_ptr = &mon_list[i];

		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Mega-Hack -- preserve Uniques XXX XXX XXX */

		/* Hack -- Reduce the racial counter */
		r_ptr->cur_num--;

		/* Monster is gone */
		cave_m_idx[m_ptr->fy][m_ptr->fx] = 0;

		/* Wipe the Monster */
		(void)WIPE(m_ptr, monster_type);
	}

	/* Reset "mon_max" */
	mon_max = 1;

	/* Reset "mon_cnt" */
	mon_cnt = 0;

	/* Hack -- reset "reproducer" count */
	num_repro = 0;

	/* Hack -- no more target */
	target_set_monster(0);

	/* Hack -- no more tracking */
	health_track(0);

	/* no monster is holding the player */
	p_ptr->held_m_idx = 0;
	clear_timed(TMD_BEAR_HOLD);
}


/*
 * Get and return the index of a "free" monster.
 *
 * This routine should almost never fail, but it *can* happen.
 */
s16b mon_pop(void)
{
	int i;


	/* Normal allocation */
	if (mon_max < z_info->m_max)
	{
		/* Get the next hole */
		i = mon_max;

		/* Expand the array */
		mon_max++;

		/* Count monsters */
		mon_cnt++;

		/* Return the index */
		return (i);
	}


	/* Recycle dead monsters */
	for (i = 1; i < mon_max; i++)
	{
		monster_type *m_ptr;

		/* Get the monster */
		m_ptr = &mon_list[i];

		/* Skip live monsters */
		if (m_ptr->r_idx) continue;

		/* Count monsters */
		mon_cnt++;

		/* Use this monster */
		return (i);
	}


	/* Warn the player (except during dungeon creation) */
	if (character_dungeon) msg_print("Too many monsters!");

	/* Try not to crash */
	return (0);
}


/*
 * Apply a "monster restriction function" to the "monster allocation table"
 */
errr get_mon_num_prep(void)
{
	int i;

	/* Scan the allocation table */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Get the entry */
		alloc_entry *entry = &alloc_race_table[i];

		/* Accept monsters which pass the restriction, if any */
		if (!get_mon_num_hook || (*get_mon_num_hook)(entry->index))
		{
			/* Accept this monster */
			entry->prob2 = entry->prob1;
		}

		/* Do not use this monster */
		else
		{
			/* Decline this monster */
			entry->prob2 = 0;
		}
	}

	/* Success */
	return (0);
}


/* 
 * Choose an especially appropriate monster for the theme.
 * because certain monsters should be significantly more common
 * in their home.
 */
s16b get_mon_force_theme(int level)
{
	s16b choose;
	int die = randint(100);
	int randlevel;

	/* randomize level */
	randlevel = 5 + rand_int(90 + badluck - goodluck);
	if (rand_int(100) < 15) level = (level + randlevel)/2;

	/* randomize even more */
	if (die < 6) level -= randint(15);
	if (die > 95) level += randint(15);
	die = rand_int(100);

	switch (p_ptr->theme)
	{
		case 1: /* fairy forest */
		{
			if (die < 14)
			{
				if (level < 20) choose = 191; /* brownie(11) */
				else if (level < 85) choose = 401; /* wild unicorn(27) */
				else choose = 665; /* white unicorn(50) */
			}
			else if (die < 28)
			{
				if (level < 41) choose = 190; /* gnome(11) */
				else choose = 845; /* gnoyem (44, theme only) */
			}
			else if (die < 43)
			{
				if (level < 36) choose = 200; /* bright sprite(12) */
				else if (level < 49) choose = 598; /* trooping fairy(41) */
				else if (level < 59) choose = 650; /* dark fairy king(47) */
				else if (level < 65) choose = 598; /* trooping fairy(41) */
				else choose = 712; /* dark fairy fool(68) */
			}
			else if (die < 57)
			{
				if (level < 33) choose = 239; /* white gnome(15) */
				else if (level < 54) choose = 554; /* extril(39) */
				else choose = 680; /* dark fairy queen(56) */
			}
			else if (die < 71)
			{
				if (level < 13) choose = 63; /* sprite(3) */
				else if (level < 19) choose = 284; /* centaur barbarian(17) */
				else if (level < 30) choose = 285; /* centaur barbarian groups(22) */
				else choose = 458; /* centaur warrior(32) */
			}
			else if (die < 85)
			{
				if (level < 23)
				{
					int dieb = randint(100);
					if (dieb < 35) choose = 138; /* gnome thief(8) */
					else if (dieb < 70) choose = 139; /* clear sprite(8) */
					else choose = 151; /* leprechaun(9) */
				}
				else if (level < 72) choose = 395; /* pooka(27) */
				else choose = 723; /* rayem(69) */
			}
			else /* (die > 85) */
			{
				if (level < 17) choose = 110; /* poison sprite(6) */
				else if (level < 29) choose = 995; /* pixie (helper) */
				else if (level < 39) choose = 490; /* grey unicorn(34) */
				else choose = 604; /* black unicorn */
			}
		}
		case 2: /* cold forest */
		{
			if (die < 14)
			{
				if ((level < 15) && (randint(100) < 47)) choose = 32; /* kobold(2) */
				else if (level < 15) choose = 62; /* kobold shaman(3) */
				else if (level < 18) choose = 116; /* kobold lord(6) */
				else if (level < 29) choose = 320; /* ranger(20) */
				else choose = 442; /* frost giant(30) */
			}
			else if (die < 28)
			{
				if (level < 15) choose = 116; /* kobold lord(6) */
				else if (level < 30) choose = 283; /* forest troll(17) */
				else choose = 468; /* ice troll(33) */
			}
			else if (die < 42)
			{
				if (level < 25) choose = 154; /* yeti(9) */
				else if (level < 75) choose = 387; /* owlbear(26) */
				else choose = 718; /* great ice wyrm(63) */
			}
			else if (die < 57)
			{
				if (level < 29) choose = 182; /* white wolf(11) */
				else choose = 471; /* slow elemental(33) */
			}
			else if (die < 71)
			{
				if (level < 11) choose = 273; /* hill orc(8) */
				else if (level < 33) choose = 273; /* uruk(16) */
				else choose = 530; /* ice elemental(37) */
			}
			else if (die < 85)
			{
				if (level < 29) choose = 174; /* bugbear(10) */
				else choose = 460; /* windigo(32) */
			}
			else /* (die > 85) */
			{
				if (level < 65) choose = 318; /* sasquatch(20) */
				else choose = 570; /* wendigo(40) */
			}
		}
		case 3: /* icky place */
		{
			if (die < 14)
			{
				if (level < 26) choose = 835; /* bubbering icky thing(3, theme only) */
				else choose = 406; /* brown pudding(28) */
			}
			else if (die < 29)
			{
				if (level < 15) choose = 112; /* small slime(6) */
				else if (level < 27) choose = 266; /* giant slime blob(16) */
				else if (level < 85) choose = 403; /* wereworm(27) */
				else choose = choose = 730; /* great bile worm(67) */
			}
			else if (die < 43)
			{
				if (level < 16) choose = 836; /* bloodshot icky thing(9, theme only) */
				else if (level < 64) choose = 301; /* gelatinous cube(18) */
				else choose = 711; /* bile devil(61) */
			}
			else if (die < 55)
			{
				choose = 837; /* blue icky thing(16, theme only) */
			}
			else if (die < 71)
			{
				if (level < 39) choose = 201; /* slime blob(12) */
				else choose = 603; /* giant ameoba slime(42) */
			}
			else if (die < 85)
			{
				if (level < 36) choose = 254; /* grey ooze(15) */
				else choose = 535; /* black pudding(37) */
			}
			else /* (die > 85) */
			{
				if ((level < 9) && (randint(100) < 50)) choose = 59; /* photoplasm(3) */
				else if ((level < 10) && (randint(100) < 85)) choose = 119; /* rot jelly(6) */
				else if (level < 30) choose = 219; /* ochre jelly(13) */
				else choose = 470; /* ooze elemental(33) */
			}
		}
		case 4: /* volcano */
		{
			if (die < 14)
			{
				if ((level < 16) && (randint(100) < 51)) choose = 65; /* naga hatchling(3) */
				else if (level < 19) choose = 70; /* black naga(4) */
				else if (level < 30) choose = 358; /* giant firefly(24) */
				else if (level < 43) choose = 436; /* doombat(30) */
				else choose = 629; /* effretti(45) */
			}
			else if (die < 27)
			{
				if (level < 16) choose = 87; /* red worm mass(5) */
				else if (level < 37) choose = 348; /* fire vortex(23) */
				else choose = 538; /* plasma vortex(37) */
			}
			else if (die < 41)
			{
				if (level < 24) choose = 131; /* red naga(7) */
				else if (level < 60) choose = 449; /* golden naga(31) */
				else choose = 714; /* greater fire elemental(62) */
			}
			else if (die < 56)
			{
				if (level < 27) choose = 222; /* mordor orc(13) */
				else if (level < 38) choose = 438; /* ifrit(30) */
				else choose = 562; /* fire elemental(39) */
			}
			else if (die < 71)
			{
				if (level < 28) choose = 335; /* salamander(21) */
				else if (level < 36) choose = 441; /* chimaera(30) */
				else choose = 529; /* magma elemental(37) */
			}
			else if (die < 85)
			{
				if (level < 28) choose = 121; /* baby salamander(7) */
				else if (level < 34) choose = 335; /* salamander(21) */
				else if (level < 75) choose = 509; /* hellhound(35) */
				else choose = 767; /* hellhound in groups(78) */
			}
			else /* (die > 85) */
			{
				if (level < 30) choose = 337; /* red dragon bat(22) */
				else if (level < 81) choose = 478; /* fire giant(33) */
				else choose = 731; /* great hell wyrm(67) */
			}
		}
		case 5: /* earthy cave */
		{
			if (die < 15)
			{
				if (level < 25) choose = 68; /* rock lizard(4) */
				else if (level < 44) choose = 439; /* xreek(30) */
				else if (level < 105) choose = 628; /* xaren(45) */
				else choose = 796; /* giant xreek(105) */
			}
			else if (die < 29)
			{
				if (level < 17) choose = 117; /* well lizard(6) */
				else if (level < 30) choose = 310; /* zhelung(19) */
				else if (level < 42) choose = 456; /* blinking zhelung(32) */
				else choose = 605; /* ancient zhelung(43) */
			}
			else if (die < 43)
			{
				if (level < 25) choose = 240; /* lurker(14) */
				else if (level < 63) choose = 412; /* stone giant(28) */
				else choose = 717; /* greater earth elemental(62) */
			}
			else if (die < 57)
			{
				if (level < 14) choose = 93; /* rock mole(5) */
				else if (level < 36) choose = 271; /* umber hulk(16) */
				else choose = 572; /* xorn(40) */
			}
			else if (die < 71)
			{
				if (level < 25) choose = 278; /* merret(17) */
				else if (level < 35) choose = 279; /* merret in groups(30) */
				else choose = 565; /* earth elemental(39) */
			}
			else if (die < 85)
			{
				if (level < 31) choose = 245; /* rubble mimmic(18) */
				else if (level < 45) choose = 475; /* crystal drake(33) */
				else choose = 640; /* great crystal drake(49) */
			}
			else /* (die >= 85) */
			{
				if ((level < 34) || (level == 64)) choose = 309; /* stone golem(19) */
				else choose = 529; /* magma elemental(37) */
			}
		}
		case 6: /* windy cave */
		{
			if (die < 16)
			{
				if (level < 30) choose = 73; /* kestral(4) */
				else if (level < 75) choose = 480; /* invisible stalker(34) */
				else choose = 719; /* great storm wyrm(63) */
			}
			else if (die < 32)
			{
				if (level < 15) choose = 110; /* poison sprite(6) */
				else if ((level < 36) && (randint(100) < 50)) choose = 349; /* energy vortex(23) */
				else if (level < 36) choose = 347; /* cold vortex(23) */
				else if (level < 66) choose = 534; /* shardstorm(37) */
				else choose = 681; /* chaos vortex(53) */
			}
			else if (die < 48)
			{
				if (level < 35) choose = 141; /* nighthawk(8) */
				else if ((level >= 40) && (randint(100) < 70)) choose = 614; /* valkyrie(43) */
				else if (randint(100) < 51) choose = 537; /* nexus vortex(37) */
				else choose = 538; /* plasma vortex(37) */
			}
			else if (die < 65)
			{
				if ((level < 60) || (randint(100) < 8)) choose = 176; /* tengu(10) */
				else choose = 716; /* greater air elemental(62) */
			}
			else if (die < 82)
			{
				if (level < 38) choose = 185; /* dust vortex(11) */
				else if ((level < 72) && (randint(100) < 95-level)) choose = 575; /* cloud giant(40) */
				else choose = 627; /* storm giant(45) */
			}
			else
			{
				if (level < 34) choose = 113; /* fog cloud(6) */
				else if (level < 103) choose = 564; /* air elemental(39) */
				else choose = 716; /* greater air elemental(62) */
			}
		}
		case 7: /* full moon */
		{
			if (die < 14)
			{
				if (level < 19) choose = 120; /* raven(7) */
				else if (level < 43) choose = 327; /* shadow cat(21) */
				else choose = 616; /* wereraven(46) */
			}
			else if (die < 28)
			{
				if (level < 27) choose = 220; /* grim(13) */
				else if ((level < 32) && (randint(100) < 49)) choose = 403; /* wereworm(27) */
				else choose = 848; /* black stalker cat(33) */
			}
			else if (die < 42)
			{
				if (level < 38) choose = 233; /* wererat(14) */
				else choose = 843; /* weremumak(39, theme only) */
			}
			else if (die < 57)
			{
				if (level < 45) choose = 333; /* werewolf(22) */
				else if ((level < 55) && (randint(100) < 90-level)) choose = 333; /* werewolf(22) */
				else choose = 844; /* greater werewolf(50, theme only) */
			}
			else if (die < 71)
			{
				if ((level < 16) && (randint(100) > level)) choose = 40; /* alleycat(2) */
				else if ((level > 23) && (randint(100) < level+52)) choose = 373; /* werebear(25) */
				else choose = 275; /* black cat(16) */
			}
			else if (die < 85)
			{
				if (level < 16) choose = 46; /* novice rogue(2) */
				else if (level < 19) choose = 100; /* novice rogue groups(6) */
				else if (level < 30) choose = 354; /* lightning rogue(23) */
				else if ((level < 40) && (randint(100) < 95-(level-25))) choose = 459; /* assassin(32) */
				else choose = 508; /* master assassin(35) */
			}
			else
			{
				if (level < 15) choose = 72; /* clear cat(4) */
				else if (level < 31) choose = 281; /* imp(17) */
				else if (level < 46) choose = 489; /* phantom(34) */
				else choose = 631; /* doppleganger(45) */
			}
		}
		case 8: /* haunted castle */
		{
			if (die < 12)
			{
				if (level < 24) choose = 81; /* grey skeleton(4) */
				else if (level < 34) choose = 393; /* yellow skeleton(27) */
				else if (level < 55) choose = 510; /* lich(35) */
				else if (level < 70) choose = 685; /* demilich(54) */
				else choose = 726; /* archlich(65) */
			}
			else if (die < 24)
			{
				if ((level < 14) && (randint(100) < 108-(level*4))) choose = 172; /* goyley(9) */
				else if (level < 16) choose = 199; /* groyle(12) */
				else if (level < 46) choose = 287; /* guardian naga(17) */
				else choose = 659; /* gorgon(49) */
			}
			else if (die < 36)
			{
				if (level < 20) choose = 299; /* gargoyle(18) */
				else if ((level < 35) && (randint(100) < 150-(level*2))) choose = 300; /* gargoyle in groups(23) */
				else choose = 507; /* margoyle(35) */
			}
			else if (die < 48)
			{
				if (level < 30) choose = 353; /* gory ghost(23) */
				else if (level < 36) choose = 467; /* crypt wight(33) */
				else choose = 549; /* barrow wight(38) */
			}
			else if (die < 60)
			{
				if (level < 35) choose = 381; /* white wraith(26) */
				else if (level < 90) choose = 559; /* black wraith(39) */
				else choose = 586; /* nether wraith(40) */
			}
			else if (die < 72)
			{
				if (level < 40) choose = 402; /* vampire(27) */
				else if (level < 55) choose = 601; /* vampire lord(42) */
				else choose = 686; /* elder vampire(54) */
			}
			else if (die < 82)
			{
				if (level < 11) choose = 126; /* lost soul(7) */
				else if (level < 98) choose = 411; /* ghost(28) */
				else if (randint(100) < 55) choose = 706; /* armored death(60) */
				else choose = 703; /* skull druj(59) */
			}
			else if (die < 91)
			{
				if (level < 36) choose = 364; /* green knight(24) */
				else if (level < 87) choose = 557; /* death knight(39) */
				else choose = 737; /* titan(70) */
			}
			else
			{
				if (level < 16) choose = 234; /* scroll mimmic(14) */
				else if (level < 21) choose = 307; /* warhorse(19) */
				else if (level < 50) choose = 414; /* black knight(28) */
				else if (level < 115) choose = 692; /* lesser titan(56) */
				else choose = 766; /* greater titan(90) */
			}
		}
		case 9: /* swamp */
		{
			if (die < 14)
			{
				if (level < 17) choose = 211; /* horned bullfrog(11) */
				else if ((level < 80) && (randint(100) < 90-level)) choose = 326; /* warty bullfrog(21) */
				else if (level < 89) choose = 351; /* giant horned bullfrog(23) */
				else if (level < 92) choose = 763; /* baby behemoth(88) */
				else choose = 790; /* behemoth(101) */
			}
			else if (die < 28)
			{
				if (level < 24) choose = 217; /* large spotted frog(13) */
				else if (level < 41) choose = 420; /* giant python(27) */
				else choose = 841; /* marsh corpse(41, theme only) */
			}
			else if (die < 43)
			{
				if (level < 25) choose = 218; /* small null(13) */
				else if (level < 41) choose = 424; /* large null(28) */
				else choose = 624; /* giant null(44) */
			}
			else if (die < 57)
			{
				if (level < 45) choose = 241; /* crockodile(14) */
				else choose = 674; /* nulfraz(50) */
			}
			else if (die < 71)
			{
				if (level < 18) choose = 230; /* mosquito(14) */
				else if (level < 23) choose = 231; /* alternate mosquito(20) */
				else choose = 842; /* swamp bat(25, theme only) */
			}
			else if (die < 84)
			{
				if (level < 62) choose = 295; /* null(18) */
				else if (level < 64) choose = 563; /* water elemental(39) */
				else choose = 715; /* greater water elemental(62) */
			}
			else
			{
				if (level < 23) choose = 94; /* harpy(5) */
				else if (level < 86) choose = 421; /* hag(29) */
				else if (level < 118) choose = 765; /* young behemoth(90) */
				else choose = 804; /* giant behemoth(120) */
			}
		}
		case 10: /* dwarf mine */
		{
			if (die < 14)
			{
				if (level < 15) choose = 92; /* goblin(5) */
				else if ((level < 22) && (randint(100) < 11)) choose = 187; /* fire orc(11) */
				else if (level < 22) choose = 124; /* cave orc(7) */
				else if (level < 35) choose = 388; /* cave ogre(26) */
				else if (level < 43) choose = 536; /* rock troll(37) */
				else choose = 625; /* rock troll groups(44) */
			}
			else if (die < 28)
			{
				if (level < 10) choose = 118; /* creeping silver coins(6) */
				else if (level < 19) choose = 223; /* creeping cithril coins(13) */
				else if (level < 44) choose = 830; /* blood diamond(21) */
				else choose = 838; /* ironwood tree(48, theme only) */
			}
			else if (die < 43)
			{
				if (level < 14) choose = 206; /* black dwarf(12) */
				else if (level < 39) choose = 207; /* black dwarf groups(16) */
				else choose = 846; /* the summoning dark(41, theme only) */
			}
			else if (die < 57)
			{
				if (level < 28) choose = 215; /* mine mirage(13) */
				else if (level < 66) choose = 486; /* mithril golem(34) */
				else choose = 727; /* silver idol(66) */
			}
			else if (die < 71)
			{
				if (level < 48) choose = 237; /* mine vyrm(14) */
				else choose = 666; /* lesser balrog(50) */
			}
			else if (die < 85)
			{
				if (level < 29) choose = 833; /* black dwarf miner(18) */
				else choose = 839; /* grag high priest(32) */
			}
			else 
			{
				if (level < 8) choose = 60; /* cave vossar(3) */
				else if ((level < 22) || (randint(100) > level+65)) choose = 303; /* black dwarf priest(19) */
				else if (level < 89) choose = 350; /* black dwarf grag(24) */
				else choose = 756; /* greater balrog(79) */
			}
		}
		case 11: /* bug cave */
		{
			if (die < 14)
			{
				if (level < 9) choose = 33; /* centipede(2) */
				else if (level < 43) choose = 181; /* millipede(11) */
				else choose = 840; /* giant cockroach(46, theme only) */
			}
			else if (die < 28)
			{
				if (level < 21) choose = 106; /* giant gnat(6) */
				else if (level < 32) choose = 392; /* giant lightning bug(27) */
				else if (level < 64) choose = 466; /* giant lightning bug groups(33) */
				else choose = 741; /* gnawing bug(70) */
			}
			else if (die < 42)
			{
				if (level < 40) choose = 226; /* drider(13) */
				else choose = 609; /* drider of Achrya(43) */
			}
			else if (die < 56)
			{
				if (level < 10) choose = 130; /* dark elf(7) */
				else if (level < 36) choose = 204; /* dark elf priest(12) */
				else if (level < 49) choose = 592; /* priest of Achrya(41) */
				else choose = 654; /* high priest of Achrya(48) */
			}
			else if (die < 70)
			{
				if ((level < 14) || (randint(100) < 5)) choose = 179; /* giant spider(10) */
				else if (level < 45) choose = 255; /* giant tarantula(15) */
				else choose = 653; /* elder aranea(48) */
			}
			else if (die < 85)
			{
				if (level < 26) choose = 280; /* giant scorpion(17) */
				else if (randint(100) < 100-level) choose = 434; /* soldier ant(30) */
				else choose = 501; /* fire ant(35) */
			}
			else
			{
				if ((level < 15) && (randint(100) < level+2)) choose = 133; /* wolf spider(8) */
				else if (level < 15) choose = 149; /* nesting spider(9) */
				else if (level < 35) choose = 265; /* giant tick(16) */
				else choose = 548; /* aranea(38) */
			}
		}
		case 12: /* domain of the grepse */
		{
			if (die < 21)
			{
				if (level < 16) choose = 69; /* silver mouse(4) */
				else if ((level < 29) || (randint(100) < 4)) choose = 308; /* minidrake(19) */
				else if ((level < 49) || (randint(100) < 4)) choose = 457; /* limbo worm(32) */
				else choose = 661; /* wemu vyrm(49) */
			}
			else if (die < 39)
			{
				if (level < 19) choose = 105; /* clear eye(6) */
				else if (level < 36) choose = 345; /* soothing moss(23) */
				else if (level < 54) choose = 552; /* silver moss(38) */
				else if (level < 93) choose = 675; /* sleepy willow tree(55) */
				else choose = 769; /* disenchantree(73) */
			}
			else if (die < 60)
			{
				if (level < 31) choose = 171; /* silverworm(9) */
				else if ((level < 76) || (randint(100) < 105-level)) choose = 511; /* singing vyrm(35) */
				else choose = 679; /* grepse devil(60) */
			}
			else if (die < 80)
			{
				if (level < 33) choose = 214; /* mirage(13) */
				else if (level < 93) choose = 543; /* will o' the wisp(38) */
				else choose = 768; /* greater silver wemu(72) */
			}
			else
			{
				if ((level < 38) || (randint(100) < 130-(level*2))) choose = 269; /* vyrm(16) */
				else choose = 585; /* rastlig(45) */
			}
		}
		case 13: /* nightmare */
		{
			if (die < 13) 
			{
				if (rand_int(100) < 100-(level*5)) choose = 64; /* poltergeist(2) */
				else if ((level < 14) && (randint(100) < 85)) choose = 810; /* tan dust bunny(14) */
				else if ((level < 19) || (randint(100) < 40-level)) choose = 811; /* silver dust bunny(17) */
				else if (level < 41) choose = 812; /* clear dust bunny(19) */
				else choose = 623; /* headless horseman(45) */
			}
			else if (die < 28)
			{
				if (level < 38) choose = 197; /* greater poltergeist(11) */
				else if (randint(100) < (level/2)-21) choose = 695; /* dread in groups(49) */
				else choose = 553; /* dread(39) */
			}
			else if (die < 42)
			{
				if (level < 18) choose = 183; /* illusionist(11) */
				else if (level < 30) choose = 328; /* skull vendor(21) */
				else choose = 463; /* nightmare doctor(32) */
			}
			else if (die < 57)
			{
				if (level < 20) choose = 250; /* hooded shade(15) */
				else if (level < 39) choose = 251; /* hooded shade groups(25) */
				else choose = 611; /* screaming reaper(43) */
			}
			else if (die < 70)
			{
				if ((level < 40) && (randint(100) < 20)) choose = 264; /* elete winged monkey(18) */
				else if (level < 36) choose = 263; /* winged monkey(16) */
				else choose = 555; /* furie(41) */
			}
			else if (die < 83)
			{
				if (level < 36) choose = 344; /* skull wisp(23) */
				else choose = 566; /* night mare(39) */
			}
			else
			{
				if (level > 93) level = 93;
				if ((level < 70) || (randint(100) < 98-level)) choose = 386; /* bogeyman(26) */
				else choose = 743; /* horned reaper(72) */
			}
		}
		case 14: /* hell hall */
		{
			if (die < 16)
			{
				if (level < 21) choose = 115; /* manes(6) */
				else if (randint(100) < 39-level) choose = 384; /* small black reaper(26) */
				else if (level < 44) choose = 513; /* small black reaper groups(36) */
				else if (level < 76) choose = 648; /* marilith(47) */
				else choose = 755; /* pit fiend(77) */
			}
			else if (die < 33)
			{
				if ((level < 27) && (randint(100) < 77)) choose = 140; /* impsprite(8) */
				else if (level < 33) choose = 281; /* imp(17) */
				else if ((level < 53) || (randint(100) < 10)) choose = 593; /* hezrou(41) */
				else choose = 688; /* barbazu(55) */
			}
			else if (die < 49)
			{
				if (level < 31) choose = 144; /* lemure(9) */
				else if (level < 45) choose = 520; /* demonologist(36) */
				else choose = 660; /* horned devil(49) */
			}
			else if (die < 67)
			{
				if (level < 35) choose = 244; /* homunculus(15) */
				else if ((level < 63) || (randint(100) < 11)) choose = 573; /* vrock(40) */
				else choose = 713; /* horned devil lord(62) */
			}
			else if (die < 83)
			{
				if (level < 39) choose = 272; /* quasit(16) */
				else if ((level < 59) || (randint(100) < 11)) choose = 630; /* nalfeshnee(45) */
				else choose = 707; /* bone devil(60) */
			}
			else
			{
				if (level < 18) choose = 291; /* evil eye(18) */
				else if ((level < 51) || (randint(100) < 10)) choose = 518; /* bodak(36) */
				else choose = 673; /* barbed devil(52) */
			}
		}
	}
	return choose;
}


/*
 * Require monster appropriate for monster temple
 * (monster temples on themed levels are handled separately)
 */
static bool temple_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* monster temple monsters are supposed to be tough */
	if ((r_ptr->level < p_ptr->depth - 2) && (!(r_ptr->flags1 & (RF1_UNIQUE))))
		return (FALSE);
	else if (r_ptr->level < (p_ptr->depth * 3) / 4) return (FALSE);

	/* monster temple monsters */
	if (r_ptr->flags7 & RF7_TEMPLE) return (TRUE);

	/* assume not okay */
	return (FALSE);
}


/*
 * Return if a monster is appropriate for the current themed level
 * mode 1 has a chance to allow any monster.
 * mode 0 always forces appropriate monsters.
 */
bool theme_okay(int r_idx, int mode, bool vault)
{
	monster_race *r_ptr = &r_info[r_idx];
	int force;

	/* never allow L0 monsters in vaults */
	if ((vault) && (r_ptr->level == 0)) return FALSE;

	/* a monster temple is like a mini themed level */
	if ((!p_ptr->theme) && (r_ptr->flags7 & (RF7_THEME_ONLY)) &&
		(get_mon_num_hook == temple_okay)) return TRUE;
	/* theme-only monsters are never okay when there's no theme */
	else if ((!p_ptr->theme) && (r_ptr->flags7 & (RF7_THEME_ONLY))) return FALSE;
	/* all other monsters are okay when there's no theme */
	else if (!p_ptr->theme) return TRUE;

		/* theme monsters */
	/* the cold forest (yeti, windigo, other wild forest/hill stuff) */
	if ((p_ptr->theme == 1) && (r_ptr->flags7 & (RF7_CFOREST))) return TRUE;
	/* the fairy forest (light fairies, and some other forest stuff) */
	else if ((p_ptr->theme == 2) && (r_ptr->flags7 & (RF7_FFOREST))) return TRUE;
	/* the icky place (yuk) */
	else if ((p_ptr->theme == 3) && (r_ptr->flags7 & (RF7_ICKY_PLACE))) return TRUE;
	/* volcano (fire) */
	else if ((p_ptr->theme == 4) && (r_ptr->flags7 & (RF7_VOLCANO))) return TRUE;
	/* earth */
	else if ((p_ptr->theme == 5) && (r_ptr->flags7 & (RF7_EARTHY_CAVE))) return TRUE;
	/* air */
	else if ((p_ptr->theme == 6) && (r_ptr->flags7 & (RF7_WINDY_CAVE))) return TRUE;
	/* full moon (werebeasts, magic cats,..) */
	else if ((p_ptr->theme == 7) && (r_ptr->flags7 & (RF7_FULL_MOON))) return TRUE;
	/* haunted castle (undead, golems..) */
	else if ((p_ptr->theme == 8) && (r_ptr->flags7 & (RF7_CASTLE))) return TRUE;
	/* swamp (nulls, some other acid, some other water monsters) */
	else if ((p_ptr->theme == 9) && (r_ptr->flags7 & (RF7_SWAMP))) return TRUE;
	/* dwarf mine */
	else if ((p_ptr->theme == 10) && (r_ptr->flags7 & (RF7_DWARF_MINE))) return TRUE;
	/* bug cave */
	else if ((p_ptr->theme == 11) && (r_ptr->flags7 & (RF7_BUG_CAVE))) return TRUE;
	/* silver (grepse & certain others) */
	else if ((p_ptr->theme == 12) && (r_ptr->flags7 & (RF7_GREPSE))) return TRUE;
	/* nightmare */
	else if ((p_ptr->theme == 13) && (r_ptr->flags7 & (RF7_NIGHTMARE))) return TRUE;
	/* hell hall (demons & devils) */
	else if ((p_ptr->theme == 14) && (r_ptr->flags7 & (RF7_HELL_HALL))) return TRUE;

	/* Hacky: allow water elementals from the swamp to appear in the cold forest */
	if ((p_ptr->theme == 1) && (r_ptr->flags7 & (RF7_SWAMP)) &&
		((r_ptr->flags7 & (RF7_WATER_HIDE)) || (r_ptr->flags7 & (RF7_WATER_ONLY))) &&
		(strchr("X%", r_ptr->d_char))) return TRUE;

	/* THEME_ONLY always requires correct theme for its flag */
	if (r_ptr->flags7 & (RF7_THEME_ONLY)) return FALSE;

	if (mode == 1)
	{
			/* theme conflicts */
		/* volcano monsters never in cold forest */
		if ((p_ptr->theme == 1) && (r_ptr->flags7 & (RF7_VOLCANO))) return FALSE;
		/* cold forest monsters never in volcano */
		if ((p_ptr->theme == 4) && (r_ptr->flags7 & (RF7_CFOREST))) return FALSE;
		/* no demons or grepse in fairy forest */
		if ((p_ptr->theme == 2) && (r_ptr->flags3 & (RF3_DEMON))) return FALSE;
		if ((p_ptr->theme == 2) && (r_ptr->flags3 & (RF3_SILVER))) return FALSE;
		/* no creatures of light in the HELL_HALL */
		if ((p_ptr->theme == 14) && (r_ptr->flags3 & (RF3_HURT_DARK))) return FALSE;
		/* no light fairies in the domain of the grepse */
		if ((p_ptr->theme == 12) && (strchr("y", r_ptr->d_char))) return FALSE;

		/* don't accept if out of theme and more than 2 levels out of depth */
		/* (unless we are chosing a monster for a vault) */
		if ((!vault) && (r_ptr->level > p_ptr->depth + 2)) return FALSE;

		force = 14;
		/* (some themes have different odds to accept non-theme monsters) */
		if (p_ptr->theme == 10) force = 11; /* DWARF_MINE */
		/* if no conflicts, then doesn't always require correct theme flag */
		if (rand_int(100) < force) return TRUE;
	}

	/* don't accept */
	return FALSE;
}

/*
 * Choose a monster race that seems "appropriate" to the given level
 *
 * This function uses the "prob2" field of the "monster allocation table",
 * and various local information, to calculate the "prob3" field of the
 * same table, which is then used to choose an "appropriate" monster, in
 * a relatively efficient manner.
 *
 * Note that "town" monsters will *only* be created in the town, and
 * "normal" monsters will *never* be created in the town, unless the
 * "level" is "modified", for example, by polymorph or summoning.
 *
 * There is a small chance (1/50) of "boosting" the given depth by
 * a small amount (up to four levels), except in the town.
 *
 * It is (slightly) more likely to acquire a monster of the given level
 * than one of a lower level.  This is done by choosing several monsters
 * appropriate to the given level and keeping the "hardest" one.
 *
 * Note that if no monsters are "appropriate", then this function will
 * fail, and return zero, but this should *almost* never happen.
 */
s16b get_mon_num(int level, bool vault)
{
	int i, j, p, ood;
	int r_idx;
	long value, total;

	monster_race *r_ptr;

	alloc_entry *table = alloc_race_table;


	/* Boost the level */
	if (level > 0)
	{
		/* Occasional "nasty" monster */
		if (rand_int(NASTY_MON) == 0)
		{
			/* Pick a level bonus */
			int d = level / 4 + 2;

			/* Boost the level */
			level += ((d < 5) ? d : 5);
		}

		/* Occasional "nasty" monster */
		if (rand_int(NASTY_MON) == 0)
		{
			/* Pick a level bonus */
			int d = level / 4 + 2;

			/* Boost the level */
			level += ((d < 5) ? d : 5);
		}
	}


	/* Reset total */
	total = 0L;

	/* Process probabilities */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Monsters are sorted by depth (allow deeper monsters with a themed level) */
		if ((table[i].level > level + 10) && (p_ptr->theme)) break;
		/* Monsters are sorted by depth */
		if ((table[i].level > level) && (!p_ptr->theme)) break;

		/* mark this entry as out of depth */
		if (table[i].level > level) ood = table[i].level - level;
		else if (table[i].level > p_ptr->depth+4) ood = table[i].level - p_ptr->depth+2;
		else ood = 0;

		/* Default */
		table[i].prob3 = 0;


		/* Get the "r_idx" of the chosen monster */
		r_idx = table[i].index;

		/* Get the actual race */
		r_ptr = &r_info[r_idx];

		/* Hack -- No town monsters in dungeon */
		/* (except if appropriate to themed level to allow L0 trees in the forests) */
		if ((p_ptr->theme) && (theme_okay(r_idx, 0, vault))) /* okay */;
		else if ((level > 0) && (table[i].level <= 0)) continue;

		/* HELPER monsters can occationally be randomly generated on themed levels */
		/* don't allow HELPER monsters in vaults */
		if ((vault) && (r_ptr->flags3 & (RF3_HELPER))) continue;

		/* Hack -- "unique" monsters must be "unique" */
		if ((r_ptr->flags1 & (RF1_UNIQUE)) &&
		    (r_ptr->cur_num >= r_ptr->max_num))
		{
			continue;
		}

		/* Depth Monsters never appear out of depth */
		if ((r_ptr->flags1 & (RF1_FORCE_DEPTH)) && (r_ptr->level > p_ptr->depth))
		{
			continue;
		}

		/* Accept */
		table[i].prob3 = table[i].prob2;

		/* don't check themed level when there's a get_mon_num_hook */
		/* (but don't summon theme-only monsters) */
		if ((!get_mon_num_hook) || (r_ptr->flags7 & (RF7_THEME_ONLY)))
		{
			/* always common when appropriate for current themed level */
			/* prob is defined as 100/r_ptr->rarity in init2.c */
			/* so common monsters have a prob of 100(1) or 50(2) */
			if ((p_ptr->theme) && (theme_okay(r_idx, 0, vault)))
			{
				if (table[i].prob3 < 75) table[i].prob3 = 90 - (r_ptr->rarity*5);
				/* else prob3 is 100 */
			}
			/* don't allow if not appropriate */
			/* can't just say 'continue' because prob3 has already been set */
			else if (!theme_okay(r_idx, 1, vault)) table[i].prob3 = 0;
		}

		/* themed levels allow out of depth monsters much easier, */
		/* but don't make them too common (especially on early levels) */
		if ((ood) && (p_ptr->theme) && (table[i].prob3))
		{
			if (p_ptr->depth < 12) table[i].prob3 = table[i].prob3/3 - (ood+1)/2;
			else table[i].prob3 = table[i].prob3/2 - ood;

			if (table[i].prob3 < 5) table[i].prob3 = 5;
		}

		/* Total */
		total += table[i].prob3;
	}

	/* No legal monsters */
	if (total <= 0) return (0);


	/* Pick a monster */
	value = rand_int(total);

	/* Find the monster */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Found the entry */
		if (value < table[i].prob3) break;

		/* Decrement */
		value = value - table[i].prob3;
	}


	/* Power boost (less likely if on a themed level) */
	p = rand_int(100);
	if (p_ptr->theme) p = rand_int(160);

	/* Try for a "harder" monster once (50%) or twice (10%) */
	if (p < 60)
	{
		/* Save old */
		j = i;

		/* Pick a monster */
		value = rand_int(total);

		/* Find the monster */
		for (i = 0; i < alloc_race_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}

	/* Try for a "harder" monster twice (10%) */
	if (p < 10)
	{
		/* Save old */
		j = i;

		/* Pick a monster */
		value = rand_int(total);

		/* Find the monster */
		for (i = 0; i < alloc_race_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}


	/* Result */
	return (table[i].index);
}

/* get the general type of monster you heard */
/* (I don't know how I could turn this into a switch?) */
cptr get_hear_race(monster_race *r_ptr)
{
	if (strchr("A", r_ptr->d_char)) return "an ape";
	if (strchr("acFIS", r_ptr->d_char)) return "a bug";
	if (strchr("B", r_ptr->d_char)) return "a bird";
	if (strchr("b", r_ptr->d_char)) return "a bat";
	if (strchr("C", r_ptr->d_char)) return "a canine";
	if (strchr("dD", r_ptr->d_char)) return "a dragon";
	if (strchr("E", r_ptr->d_char)) return "a moving tree"; /* (NEVER_MOVE monsters are silent) */
	if (strchr("e", r_ptr->d_char)) return "an eye";
	if (strchr("f", r_ptr->d_char)) return "a feline (you must have insanely good ears)";
	if (strchr("GW", r_ptr->d_char)) return "something ghostly";
	if (strchr("g", r_ptr->d_char)) return "a golem";
	if (strchr("HMq", r_ptr->d_char)) return "a beast";
	if (strchr("h", r_ptr->d_char)) return "a humanoid monster";
	if (strchr("i", r_ptr->d_char)) return "a dark fairy or imp";
	if (strchr("J", r_ptr->d_char)) return "a snake";
	if (strchr("j", r_ptr->d_char)) return "a slime"; /* (most moving j's are slimes) */
	if (strchr("K", r_ptr->d_char)) return "a knight";
	if (strchr("k", r_ptr->d_char)) return "a kobold";
	if (strchr("L", r_ptr->d_char)) return "a lich";
	if (strchr("l", r_ptr->d_char)) return "a lizard";
	if (strchr("N", r_ptr->d_char)) return "a null";
	if (strchr("n", r_ptr->d_char)) return "a naga";
	if (strchr("O", r_ptr->d_char)) return "an ogre";
	if (strchr("o", r_ptr->d_char)) return "orcs"; /* almost never alone */
	if (strchr("P", r_ptr->d_char)) return "a giant";
	if (strchr("pt", r_ptr->d_char)) return "someone";
	if (strchr("R", r_ptr->d_char)) return "an amphibian";
	if (strchr("r", r_ptr->d_char)) return "a rodent";
	if (strchr("s", r_ptr->d_char)) return "a skeleton";
	if (strchr("T", r_ptr->d_char)) return "a troll";
	if (strchr("U", r_ptr->d_char)) return "a devil";
	if (strchr("u&", r_ptr->d_char)) return "a demon";
	if (strchr("V", r_ptr->d_char)) return "a vampire";
	if (strchr("w", r_ptr->d_char)) return "a worm";
	if (strchr("X%", r_ptr->d_char)) return "an elemental";
	if (strchr("x", r_ptr->d_char)) return "a gargoyle";
	if (strchr("Y", r_ptr->d_char)) return "the sound of hooves";
	if (strchr("y", r_ptr->d_char)) return "a fairy";
	if (strchr("Z", r_ptr->d_char)) return "a hound";
	if (strchr("z", r_ptr->d_char)) return "a zombie";
	/* m,$ and mimmics */
	return "an unknown monster";
}

/*
 * Display visible monsters in a window (the monster list)
 */
void display_monlist(void)
{
	int i, max;
	int line = 1, x = 0;
	int cur_x;
	unsigned total_count = 0, disp_count = 0;
	unsigned count_asleep = 0, count_normal = 0;
	unsigned count_snolos = 0, count_nolos = 0;
	bool uniq, messy;

	byte attr;

	char *m_name;
	char buf[80];
	monster_type *m_ptr;
	monster_race *r_ptr;

	u16b *race_count;
	u16b *race_count_nolos;
	u16b *race_count_snolos;
	u16b *race_count_asleep;
	int unknown = 0;
	int hearmon = 0;
	cptr heartype;


	/* Clear the term if in a subwindow, set x otherwise */
	if (Term != angband_term[0])
	{
		clear_from(0);
		max = Term->hgt - 1;
	}
	else
	{
		x = 13;
		max = Term->hgt - 2;
	}

	/* Allocate the array */
	C_MAKE(race_count, z_info->r_max, u16b);
	C_MAKE(race_count_nolos, z_info->r_max, u16b);
	C_MAKE(race_count_snolos, z_info->r_max, u16b);
	C_MAKE(race_count_asleep, z_info->r_max, u16b);

	uniq = FALSE;
	messy = FALSE;
	if (p_ptr->timed[TMD_IMAGE]) messy = TRUE;

	/* Scan the monster list */
	for (i = 1; i < mon_max; i++)
	{
		m_ptr = &mon_list[i];
		r_ptr = &r_info[m_ptr->r_idx];

		/* Only visible monsters */
		if ((!m_ptr->ml) && (!m_ptr->heard)) continue;

		/* ordinary trees aren't really monsters.. */
		if (m_ptr->r_idx == 834) continue;
		
        /* display flags should be correct even when hallucenating */
        if (m_ptr->csleep) m_ptr->display_sleep = TRUE;
        else m_ptr->display_sleep = FALSE;

		/* heard but not seen- can't tell monster race */
		if ((!m_ptr->ml) && (m_ptr->heard))
		{
			/* don't increment for more of the same race (if you show the race) */
			if ((unknown == 1) && (get_hear_race(r_ptr) == heartype)) /* */;
			else unknown++;
			hearmon++;

			/* get the race type for only the first heard unseen monster */
			if (unknown == 1) heartype = get_hear_race(r_ptr);
		}
        /* halucenation messes things up */
        else if ((messy) && (randint(40) < 30 + (badluck/2)))
        {
           int mess = randint(4);
           int friedx = randint(682) + 17;
           if (mess == 1)
           {
              race_count_asleep[friedx] += 1;
              count_asleep += 1;
           }
           else if (mess == 2)
           {
              race_count[friedx] += 1;
              count_normal += 1;
           }
           else if (mess == 3)
           {
              race_count_snolos[friedx] += 1;
              count_snolos += 1;
           }
           else if (mess == 4)
           {
              race_count_nolos[friedx] += 1;
              count_nolos += 1;
           }
		   /* sometimes see more monsters than there really are */
		   if (randint(100) < 9) i -= 1;
        }
		/* Bump the count for this race, and the total count */
		/* separate race count for los / no los / asleep */
        else if ((!(m_ptr->mflag & (MFLAG_VIEW))) && (m_ptr->csleep))
	    {
            race_count_snolos[m_ptr->r_idx]++;
            count_snolos++;
        }
        else if (!(m_ptr->mflag & (MFLAG_VIEW)))
	    {
            race_count_nolos[m_ptr->r_idx]++;
            count_nolos++;
        }
        else if (m_ptr->csleep)
        {
            race_count_asleep[m_ptr->r_idx]++;
            count_asleep++;
        }
        else
        {
            race_count[m_ptr->r_idx]++;
            count_normal++;
        }
        if ((r_ptr->flags1 & RF1_UNIQUE) && (!messy)) uniq = TRUE;

		total_count++;
	}

	/* Note no visible monsters */
	if (!total_count)
	{
		/* Clear display and print note */
		c_prt(TERM_SLATE, "You see no monsters.", 0, 0);
		if (Term == angband_term[0])
		    Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");

		/* Free up memory */
		FREE(race_count);
		FREE(race_count_asleep);
		FREE(race_count_nolos);
		FREE(race_count_snolos);

		/* Done */
		return;
	}

    /* if (count_asleep) */
	if (((count_asleep) || (count_snolos) || (count_nolos)) && (!messy))
    {
		/* Reset position */
		cur_x = x;

        /* my_strcpy(buf, "  The following monsters are in sight but aren't aware of you:", sizeof(buf)); */
        my_strcpy(buf, " (darker shade means unaware of you, blue means out of line of sight) ", sizeof(buf));

        /* Print and bump line counter */
		c_prt(TERM_L_WHITE, buf, line, cur_x);
		line++;
    }

    if ((uniq) && (!messy))
    {
		/* Reset position */
		cur_x = x;

        my_strcpy(buf, " (uniques in red if in sight, or brown if out of LOS) ", sizeof(buf));

        /* Print and bump line counter */
		c_prt(TERM_L_WHITE, buf, line, cur_x);
		line++;
    }

	/* Go over (line of sight and awake) */
	/* for (i = 1; (i < z_info->r_max) && (line < max); i++) */
	/* sort from highest to lowest */
	for (i = z_info->r_max-1; (i > 0) && (line < max); i--)
	{
		/* Reset position */
		cur_x = x;

        /* no monsters in los who are aware of you */
        if (!count_normal) break;
        
		/* No monsters of this race are visible (and awake) */
		if (!race_count[i]) continue;

		/* Note that these have been displayed */
		disp_count += race_count[i];

		/* Get monster race and name */
		r_ptr = &r_info[i];
		m_name = r_name + r_ptr->name;

		/* Display uniques in a special colour */
		if (r_ptr->flags1 & RF1_UNIQUE)
			attr = TERM_L_RED;
		else
			attr = TERM_WHITE;

		/* Build the monster name */
		if (race_count[i] == 1)
			my_strcpy(buf, m_name, sizeof(buf));
		else
			strnfmt(buf, sizeof(buf), "%s (x%d) ", m_name, race_count[i]);

		/* Display the pict */
		Term_putch(cur_x++, line, r_ptr->x_attr, r_ptr->x_char);
		if (use_bigtile) Term_putch(cur_x++, line, 255, -1);
		Term_putch(cur_x++, line, TERM_WHITE, ' ');

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;
	}
        
	/* Go over (line of sight and unaware of the player) */
	/* for (i = 1; (i < z_info->r_max) && (line < max); i++) */
	/* sort from highest to lowest */
	for (i = z_info->r_max-1; (i > 0) && (line < max); i--)
	{
		/* Reset position */
		cur_x = x;

        /* no sleeping (or roaming) monsters */
        if (!count_asleep) break;
        
        /* No monsters of this race are visible and unaware */
		if (!race_count_asleep[i]) continue;

		/* Note that these have been displayed */
		disp_count += race_count_asleep[i];

		/* Get monster race and name */
		r_ptr = &r_info[i];
		m_name = r_name + r_ptr->name;

		/* Display uniques in a special colour */
		if (r_ptr->flags1 & RF1_UNIQUE)
			attr = TERM_RED;
		else
			attr = TERM_L_WHITE;

		/* Build the monster name */
		if (race_count_asleep[i] == 1)
			my_strcpy(buf, m_name, sizeof(buf));
		else
			strnfmt(buf, sizeof(buf), "%s (x%d) ", m_name, race_count_asleep[i]);

		/* Display the pict */
		Term_putch(cur_x++, line, r_ptr->x_attr, r_ptr->x_char);
		if (use_bigtile) Term_putch(cur_x++, line, 255, -1);
		Term_putch(cur_x++, line, TERM_WHITE, ' ');

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;
	}

/*#if removed        
    if ((count_nolos) || (count_snolos))
    {
        my_strcpy(buf, "  The following monsters are not in line of sight:", sizeof(buf));

        /* Print and bump line counter *
		c_prt(TERM_L_BLUE, buf, line, cur_x);
		line++;
    }
#endif */

	/* Go over (for not in LOS monsters) */
	/* for (i = 1; (i < z_info->r_max) && (line < max); i++) */
	/* sort from highest to lowest */
	for (i = z_info->r_max-1; (i > 0) && (line < max); i--)
	{
		/* Reset position */
		cur_x = x;

        /* no monsters out of LOS and aware */
        if (!count_nolos) break;
        
		/* No monsters of this race are visible */
		if (!race_count_nolos[i]) continue;

		/* Note that these have been displayed */
		disp_count += race_count_nolos[i];

		/* Get monster race and name */
		r_ptr = &r_info[i];
		m_name = r_name + r_ptr->name;

		/* Display uniques in a special colour */
		if (r_ptr->flags1 & RF1_UNIQUE)
			attr = TERM_L_UMBER;
		else
			attr = TERM_L_BLUE;

		/* Build the monster name */
		if (race_count_nolos[i] == 1)
			my_strcpy(buf, m_name, sizeof(buf));
		else
			strnfmt(buf, sizeof(buf), "%s (x%d) ", m_name, race_count_nolos[i]);

		/* Display the pict */
		Term_putch(cur_x++, line, r_ptr->x_attr, r_ptr->x_char);
		if (use_bigtile) Term_putch(cur_x++, line, 255, -1);
		Term_putch(cur_x++, line, TERM_WHITE, ' ');

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;
	}

	/* Go over (for out of LOS and not aware) */
	/* for (i = 1; (i < z_info->r_max) && (line < max); i++) */
	/* sort from highest to lowest */
	for (i = z_info->r_max-1; (i > 0) && (line < max); i--)
	{
		/* Reset position */
		cur_x = x;

        /* no monsters out of LOS and unaware */
        if (!count_snolos) break;

        /* no message */
        
		/* No monsters of this race are visible */
		if (!race_count_snolos[i]) continue;

		/* Note that these have been displayed */
		disp_count += race_count_snolos[i];

		/* Get monster race and name */
		r_ptr = &r_info[i];
		m_name = r_name + r_ptr->name;

		/* Display uniques in a special colour */
		if (r_ptr->flags1 & RF1_UNIQUE)
			attr = TERM_UMBER;
		else
			attr = TERM_BLUE;
		

		/* Build the monster name */
		if (race_count_snolos[i] == 1)
			my_strcpy(buf, m_name, sizeof(buf));
		else
			strnfmt(buf, sizeof(buf), "%s (x%d) ", m_name, race_count_snolos[i]);

		/* Display the pict */
		Term_putch(cur_x++, line, r_ptr->x_attr, r_ptr->x_char);
		if (use_bigtile) Term_putch(cur_x++, line, 255, -1);
		Term_putch(cur_x++, line, TERM_WHITE, ' ');

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;
	}

	/* can hear unseen monsters */
	if (unknown)
	{
		if (unknown == 1) strnfmt(buf, sizeof buf, " and you hear %s out of sight.", heartype);
		else strnfmt(buf, sizeof buf, " and you hear %d unknown monsters.", unknown);

		/* don't print "... and X others" for monsters you hear */
		total_count = total_count - hearmon;

        /* Print and bump line counter */
		c_prt(TERM_L_BLUE, buf, line, cur_x);
		line++;
	}

	/* Print "and others" message if we've run out of space */
	if (disp_count != total_count)
	{
		strnfmt(buf, sizeof buf, "  ...and %d others.", total_count - disp_count);
		c_prt(TERM_WHITE, buf, line, x);
	}

	/* Otherwise clear a line at the end, for main-term display */
	else
	{
		prt("", line, x);
	}

	/* Message */
	prt(format("You can see %d monster%s:",
	           total_count, (total_count == 1 ? "" : "s")), 0, 0);

	if (Term == angband_term[0])
		Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");

	/* Free the race counters */
	FREE(race_count);
	FREE(race_count_asleep);
	FREE(race_count_nolos);
	FREE(race_count_snolos);
}

/*
 * Build a string describing a monster in some way.
 *
 * We can correctly describe monsters based on their visibility.
 * We can force all monsters to be treated as visible or invisible.
 * We can build nominatives, objectives, possessives, or reflexives.
 * We can selectively pronominalize hidden, visible, or all monsters.
 * We can use definite or indefinite descriptions for hidden monsters.
 * We can use definite or indefinite descriptions for visible monsters.
 *
 * Pronominalization involves the gender whenever possible and allowed,
 * so that by cleverly requesting pronominalization / visibility, you
 * can get messages like "You hit someone.  She screams in agony!".
 *
 * Reflexives are acquired by requesting Objective plus Possessive.
 *
 * I am assuming that no monster name is more than 65 characters long,
 * so that "char desc[80];" is sufficiently large for any result, even
 * when the "offscreen" notation is added.
 *
 * Note that the "possessive" for certain unique monsters will look
 * really silly, as in "Morgoth, King of Darkness's".  We should
 * perhaps add a flag to "remove" any "descriptives" in the name.
 *
 * Note that "offscreen" monsters will get a special "(offscreen)"
 * notation in their name if they are visible but offscreen.  This
 * may look silly with possessives, as in "the rat's (offscreen)".
 * Perhaps the "offscreen" descriptor should be abbreviated.
 *
 * Mode Flags:
 *   0x01 --> Objective (or Reflexive)
 *   0x02 --> Possessive (or Reflexive)
 *   0x04 --> Use indefinites for hidden monsters ("something")
 *   0x08 --> Use indefinites for visible monsters ("a kobold")
 *   0x10 --> Pronominalize hidden monsters
 *   0x20 --> Pronominalize visible monsters
 *   0x40 --> Assume the monster is hidden
 *   0x80 --> Assume the monster is visible
 *
 * Useful Modes:
 *   0x00 --> Full nominative name ("the kobold") or "it"
 *   0x04 --> Full nominative name ("the kobold") or "something"
 *   0x80 --> Banishment resistance name ("the kobold")
 *   0x88 --> Killing name ("a kobold")
 *   0x22 --> Possessive, genderized if visable ("his") or "its"
 *   0x23 --> Reflexive, genderized if visable ("himself") or "itself"
 */
void monster_desc(char *desc, size_t max, const monster_type *m_ptr, int mode)
{
	cptr res;
	cptr name;
	bool seen, pron;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	
	/* Hallucenation */
    if ((p_ptr->timed[TMD_IMAGE]) && (randint(40) < 35 + ((badluck+3)/4)))
	{
       r_ptr = &r_info[randint(682) + 17];
    }

	name = (r_name + r_ptr->name);


	/* Can we "see" it (forced, or not hidden + visible) */
	seen = ((mode & (0x80)) || (!(mode & (0x40)) && m_ptr->ml));

	/* Sexed Pronouns (seen and forced, or unseen and allowed) */
	pron = ((seen && (mode & (0x20))) || (!seen && (mode & (0x10))));


	/* First, try using pronouns, or describing hidden monsters */
	if (!seen || pron)
	{
		/* an encoding of the monster "sex" */
		int kind = 0x00;

		/* Extract the gender (if applicable) */
		if (r_ptr->flags1 & (RF1_FEMALE)) kind = 0x20;
		else if (r_ptr->flags1 & (RF1_MALE)) kind = 0x10;

		/* Ignore the gender (if desired) */
		if (!m_ptr || !pron) kind = 0x00;


		/* Assume simple result */
		res = "it";

		/* Brute force: split on the possibilities */
		switch (kind + (mode & 0x07))
		{
			/* Neuter, or unknown */
			case 0x00: res = "it"; break;
			case 0x01: res = "it"; break;
			case 0x02: res = "its"; break;
			case 0x03: res = "itself"; break;
			case 0x04: res = "something"; break;
			case 0x05: res = "something"; break;
			case 0x06: res = "something's"; break;
			case 0x07: res = "itself"; break;

			/* Male (assume human if vague) */
			case 0x10: res = "he"; break;
			case 0x11: res = "him"; break;
			case 0x12: res = "his"; break;
			case 0x13: res = "himself"; break;
			case 0x14: res = "someone"; break;
			case 0x15: res = "someone"; break;
			case 0x16: res = "someone's"; break;
			case 0x17: res = "himself"; break;

			/* Female (assume human if vague) */
			case 0x20: res = "she"; break;
			case 0x21: res = "her"; break;
			case 0x22: res = "her"; break;
			case 0x23: res = "herself"; break;
			case 0x24: res = "someone"; break;
			case 0x25: res = "someone"; break;
			case 0x26: res = "someone's"; break;
			case 0x27: res = "herself"; break;
		}

		/* Copy the result */
		my_strcpy(desc, res, max);
	}


	/* Handle visible monsters, "reflexive" request */
	else if ((mode & 0x02) && (mode & 0x01))
	{
		/* The monster is visible, so use its gender */
		if (r_ptr->flags1 & (RF1_FEMALE)) my_strcpy(desc, "herself", max);
		else if (r_ptr->flags1 & (RF1_MALE)) my_strcpy(desc, "himself", max);
		else my_strcpy(desc, "itself", max);
	}


	/* Handle all other visible monster requests */
	else
	{
		/* It could be a Unique */
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			/* Start with the name (thus nominative and objective) */
			my_strcpy(desc, name, max);
		}

		/* It could be an indefinite monster */
		else if (mode & 0x08)
		{
			/* XXX Check plurality for "some" */

			/* Indefinite monsters need an indefinite article */
			my_strcpy(desc, is_a_vowel(name[0]) ? "an " : "a ", max);
			my_strcat(desc, name, max);
		}

		/* It could be a normal, definite, monster */
		else
		{
			/* Definite monsters need a definite article */
			my_strcpy(desc, "the ", max);
			my_strcat(desc, name, max);
		}

		/* Handle the Possessive as a special afterthought */
		if (mode & 0x02)
		{
			/* XXX Check for trailing "s" */

			/* Simply append "apostrophe" and "s" */
			my_strcat(desc, "'s", max);
		}

		/* Mention "offscreen" monsters XXX XXX */
		if (!panel_contains(m_ptr->fy, m_ptr->fx))
		{
			/* Append special notation */
			my_strcat(desc, " (offscreen)", max);
		}
	}
}



/*
 * Learn about a monster (by "probing" it)
 */
void lore_do_probe(int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];


	/* Hack -- Memorize some flags, DJA: changed to include spells */
	l_ptr->flags1 = r_ptr->flags1;
	l_ptr->flags2 = r_ptr->flags2;
	l_ptr->flags3 = r_ptr->flags3;
	l_ptr->flags4 = r_ptr->flags4; /* breath / projectiles */
	/* spells */
	if (randint(100) < 34) l_ptr->flags5 = r_ptr->flags5;
	if (randint(100) < 34) l_ptr->flags6 = r_ptr->flags6;

	/* this monster race has been probed (reveals more info in recall) */
	l_ptr->xtra1 = 1;

	/* Update monster recall window */
	if (p_ptr->monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}
}


/*
 * Take note that the given monster just dropped some treasure
 *
 * Note that learning the "GOOD"/"GREAT" flags gives information
 * about the treasure (even when the monster is killed for the first
 * time, such as uniques, and the treasure has not been examined yet).
 *
 * This "indirect" method is used to prevent the player from learning
 * exactly how much treasure a monster can drop from observing only
 * a single example of a drop.  This method actually observes how much
 * gold and items are dropped, and remembers that information to be
 * described later by the monster recall code.
 */
void lore_treasure(int m_idx, int num_item, int num_gold)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];


	/* Note the number of things dropped */
	if (num_item > l_ptr->drop_item) l_ptr->drop_item = num_item;
	if (num_gold > l_ptr->drop_gold) l_ptr->drop_gold = num_gold;

	/* Hack -- memorize the good/great flags */
	if (r_ptr->flags1 & (RF1_DROP_GOOD)) l_ptr->flags1 |= (RF1_DROP_GOOD);
	if (r_ptr->flags1 & (RF1_DROP_GREAT)) l_ptr->flags1 |= (RF1_DROP_GREAT);

	/* Update monster recall window */
	if (p_ptr->monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}
}



/*
 * This function updates the monster record of the given monster
 *
 * This involves extracting the distance to the player (if requested),
 * and then checking for visibility (natural, stealth, see-invis,
 * telepathy), updating the monster visibility flag, redrawing (or
 * erasing) the monster when its visibility changes, and taking note
 * of any interesting monster flags (cold-blooded, invisible, etc).
 *
 * Note the new "mflag" field which encodes several monster state flags,
 * including "view" for when the monster is currently in line of sight,
 * and "mark" for when the monster is currently visible via detection.
 *
 * The only monster fields that are changed here are "cdis" (the
 * distance from the player), "ml" (visible to the player), and
 * "mflag" (to maintain the "MFLAG_VIEW" flag).
 *
 * Note the special "update_monsters()" function which can be used to
 * call this function once for every monster.
 *
 * Note the "full" flag which requests that the "cdis" field be updated,
 * this is only needed when the monster (or the player) has moved.
 *
 * Every time a monster moves, we must call this function for that
 * monster, and update the distance, and the visibility.  Every time
 * the player moves, we must call this function for every monster, and
 * update the distance, and the visibility.  Whenever the player "state"
 * changes in certain ways ("blindness", "telepathy",
 * and "see invisible"), we must call this function for every monster,
 * and update the visibility.
 *
 * Routines that change the "illumination" of a grid must also call this
 * function for any monster in that grid, since the "visibility" of some
 * monsters may be based on the illumination of their grid.
 *
 * Note that this function is called once per monster every time the
 * player moves.  When the player is running, this function is one
 * of the primary bottlenecks, along with "update_view()" and the
 * "process_monsters()" code, so efficiency is important.
 *
 * Note the optimized "inline" version of the "distance()" function.
 *
 * A monster is "visible" to the player if (1) it has been detected
 * by the player, (2) it is close to the player and the player has
 * telepathy, or (3) it is close to the player, and in line of sight
 * of the player, and it is "illuminated" by some combination of
 * torch light, or permanent light (invisible monsters
 * are only affected by "light" if the player can see invisible).
 *
 * Stealthy monsters who are visible and in line of sight can still 
 * escape notice.
 *
 * Monsters which are not on the current panel may be "visible" to
 * the player, and their descriptions will include an "offscreen"
 * reference.  Currently, offscreen monsters cannot be targetted
 * or viewed directly, but old targets will remain set.  XXX XXX
 *
 * The player can choose to be disturbed by several things, including
 * "disturb_move" (monster which is viewable moves in some way), and
 * "disturb_near" (monster which is "easily" viewable moves in some
 * way).  Note that "moves" includes "appears" and "disappears".
 *
 * DJA: "disturb_near" now only triggers when the monster is aware of you.
 * (or when you're running and about to run into a monster)
 */
void update_mon(int m_idx, bool full)
{
	bool do_invisible = FALSE;
	bool darkvs, msilent, lore, nevermove;
	int hearcheck, discernmod, d;
	u32b f4, f5, f6;

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	/* Current location */
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	/* Seen at all */
	bool flag = FALSE;

	/* Seen by vision */
	bool easy = FALSE;

	int py = p_ptr->py;
	int px = p_ptr->px;

	/* Compute distance */
	if (full)
	{
		/* Distance components */
		int dy = (py > fy) ? (py - fy) : (fy - py);
		int dx = (px > fx) ? (px - fx) : (fx - px);

		/* Approximate distance */
		d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

		/* Restrict distance */
		if (d > 255) d = 255;

		/* Save the distance */
		m_ptr->cdis = d;
	}

	/* Extract distance */
	else
	{
		/* Extract the distance */
		d = m_ptr->cdis;
	}

	/* paranoia */
	if ((p_ptr->timed[TMD_BEAR_HOLD]) && (p_ptr->held_m_idx == m_idx))
	{
		/* prevent non-adjacent monsters holding the player */
		if (m_ptr->cdis > 2)
		{
			p_ptr->held_m_idx = 0;
			clear_timed(TMD_BEAR_HOLD);
		}
	}

	/* Extract the racial spell flags to check for certain things */
	f4 = r_ptr->flags4;
	f5 = r_ptr->flags5;
	f6 = r_ptr->flags6;

	/* Detected */
	if (m_ptr->mflag & (MFLAG_MARK)) flag = TRUE;


	/* Nearby */
	if (d <= MAX_SIGHT)
	{
		/* Basic telepathy */
		if (p_ptr->telepathy)
		{
			/* Empty mind, no telepathy */
			if (r_ptr->flags2 & (RF2_EMPTY_MIND))
			{
				/* Memorize flags */
				l_ptr->flags2 |= (RF2_EMPTY_MIND);
			}

			/* Weird mind, occasional telepathy */
			else if (r_ptr->flags2 & (RF2_WEIRD_MIND))
			{
				/* One in ten individuals are detectable */
				/* or any detectable rarely */
				if (((m_idx % 10) == 5) || (rand_int(100) < 3 + (goodluck/2)))
				{
					/* Detectable */
					flag = TRUE;
					
					/* monster moved into esp range */
					if ((!m_ptr->ml) && (disturb_espmove)) disturb(1, 0);

					/* Memorize mental flags */
					l_ptr->flags2 |= (RF2_WEIRD_MIND);
					if (r_ptr->flags2 & (RF2_SMART)) l_ptr->flags2 |= (RF2_SMART);
					if (r_ptr->flags2 & (RF2_STUPID)) l_ptr->flags2 |= (RF2_STUPID);
				}
			}

			/* Normal mind, allow telepathy */
			else
			{
				/* Detectable */
				flag = TRUE;
					
				/* monster moved into esp range */
				if ((!m_ptr->ml) && (disturb_espmove)) disturb(1, 0);

				/* Hack -- Memorize mental flags */
				if (r_ptr->flags2 & (RF2_SMART)) l_ptr->flags2 |= (RF2_SMART);
				if (r_ptr->flags2 & (RF2_STUPID)) l_ptr->flags2 |= (RF2_STUPID);
			}
		}

		/* Normal line of sight, and not blind */
		if (player_has_los_bold(fy, fx) && !p_ptr->timed[TMD_BLIND])
		{
			/* Use "illumination" */
			if (player_can_see_bold(fy, fx))
			{
				/* Handle "invisible" monsters */
				if ((r_ptr->flags2 & (RF2_INVISIBLE)) || (m_ptr->tinvis))
				{
					/* Take note */
					do_invisible = TRUE;

					/* See invisible */
					if (p_ptr->see_inv)
					{
						/* Easy to see */
						easy = TRUE;
					}
				}

				/* Handle "normal" monsters */
				else
				{
					/* Easy to see */
					easy = TRUE;
				}
			}

			/* Visible */
			if ((flag) || (easy))
			{
				/* Memorize flags */
				if ((do_invisible) && (!m_ptr->tinvis)) l_ptr->flags2 |= (RF2_INVISIBLE);
			}
		}
	}

	/* Temporary invisibility decrements in process_monster() */
	
	/* make sure all invisible monsters are marked as invisible for darkvision check */
	if ((r_ptr->flags2 & (RF2_INVISIBLE)) || (m_ptr->tinvis)) do_invisible = TRUE;

    /* Darkvision: If monster is in line of sight, the player is not blind, */ 
    /* and the monster is not invisible, then the player can see it */
    /* (even if the space is not lit) */
    /* (should darkvision let you see walls without light?) */
	darkvs = FALSE;
    if (((p_ptr->darkvis) || (p_ptr->timed[TMD_DARKVIS])) &&
	   (player_has_los_bold(fy, fx)) && (!p_ptr->timed[TMD_BLIND]))
	{   
		if ((p_ptr->see_inv) || (!do_invisible))
		{
			easy = TRUE;
			darkvs = TRUE;
		}
    }

	/* if a monster would otherwise be seen easily, check monster stealth */
	if (easy)
	{
        int mstealth = r_ptr->stealth;
		/* WATER_HIDE monsters get stealth bonus when in water */
		if (((r_ptr->flags7 & (RF7_WATER_HIDE)) || (r_ptr->flags7 & (RF7_WATER_ONLY))) &&
			(cave_feat[fy][fx] == FEAT_WATER))
		{
			if (mstealth < 3) mstealth += 2;
			else if (mstealth == 3) mstealth += randint(2);
		}
		/* WATER_ONLY monsters can't hide out of water */
		else if ((r_ptr->flags7 & (RF7_WATER_ONLY)) && (mstealth > 1))
		{
			if (mstealth < 5) mstealth = 1;
			else mstealth -= 3;
		}
		/* earth elementals get stealth bonus when in a wall */
		if ((strchr("X%", r_ptr->d_char)) && (r_ptr->flags2 & (RF2_PASS_WALL)) &&
			(!cave_floor_bold(fy, fx)))
		{
			if (mstealth < 3) mstealth += 1 + randint(2);
			else if (mstealth < 4)  mstealth += randint(2);
		}

		/* cap & scale up */
		if (mstealth > 6) mstealth = 6;
		mstealth = (mstealth * 11) + 1;
		/* luck factor */
        if (goodluck > 5) mstealth -= (goodluck/2);
        else if (goodluck > 3) mstealth -= 1;
        if (badluck > 3) mstealth += (badluck/2);
        
		/* distance factor */
        if ((r_ptr->stealth > 1) && (d < 15)) mstealth += d * 2;
        if ((d > 14) && (r_ptr->stealth > 2)) mstealth += (r_ptr->stealth*10);
        else if ((d > 14) && (r_ptr->stealth == 2)) mstealth += 30;
        else if (d > 14) mstealth += 16;
        else if ((d > 3) && (r_ptr->stealth > 3)) mstealth += (d-3) * (r_ptr->stealth - 2);
        else if (d > 3) mstealth += (d-3);
        else if (d < 2) mstealth -= 16;

        if ((m_ptr->stunned) || (m_ptr->confused) || (m_ptr->charmed)) mstealth -= 16;
        if (p_ptr->depth == 0) mstealth -= 16;
                
        /* monster hasn't been noticed yet (it is still hiding) */
        /* darkvision makes it hard to hide in the shadows */
        /* this helps dwarves who have darkvision but horrible alertness */
        if ((!m_ptr->monseen) && (r_ptr->stealth > 2) && (!darkvs)) mstealth += 20;
		else if ((!m_ptr->monseen) && (r_ptr->stealth > 2)) mstealth += 8;

        /* much easier to notice if you've noticed it before */
        if ((m_ptr->monseen > 3) && (d < 5) && (!m_ptr->monfear)) mstealth -= m_ptr->monseen * 15;
        else if ((m_ptr->monseen > 0) && (d < 7) && (!m_ptr->monfear)) mstealth -= 60;
        else if (m_ptr->monseen > 2) mstealth -= m_ptr->monseen * 4;
        else if ((m_ptr->monseen > 0) || (m_ptr->heard)) mstealth -= 12;
        
        /* sleeping monsters can't actively hide */
        if ((m_ptr->csleep) && (!m_ptr->roaming)) mstealth -= 12;

        /* NEVER_MOVE monsters can't try to hide once they've been noticed */
        if ((m_ptr->monseen > 0) && (r_ptr->flags1 & (RF1_NEVER_MOVE))) mstealth -= 20;

#if irrevelentnow
        /* always visible if it just hit you or you hit it */
        /* may be irrevelent now that you always notice adjacent monsters */
        if (m_ptr->monseen > 9)
        {
           m_ptr->monseen -= 1;
           mstealth = 1;
        }
#endif
        /* prevent randint(negative mstealth) because that makes it crash */
        if (mstealth < 1) mstealth = 1;

        /* examples which assume monseen is 0 and luck is 0: */
        /* monster of stealth 1 at a distance of 8 now has mstealth of 16 */
        /* monster of stealth 1 at a distance of 15 now has mstealth of 27 */
        /* monster of stealth 3 at a distance of 15 now has mstealth of 71 */
        /* monster of stealth 5 at a distance of 15 now has mstealth of 113 */
        /* monster of stealth 2 at a distance of 9 now has mstealth of 54 */
        /* monster of stealth 3 at a distance of 7 now has mstealth of 59 */
        /* monster of stealth 4 at a distance of 6 now has mstealth of 70 */
        /* monster of stealth 5 at a distance of 5 now has mstealth of 79 */
        /* monster of stealth 6 at a distance of 4 now has mstealth of 86 */
        /* monster of stealth 5 at a distance of 1 now has mstealth of 53 */
        /* monster of stealth 6 at a distance of 8 now has mstealth of 110 */
        /* does the player notice the monster? */
        if (d > 2)
        {
              /* give monsters a few chances to avoid being noticed */
              if ((mstealth > (palert + 50)) && (m_ptr->monseen < 1)) easy = FALSE;
              if ((mstealth > (palert * 2)) && (randint(mstealth) > palert + 4)) easy = FALSE;
              if (randint(mstealth) > palert) easy = FALSE;
              if (randint(mstealth) > palert + rp_ptr->r_fos) easy = FALSE;
              if ((palert > mstealth) && (r_ptr->stealth > 1) && 
                  (randint(palert * 20) < (mstealth/2 + d + badluck/2 - goodluck/2))) easy = FALSE;
        }
        else if (d == 2)
        {
              /* only one chance to escape notice when very close */
              if (randint(mstealth) > palert) easy = FALSE;
        }
        /* d == 1: always notice if adjacent */
    }
    /* an extremely alert character may notice signs of an invisible monster */
    else if ((!p_ptr->see_inv) && (r_ptr->flags2 & (RF2_INVISIBLE)) && (full))
    {
        int mstealth = 48 + (r_ptr->stealth * 5) + (d * 2);
        if (goodluck > 4) mstealth -= (goodluck/4 + randint(goodluck/2));
        if (badluck > 4) mstealth += (badluck/4 + rand_int(badluck/3));
        if ((r_ptr->flags3 & (RF3_UNDEAD)) || (r_ptr->flags3 & (RF3_SILVER)))
        {
           mstealth += 25;
        }

        if ((d > 3) && (r_ptr->stealth > 3)) mstealth += (d-3) * 2;
        else if (d > 3) mstealth += (d-3);
        else if (d < 2) mstealth -= 10;
        if ((m_ptr->monseen > 0) && (r_ptr->flags1 & (RF1_NEVER_MOVE))) mstealth -= (5 + m_ptr->monseen);
        if (m_ptr->monseen > 0) mstealth -= m_ptr->monseen;

        /* minimum stealth for invisibile monsters */
        if (mstealth < 30) mstealth = 30;
        if (palert < 1) palert = 1;

        /* impossible to notice invisible monsters further than 6 spaces away */
        if ((d < 7) && (r_ptr->stealth < 5))
        {
              if (randint(palert) > mstealth)
              {
                  easy = TRUE;
                  msg_format("You notice the signs of an invisible monster!");
              }
        }
    }

    /* once it's noticed by the player it's easier to see afterwords */
    if ((easy) && (m_ptr->monseen < 1)) m_ptr->monseen = 2;

    /* water hide monsters can hide even if you can still see them */
	if (((r_ptr->flags7 & (RF7_WATER_HIDE)) || (r_ptr->flags7 & (RF7_WATER_ONLY))) &&
		(cave_feat[fy][fx] == FEAT_WATER))
	{
		 if ((m_ptr->monseen > 2) || (randint(200) < r_ptr->stealth*5))
			 m_ptr->monseen -= 1;
	}
    /* monster has been seen but is hiding again */
	else if ((!easy) && (m_ptr->monseen > 2)) m_ptr->monseen -= 1;
    else if ((!easy) && (m_ptr->monseen > 1) && (randint(200) < r_ptr->stealth*5))
          m_ptr->monseen -= 1;
    if (m_ptr->monseen > 5) m_ptr->monseen -= 1;

    /* is it still visible after monster stealth is checked? */
    if (easy) flag = TRUE;

	/*** DJA: hearing out-of-sight monsters ***/
	/* maybe should add a 'move silently' stat for monsters separate from 'hide' stealth */
    hearcheck = (palert * 4) + (goodluck/2);
	/* hear better when you're blind */
	if (p_ptr->timed[TMD_BLIND]) hearcheck += 100;
	/* not attentive when you're running */
    if (p_ptr->running) hearcheck = hearcheck/2;
	/* hear it better when you're listening for it */
    else if (m_ptr->heard) hearcheck += 100;

	/* PASS_WALL and NEVER_MOVE monsters are considered completely silent */
	/* (unless their stealth is 0: poltergeists, green glutton ghosts, and earth elementals) */
	msilent = FALSE; /* assume not completely silent */
	if ((r_ptr->flags2 & (RF2_PASS_WALL)) && (r_ptr->stealth)) msilent = TRUE;
	if (r_ptr->flags1 & (RF1_NEVER_MOVE)) msilent = TRUE;
	/* poltergeists make a lot of noise */
	if (r_ptr->flags4 & (RF4_THROW)) hearcheck += 30;
	/* sleeping monsters are silent */
	if ((m_ptr->csleep) && (!m_ptr->roaming)) msilent = TRUE;
	/* don't hear offscreen monsters */
	if (!panel_contains(fy, fx)) msilent = TRUE;
	/* alertness isn't high enough */
	if ((r_ptr->stealth == 3) && (palert < 26)) msilent = TRUE;
	if ((r_ptr->stealth == 2) && (palert < 20)) msilent = TRUE;

	/* if monster is nearby and not already visible there's a chance to hear it */
	/* the town is considered safe, so don't bother if monster level is 0 */
	if ((!flag) && (full) && (r_ptr->stealth < 4) && (d < MAX_SIGHT) &&
	   (!p_ptr->timed[TMD_IMAGE]) && (r_ptr->level) && (!msilent) &&
	   (randint(2500 + (r_ptr->stealth * 500) + (d * 25)) < hearcheck))
	{
		bool eardisturb = FALSE;

		/* decide whether to disturb */
		if (disturb_move) eardisturb = TRUE;
		/* don't disturb when running, resting, or digging */
		if ((p_ptr->resting) || (p_ptr->running) || (p_ptr->command_rep)) eardisturb = FALSE;
		/* ..unless it's in line of sight */
		if ((disturb_near) && (player_has_los_bold(fy, fx))) eardisturb = TRUE;

		/* disturb on hearing */
		if ((!m_ptr->heard) && (eardisturb))
		{
			/* (no message if no disturb) */
			msg_format("You hear a monster nearby but out of sight.");
			disturb(1, 0);
		}

        /* you heard it, now you're listening for it */
        m_ptr->heard = TRUE;

        /* you heard it, now you're watching for it */
        if (m_ptr->monseen < 1) m_ptr->monseen = 1;

		/* can't target or examine the monster */
    }
    else if ((full) && (m_ptr->heard))
	{
		m_ptr->heard = FALSE;

		/* forget where the monster is */
		lite_spot(fy, fx);
		p_ptr->window |= PW_MONLIST;
	}

	if (m_ptr->heard)
	{
		/* Draw the monster (always grey color) */
		lite_spot(fy, fx);

		/* (is listed in monster list but only tells type, not exact race) */
		p_ptr->window |= PW_MONLIST;
	}

	/* check if the monster never moves (check more than just NEVER_MOVE flag) */
	if ((l_ptr->flags1 & (RF1_NEVER_MOVE)) &&
		(!(r_ptr->flags6 & (RF6_BLINK))) &&
		(!(r_ptr->flags6 & (RF6_TPORT)))) nevermove = TRUE;
	else nevermove = FALSE;

	/* The monster is now visible */
	if (flag)
	{
		/* It was previously unseen */
		if (!m_ptr->ml)
		{
			/* Mark as visible */
			m_ptr->ml = TRUE;

			/* Draw the monster */
			lite_spot(fy, fx);

			/* Update health bar as needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Hack -- Count "fresh" sightings */
			if (l_ptr->sights < MAX_SHORT) l_ptr->sights++;

			/* Disturb on appearance */
			if (disturb_move) disturb(1, 0);

			/* Window stuff */
			p_ptr->window |= PW_MONLIST;
		}

		/* everyone knows ordinary trees don't move.. */
		if (m_ptr->r_idx == 834)
		{
			monster_lore *l_ptr = &l_list[m_ptr->r_idx];
			l_ptr->flags1 |= (RF1_NEVER_MOVE);
		}
	}

	/* The monster is not visible */
	else
	{
		/* It was previously seen */
		/* if you know that a monster never moves, remember where it is */
		if ((m_ptr->ml) && (!nevermove))
		{
			/* Mark as not visible */
			m_ptr->ml = FALSE;

			/* Erase the monster */
			lite_spot(fy, fx);

			/* Update health bar as needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Disturb on disappearance */
			if (disturb_move) disturb(1, 0);

			/* Window stuff */
			p_ptr->window |= PW_MONLIST;
		}
	}

	/* The monster is now easily visible */
	if (easy)
	{
		/* Change */
		if (!(m_ptr->mflag & (MFLAG_VIEW)))
		{
			/* Mark as easily visible */
			m_ptr->mflag |= (MFLAG_VIEW);

#if idontrememberwhatthisisfor
			/* Disturb on appearance */
			if ((disturb_near) && ((!m_ptr->roaming) || (m_ptr->roaming < 20))) disturb(1, 0);
#endif

			/* Disturb on appearance (even if roaming) */
			if (disturb_near) disturb(1, 0);

			/* Window stuff */
			p_ptr->window |= PW_MONLIST;
		}
	}

	/* The monster is not easily visible */
	else
	{
		/* Change */
		if (m_ptr->mflag & (MFLAG_VIEW))
		{
			/* Mark as not easily visible */
			m_ptr->mflag &= ~(MFLAG_VIEW);

			/* Window stuff */
			p_ptr->window |= PW_MONLIST;
		}
	}

	/* don't mark nevermove monster as in LOS when they aren't */
	/* (don't get why the easy flag doesn't work for this) */
	if ((nevermove) && (!player_has_los_bold(fy, fx)))
	{
		/* Change */
		if (m_ptr->mflag & (MFLAG_VIEW))
		{
			/* Mark as not easily visible */
			m_ptr->mflag &= ~(MFLAG_VIEW);

			/* Window stuff */
			p_ptr->window |= PW_MONLIST;
		}
	}

	
    /* monster now has LOS to the player */
    if ((m_ptr->cdis <= MAX_RANGE) && (projectable(m_ptr->fy, m_ptr->fx, py, px)))
	{
        if (!(m_ptr->mflag & (MFLAG_MLOS)))
        {
			/* Mark as having LOS to the player */
			m_ptr->mflag |= (MFLAG_MLOS);
			
            /* only note if awake and sensed by telepathy */
            if ((disturb_espmove) && (flag) && (!easy) && (!m_ptr->csleep))
            {
               disturb(1, 0);
#if nomessage
               /* get monster name */
		       char m_name[80];
               monster_desc(m_name, sizeof(m_name), m_ptr, 0x04);
                       
               /* should there be a message here? */
               msg_format("You sense that %s can see you.", m_name);
#endif
            }
        }
    }
    else if (m_ptr->mflag & (MFLAG_MLOS))
    {
		/* Mark as not having LOS to the player */
		m_ptr->mflag &= ~(MFLAG_MLOS);
    }

	/* check monster's awareness of you */
	/* asleep but displayed as awake in monster list */
	if ((m_ptr->csleep) && (!m_ptr->display_sleep))
	{
	   /* update monster list */
	   p_ptr->window |= PW_MONLIST;
    }
    /* awake but displayed as asleep in monster list */
	if ((!m_ptr->csleep) && (m_ptr->display_sleep))
	{
	   /* update monster list */
	   p_ptr->window |= PW_MONLIST;
    }

   /* done if you've met the monster before or if you're not meeting it now */
   if ((m_ptr->meet) || (!easy)) return;

   /* "Nice to meet you" (First impression) */
   if (easy)
   {
       /* bluff */
	   int bluff = r_ptr->stealth;
	   if (r_ptr->level <= 20) bluff += 1;
	   else bluff += r_ptr->level / 20;
	   if (r_ptr->flags2 & (RF2_SMART)) bluff += randint((bluff+1)/2) + 1;
	   if ((r_ptr->flags2 & (RF2_STUPID)) && (bluff > 1)) bluff = bluff / 2;
	   bluff -= (l_ptr->sights)/10;
	   if (bluff < 1) bluff = 1;
	   
	   /* Meeting the monster for the first time */
       m_ptr->meet = 1;
      
       /*** Can you discern if the monster is evil? ***/

       /* discernment based on wisdom */
       discernmod = (adj_mag_study[p_ptr->stat_ind[A_WIS]] / 10);

       /* paladins almost always recognise evil */
       if ((cp_ptr->spell_book == TV_PRAYER_BOOK) && (!cp_ptr->flags & CF_ZERO_FAIL))
       {
          discernmod += 21;
       }
       /* aligned classes recognise alignment easier */
       /* (priests and druids but not healers) */
       else if ((cp_ptr->flags & CF_BLESS_WEAPON) ||
            (cp_ptr->spell_book == TV_DARK_BOOK))
       {
            /* wielding equipment with alignment that matches yours */
            if ((magicmod == 4) || (magicmod == 6)) discernmod += 14;
            else discernmod += 7;
       }
       /* small bonus for healers */
       else if (cp_ptr->spell_book == TV_PRAYER_BOOK)
       {
            discernmod += 3;
       }

	   /* telepathy helps */
	   if (p_ptr->telepathy) discernmod += 7 + (goodluck/2);

       /* chance realm casters get modifier from luck */
       if (cp_ptr->spell_book == TV_LUCK_BOOK)
       {
          if (goodluck > 2) discernmod += goodluck/3;
          if (badluck > 2) discernmod -= badluck/3;
       }
       /* everyone else gets much weaker modifier from luck */
       else
       {
           if (goodluck >= 5) discernmod += goodluck/5;
           if (badluck >= 5) discernmod -= badluck/5;
       }

       lore = TRUE;

       /* always recognise if you know the race is always evil */
       if (l_ptr->flags3 & (RF3_EVIL)) discernmod += 50;
       /* easier to recognise if you know it's sometimes evil */
       else if (l_ptr->flags2 & (RF2_S_EVIL2)) discernmod += 4;
       else if (l_ptr->flags2 & (RF2_S_EVIL1)) discernmod += 2;
       else lore = FALSE;
   
       if (!lore)
       {
          /* always evil monsters are easier to recognise */
          if (r_ptr->flags3 & (RF3_EVIL)) discernmod += 2;
          /* occationally evil monsters are harder to recognise */
          /* if you don't know that they're sometimes evil */
          else if (r_ptr->flags2 & (RF2_S_EVIL2)) discernmod -= 5;
          else if (r_ptr->flags2 & (RF2_S_EVIL1)) discernmod -= 10;
       }
   
       /* discern */
       if (bluff + randint(24) < discernmod)
       {
          /* If it's always evil then the monster lore tells you that it's evil */
          if (r_ptr->flags3 & (RF3_EVIL))
          {
             /* remember */
             l_ptr->flags3 |= (RF3_EVIL);

             /* don't need a separate message telling you it's evil */
          }
          else if (m_ptr->evil)
          {
			  /* can tell (later) that the monster is evil */
			  m_ptr->meet = 2;
              
			  if (r_ptr->flags2 & (RF2_S_EVIL2)) l_ptr->flags2 |= (RF2_S_EVIL2);
			  else if (r_ptr->flags2 & (RF2_S_EVIL1)) l_ptr->flags2 |= (RF2_S_EVIL1);
          }
          /* (much harder to tell for sure that a monster is NOT evil) */
          else if (bluff + randint(50) < discernmod)
          {
              /* can tell (later) that the monster is not evil */
              m_ptr->meet = 3;
          }
       }
       /* if you fail this first chance, a detect evil spell is the only */
       /* way to find out if an individual monster is evil or not */
   }
}




/*
 * This function simply updates all the (non-dead) monsters (see above).
 */
void update_monsters(bool full)
{
	int i;

	/* Update each (live) monster */
	for (i = 1; i < mon_max; i++)
	{
		monster_type *m_ptr = &mon_list[i];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Update the monster */
		update_mon(i, full);
	}
}




/*
 * Make a monster carry an object
 */
s16b monster_carry(int m_idx, object_type *j_ptr)
{
	s16b o_idx;

	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = &mon_list[m_idx];


	/* Scan objects already being held for combination */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr))
		{
			/* Combine the items */
			object_absorb(o_ptr, j_ptr);

			/* Result */
			return (this_o_idx);
		}
	}


	/* Make an object */
	o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		object_type *o_ptr;

		/* Get new object */
		o_ptr = &o_list[o_idx];

		/* Copy object */
		object_copy(o_ptr, j_ptr);

		/* Forget mark */
		o_ptr->marked = FALSE;

		/* Forget location */
		o_ptr->iy = o_ptr->ix = 0;

		/* Link the object to the monster */
		o_ptr->held_m_idx = m_idx;

		/* Link the object to the pile */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		/* Link the monster to the object */
		m_ptr->hold_o_idx = o_idx;
	}

	/* Result */
	return (o_idx);
}


/*
 * Swap the players/monsters (if any) at two locations XXX XXX XXX
 * (this is used whenever a monster or the player moves)
 */
void monster_swap(int y1, int x1, int y2, int x2)
{
	int m1, m2;

	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_race *ar_ptr;

	/* Monsters */
	m1 = cave_m_idx[y1][x1];
	m2 = cave_m_idx[y2][x2];


	/* Update grids */
	cave_m_idx[y1][x1] = m2;
	cave_m_idx[y2][x2] = m1;

	/* if the PC moves, DEMON_WARD ends */
	if ((m1 < 0) || (m2 < 0)) clear_timed(TMD_DEMON_WARD);

	/* Monster 1 */
	if (m1 > 0)
	{
		m_ptr = &mon_list[m1];
		r_ptr = &r_info[m_ptr->r_idx];

		/* Move monster */
		m_ptr->fy = y2;
		m_ptr->fx = x2;

		/* handle monsters that block line of sight */
		if (r_ptr->flags7 & (RF7_BLOCK_LOS))
		{
			cave_info[y2][x2] |= (CAVE_WALL);
			/* allow for PASS_WALL monsters that block LOS */
			if ((cave_feat[y1][x1] >= FEAT_SECRET) ||
				((cave_feat[y1][x1] >= FEAT_DOOR_HEAD) &&
				(cave_feat[y1][x1] <= FEAT_DOOR_TAIL)))
			{
				/* don't remove CAVE_WALL */
			}
			else
			{
				cave_info[y1][x1] &= ~(CAVE_WALL);
			}
		}

		/* Update monster */
		update_mon(m1, TRUE);
	}

	/* Player 1 */
	else if (m1 < 0)
	{
		/* Move player */
		p_ptr->py = y2;
		p_ptr->px = x2;

		/* Update the panel */
		p_ptr->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_UPDATE_FLOW);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD | PW_MAP);
	}

	/* Monster 2 */
	if (m2 > 0)
	{
		m_ptr = &mon_list[m2];
		ar_ptr = &r_info[m_ptr->r_idx];

		/* Move monster */
		m_ptr->fy = y1;
		m_ptr->fx = x1;

		/* handle monsters that block line of sight */
		if (ar_ptr->flags7 & (RF7_BLOCK_LOS))
		{
			cave_info[y2][x2] |= (CAVE_WALL);
			/* allow for PASS_WALL monsters that block LOS */
			if ((cave_feat[y1][x1] >= FEAT_SECRET) ||
				((cave_feat[y1][x1] >= FEAT_DOOR_HEAD) &&
				(cave_feat[y1][x1] <= FEAT_DOOR_TAIL)))
			{
				/* don't remove CAVE_WALL */
			}
			/* allow for two BLOCK_LOS monsters switching with each other */
			/* (although I don't think this should ever happen..) */
			else if ((m1 > 0) && (r_ptr->flags7 & (RF7_BLOCK_LOS)))
			{
				/* don't remove CAVE_WALL */
			}
			else
			{
				cave_info[y1][x1] &= ~(CAVE_WALL);
			}
		}

		/* Update monster */
		update_mon(m2, TRUE);
	}

	/* Player 2 */
	else if (m2 < 0)
	{
		/* Move player */
		p_ptr->py = y1;
		p_ptr->px = x1;

		/* Update the panel */
		p_ptr->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_UPDATE_FLOW);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD | PW_MAP);
	}

	/* Notice */
	note_spot(y1, x1);
	note_spot(y2, x2);

	/* Redraw */
	lite_spot(y1, x1);
	lite_spot(y2, x2);
}


/*
 * Place the player in the dungeon XXX XXX
 */
s16b player_place(int y, int x)
{
	/* Paranoia XXX XXX */
	if (cave_m_idx[y][x] != 0) return (0);


	/* Save player location */
	p_ptr->py = y;
	p_ptr->px = x;

	/* Mark cave grid */
	cave_m_idx[y][x] = -1;

	/* Success */
	return (-1);
}


/*
 * Place a copy of a monster in the dungeon XXX XXX
 */
s16b monster_place(int y, int x, monster_type *n_ptr)
{
	s16b m_idx;

	monster_type *m_ptr;
	monster_race *r_ptr;


	/* Paranoia XXX XXX */
	if (cave_m_idx[y][x] != 0) return (0);


	/* Get a new record */
	m_idx = mon_pop();

	/* Oops */
	if (m_idx)
	{
		/* Make a new monster */
		cave_m_idx[y][x] = m_idx;

		/* Get the new monster */
		m_ptr = &mon_list[m_idx];

		/* Copy the monster XXX */
		COPY(m_ptr, n_ptr, monster_type);

		/* Location */
		m_ptr->fy = y;
		m_ptr->fx = x;

		/* Update the monster */
		update_mon(m_idx, TRUE);

		/* Get the new race */
		r_ptr = &r_info[m_ptr->r_idx];

		/* Hack -- Notice new multi-hued monsters */
		if (r_ptr->flags1 & (RF1_ATTR_MULTI)) shimmer_monsters = TRUE;

		/* Hack -- Count the number of "reproducers" */
		if (r_ptr->flags2 & (RF2_MULTIPLY)) num_repro++;

		/* Count racial occurances */
		r_ptr->cur_num++;
	}

	/* Result */
	return (m_idx);
}



/*
 * Attempt to place a monster of the given race at the given location.
 *
 * To give the player a sporting chance, any monster that appears in
 * line-of-sight and is extremely dangerous can be marked as
 * "FORCE_SLEEP", which will cause them to be placed with low energy,
 * which often (but not always) lets the player move before they do.
 *
 * This routine refuses to place out-of-depth "FORCE_DEPTH" monsters.
 *
 * XXX XXX XXX Use special "here" and "dead" flags for unique monsters,
 * remove old "cur_num" and "max_num" fields.
 *
 * XXX XXX XXX Actually, do something similar for artifacts, to simplify
 * the "preserve" mode, and to make the "what artifacts" flag more useful.
 *
 * This is the only function which may place a monster in the dungeon,
 * except for the savefile loading code.
 */
static bool place_monster_one(int y, int x, int r_idx, bool slp, bool group)
{
	int i;
	byte isroam;
	bool roamflag;
	int evilchance;

	monster_race *r_ptr;
	monster_type *n_ptr;
	monster_type monster_type_body;
	cptr name;

	/* Paranoia */
	if (!in_bounds(y, x)) return (FALSE);

	/* Paranoia */
	if (!r_idx) return (FALSE);

	/* Race */
	r_ptr = &r_info[r_idx];

	/* WATER_ONLY monsters are only generated in water */
	/* checks this elseware, and should be allowed if caused by polymorph */
	/* if ((r_ptr->flags7 & (RF7_WATER_ONLY)) &&
		(!(cave_feat[y][x] == FEAT_WATER))) return (FALSE); */

	/* pass_door monsters (silver moss) can multiply into a door space */
	if ((((cave_feat[y][x] >= FEAT_DOOR_HEAD) &&
          (cave_feat[y][x] <= FEAT_DOOR_TAIL)) ||
         /* (cave_feat[y][x] == FEAT_SECRET) || */
		  (cave_feat[y][x] == FEAT_RUBBLE)) &&
		  (cave_m_idx[y][x] == 0) &&
		  (r_ptr->flags2 & (RF2_PASS_DOOR))) /* okay */;

	/* Require empty space */
	else if (!cave_can_occupy_bold(y, x)) return (FALSE);

	/* Hack -- no creation on glyph of warding */
	if (cave_feat[y][x] == FEAT_GLYPH) return (FALSE);


	/* Paranoia */
	if (!r_ptr->name) return (FALSE);

	/* Name */
	name = (r_name + r_ptr->name);


	/* Hack -- "unique" monsters must be "unique" */
	if ((r_ptr->flags1 & (RF1_UNIQUE)) && (r_ptr->cur_num >= r_ptr->max_num))
	{
		/* Cannot create */
		return (FALSE);
	}

	/* Depth monsters may NOT be created out of depth */
	if ((r_ptr->flags1 & (RF1_FORCE_DEPTH)) && (p_ptr->depth < r_ptr->level))
	{
		/* Cannot create */
		return (FALSE);
	}


	/* Powerful monster */
	if (r_ptr->level > p_ptr->depth)
	{
		/* Unique monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			/* Message for cheaters */
			if (cheat_hear) msg_format("Deep Unique (%s).", name);

			/* Boost rating by twice delta-depth */
			rating += (r_ptr->level - p_ptr->depth) * 2;
		}

		/* Normal monsters */
		else
		{
			/* Message for cheaters */
			if (cheat_hear) msg_format("Deep Monster (%s).", name);

			/* Boost rating by delta-depth */
			rating += (r_ptr->level - p_ptr->depth);
		}
	}

	/* Note the monster */
	else if (r_ptr->flags1 & (RF1_UNIQUE))
	{
		/* Unique monsters induce message */
		if (cheat_hear) msg_format("Unique (%s).", name);
	}


	/* Get local monster */
	n_ptr = &monster_type_body;

	/* Clean out the monster */
	(void)WIPE(n_ptr, monster_type);


	/* Save the race */
	n_ptr->r_idx = r_idx;


	/* likeliness of being awake without noticing the player */
	/* currently no monster has both ROAM1 and ROAM2 but could use that to stack */
	/* slp is only FALSE when monster is being generated over time after dungeon creation */
	isroam = 1;
	roamflag = FALSE;
	if ((r_ptr->flags2 & (RF2_ROAM2)) || (r_ptr->flags2 & (RF2_ROAM1))) roamflag = TRUE;
	if (r_ptr->flags2 & (RF2_ROAM1)) isroam += 25;
	if (r_ptr->flags2 & (RF2_ROAM2)) isroam += 74;
	/* monsters who are immune to sleep shouldn't sleep */
	if ((r_ptr->flags3 & (RF3_NO_SLEEP)) && (!roamflag)) isroam += 15;
	else if (r_ptr->flags3 & (RF3_NO_SLEEP)) isroam += 5;
	/* townspeople shouldn't just stand there (shouldn't have ROAM2) */
	if (r_ptr->level == 0) isroam += 55;
	/* some types of monsters rarely roam */
	if ((strchr("eglLsVwWXz$.", r_ptr->d_char)) && (!roamflag)) isroam = isroam/3;
	/* some types of monsters never roam */
	if ((!r_ptr->sleep) && (!roamflag)) isroam = 0;
	if ((strchr(",dDEjmQvZ%?!=_~", r_ptr->d_char)) && (!roamflag)) isroam = 0;

	/* water monsters never roam when in water */
	if (((r_ptr->flags7 & (RF7_WATER_ONLY)) || (r_ptr->flags7 & (RF7_WATER_HIDE))) &&
		(cave_feat[y][x] == FEAT_WATER))
	{
		isroam = 0;
	}

	/* is it roaming? */
	if (randint(100) < isroam)
    {
        n_ptr->roaming = 1;
        /* roams as part of a group */
        if (group) n_ptr->roaming = 11;
    }
	else n_ptr->roaming = 0;

	/* Enforce sleeping if needed */
	/* (old: all monsters start out asleep unless it was random generation over time */
	/* or unless they have 0 in the monster alertness field in monster.txt) */
	/* now: n_ptr->csleep means hasn't noticed the player yet, not nessesarily sleeping */
	/* if n_ptr->csleep > 0 and n_ptr->roaming = 0 then monster is asleep */
	if ((slp) && ((r_ptr->sleep) || (n_ptr->roaming)))
	{
		int val = r_ptr->sleep;
		if (val < 1) val = 1;
		
        /* town monsters ignore you most of the time */
       	if (!r_ptr->level) n_ptr->csleep = (val * 4) + randint(val * 9);
       	else n_ptr->csleep = (val * 2) + randint(val * 10);
	}

	/* BLOCK_LOS monsters get the CAVE_WALL flag */
	if (r_ptr->flags7 & (RF7_BLOCK_LOS))
	{
		cave_info[y][x] |= (CAVE_WALL);
	}

	/* Assign maximal hitpoints */
	if (r_ptr->flags1 & (RF1_FORCE_MAXHP))
	{
		n_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
	}
	else
	{
		n_ptr->maxhp = damroll(r_ptr->hdice, r_ptr->hside);
	}

	/* And start out fully healthy */
	n_ptr->hp = n_ptr->maxhp;

	/* Extract the monster base speed */
	n_ptr->mspeed = r_ptr->speed;

	/* Some monster races are sometimes evil but not always */
	evilchance = 0;
	/* uniques never have S_EVIL1 or S_EVIL2 */
	/* sometimes evil (should never be invisible except on very high levels) */
	if (r_ptr->flags2 & (RF2_S_EVIL1)) evilchance = 22;
	/* usually evil */
	else if (r_ptr->flags2 & (RF2_S_EVIL2)) evilchance = 78;
	
	/* if it's invisible, it's evil more often so it'll be easier to find */
	if ((evilchance) && (r_ptr->flags2 & (RF2_INVISIBLE)))
    {
        if (p_ptr->depth < 60) evilchance += (61 - p_ptr->depth)/3 + 1;
    }

    if ((randint(100) < evilchance) || (r_ptr->flags3 & (RF3_EVIL)))
    {
        n_ptr->evil = 1;
    }
    else
    {
        n_ptr->evil = 0;
    }
    
    /* nonevil monsters take less notice of you */
    if ((evilchance) && (n_ptr->csleep) && (!n_ptr->evil))
    {
        if (n_ptr->csleep < r_ptr->sleep * 6) n_ptr->csleep += r_ptr->sleep * 2 + 1;
    }
    else if ((evilchance) && (n_ptr->csleep))
    {
        if (n_ptr->csleep > r_ptr->sleep * 8) n_ptr->csleep -= r_ptr->sleep * 2;
    }

    /* you've never met this monster */
	n_ptr->meet = 0;

	/* Hack -- small racial variety */
	if (!(r_ptr->flags1 & (RF1_UNIQUE)))
	{
		/* Allow some small variation per monster */
		i = extract_energy[r_ptr->speed] / 10;
		if (i) n_ptr->mspeed += rand_spread(0, i);
	}

	/* Give a random starting energy */
	n_ptr->energy = (byte)rand_int(100);

	/* Force monster to wait for player */
	if (r_ptr->flags1 & (RF1_FORCE_SLEEP))
	{
		/* Monster is still being nice */
		n_ptr->mflag |= (MFLAG_NICE);

		/* Optimize -- Repair flags */
		repair_mflag_nice = TRUE;
	}

	/* Place the monster in the dungeon */
	if (!monster_place(y, x, n_ptr)) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Maximum size of a group of monsters
 */
#define GROUP_MAX	31


/*
 * Attempt to place a "group" of monsters around the given location
 */
static bool place_monster_group(int y, int x, int r_idx, bool slp)
{
	monster_race *r_ptr = &r_info[r_idx];

	int old, n, i;
	int tag = 0;
	int total, extra = 0;
	int hack_n;

	byte hack_y[GROUP_MAX];
	byte hack_x[GROUP_MAX];


	/* Pick a group size */
	total = randint(13);

	/* Hard monsters, small groups */
	if (r_ptr->level > p_ptr->depth)
	{
		extra = r_ptr->level - p_ptr->depth;
		extra = 0 - randint(extra);
	}

	/* Easy monsters, large groups */
	else if (r_ptr->level < p_ptr->depth)
	{
		extra = p_ptr->depth - r_ptr->level;
		extra = randint(extra);
	}

	/* Hack -- limit group reduction */
	if (extra > 12) extra = 12;

	/* Modify the group size */
	total += extra;

	/* Minimum size */
	if (total < 1) total = 1;

	/* Maximum size */
	if (total > GROUP_MAX) total = GROUP_MAX;

	/* Pairs (usually) */
	if ((r_ptr->flags1 & RF1_FRIEND) && (total > 2))
	{
       	if ((extra == 12) && (randint(100) < 60)) total = 4;
       	else if ((extra > 7) && (randint(100) < 20)) total = 3;
       	else total = 2;
    }

	/* small groups */
	if ((r_ptr->flags2 & RF2_FRIEND1) && (total > 7))
	{
       	if ((extra > 8) && (randint(100) < 50)) total = ((total * 3) / 4);
       	else total = ((total * 2) / 3);
	    if ((r_ptr->level > p_ptr->depth) && (total > 8)) total = 8;
    }

	/* Save the rating */
	old = rating;

	/* Start on the monster */
	hack_n = 1;
	hack_x[0] = x;
	hack_y[0] = y;

	/* maybe add a tribe shaman to the group */
	if ((r_ptr->flags3 & RF3_ORC) && (p_ptr->depth > 9))
	{
		int die = randint(101 - badluck);
		if (die < 35) tag = 155; /* orc shaman */
		else if ((die < 40) && (p_ptr->depth > 14)) tag = 187; /* fire orc */
	}
	else if (strchr("x", r_ptr->d_char))
	{
		int die = randint(70 + p_ptr->depth);
		if (die > 135) tag = 547; /* riverflow gargoyle */
		else if (die > 100) tag = 556; /* margoyle shaman */
	}
	else if (strchr("O", r_ptr->d_char))
	{
		int die = randint(70 + p_ptr->depth);
		if (die > 80) tag = 397; /* ogre shaman */
	}

	/* Puddle monsters, breadth first, up to total */
	for (n = 0; (n < hack_n) && (hack_n < total); n++)
	{
		/* Grab the location */
		int hx = hack_x[n];
		int hy = hack_y[n];

		/* Check each direction, up to total */
		for (i = 0; (i < 8) && (hack_n < total); i++)
		{
			int mx = hx + ddx_ddd[i];
			int my = hy + ddy_ddd[i];

			/* Walls and Monsters block flow */
			if (!cave_can_occupy_bold(my, mx)) continue;

			/* ordinary trees shouldn't block a doorway */
			if ((r_idx == 834) && ((cave_feat[my][mx] == FEAT_OPEN) ||
				(cave_feat[my][mx] == FEAT_BROKEN))) continue;

			/* maybe add a tribe shaman to the group */
			if ((hack_n == (total - 1)) && (tag))
			{
				if (place_monster_one(my, mx, tag, slp, TRUE))
				{
					/* Add it to the "hack" set */
					hack_y[hack_n] = my;
					hack_x[hack_n] = mx;
					hack_n++;
					continue;
				}
			}

			/* Attempt to place another monster */
			if (place_monster_one(my, mx, r_idx, slp, TRUE))
			{
				/* Add it to the "hack" set */
				hack_y[hack_n] = my;
				hack_x[hack_n] = mx;
				hack_n++;
			}
		}
	}

	/* Hack -- restore the rating */
	rating = old;


	/* Success */
	return (TRUE);
}


/*
 * Hack -- help pick an escort type
 */
static int place_monster_idx = 0;

/*
 * Hack -- help pick an escort type
 */
static bool place_monster_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[place_monster_idx];

	monster_race *z_ptr = &r_info[r_idx];

	/* Require similar "race" */
	if (z_ptr->d_char != r_ptr->d_char) return (FALSE);

	/* Skip more advanced monsters */
	if (z_ptr->level > r_ptr->level) return (FALSE);

	/* Skip unique monsters */
	if (z_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Paranoia -- Skip identical monsters */
	if (place_monster_idx == r_idx) return (FALSE);

	/* NEVER_MOVE monsters don't make good escorts */
	if ((z_ptr->flags1 & (RF1_NEVER_MOVE)) && (randint(100) < 85)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Attempt to place a monster of the given race at the given location
 *
 * Note that certain monsters are now marked as requiring "friends".
 * These monsters, if successfully placed, and if the "grp" parameter
 * is TRUE, will be surrounded by a "group" of identical monsters.
 *
 * Note that certain monsters are now marked as requiring an "escort",
 * which is a collection of monsters with similar "race" but lower level.
 *
 * Some monsters induce a fake "group" flag on their escorts.
 *
 * Note the "bizarre" use of non-recursion to prevent annoying output
 * when running a code profiler.
 *
 * Note the use of the new "monster allocation table" code to restrict
 * the "get_mon_num()" function to "legal" escort types.
 */
bool place_monster_aux(int y, int x, int r_idx, bool slp, bool grp)
{
	int i, place;

	monster_race *r_ptr = &r_info[r_idx];


	/* Place one monster, or fail */
	if (!place_monster_one(y, x, r_idx, slp, FALSE)) return (FALSE);


	/* Require the "group" flag */
	if (!grp) return (TRUE);


	/* Friends for certain monsters */
	if ((r_ptr->flags1 & (RF1_FRIENDS)) || (r_ptr->flags1 & (RF1_FRIEND)) || (r_ptr->flags2 & (RF2_FRIEND1)))
	{
		/* Attempt to place a group (if allowed) */
		(void)place_monster_group(y, x, r_idx, slp);
	}


	/* Escorts for certain monsters */
	if ((r_ptr->flags1 & (RF1_ESCORT)) || (r_ptr->flags2 & (RF2_ESCORT1)))
	{
 
        /* how many escorts to try to place */
        if (r_ptr->flags2 & (RF2_ESCORT1)) place = 20;
        else place = 50;
 
		/* Try to place several "escorts" */
		for (i = 0; i < place; i++)
		{
			int nx, ny, z, d = 3;

			/* Pick a location */
			scatter(&ny, &nx, y, x, d, 0);

			/* Require empty grids */
			if (!cave_can_occupy_bold(ny, nx)) continue;


			/* Set the escort index */
			place_monster_idx = r_idx;


			/* Set the escort hook */
			get_mon_num_hook = place_monster_okay;

			/* Prepare allocation table */
			get_mon_num_prep();


			/* Pick a random race */
			z = get_mon_num(r_ptr->level, FALSE);


			/* Remove restriction */
			get_mon_num_hook = NULL;

			/* Prepare allocation table */
			get_mon_num_prep();


			/* Handle failure */
			if (!z) break;

			/* Place a single escort */
			(void)place_monster_one(ny, nx, z, slp, TRUE);

			/* Place a "group" of escorts if needed */
			if ((r_info[z].flags1 & (RF1_FRIENDS)) ||
			    (r_ptr->flags1 & (RF1_ESCORTS)))
			{
				/* Place a group of monsters */
				(void)place_monster_group(ny, nx, z, slp);
			}
		}
	}


	/* Success */
	return (TRUE);
}


/*
 * Hack -- attempt to place a monster at the given location
 *
 * Attempt to find a monster appropriate to the "monster_level"
 */
bool place_monster(int y, int x, bool slp, bool grp, bool vault)
{
	int r_idx, tries = 0;
	monster_race *r_ptr;

	/* keeps trying until it gets a valid r_idx */
	while (tries < 90)
	{
		tries++;
		/* occationally force an especially appropriate themed monster */
		/* this will rarely cause an unusually out-of-depth monster */
		/* or very low level monster in a deep depth */
		if ((p_ptr->theme) && (rand_int(100) < 5))
		{
			r_idx = get_mon_force_theme(monster_level);
		}
		else
		{
			/* Pick a monster */
			r_idx = get_mon_num(monster_level, vault);
		}

		/* Handle failure */
		if (!r_idx) continue;

		/* don't create WATER_ONLY monsters in a space without water */
		/* (check this here to allow for 2nd tries to get_mon_num) */
		r_ptr = &r_info[r_idx];
		if ((r_ptr->flags7 & (RF7_WATER_ONLY)) &&
			(!(cave_feat[y][x] == FEAT_WATER)))
		{
			/* try again */
			continue;
		}

		/* otherwise accept */
		break;
	}

	/* Handle failure */
	if (!r_idx) return FALSE;

	/* Attempt to place the monster */
	if (place_monster_aux(y, x, r_idx, slp, grp)) return (TRUE);

	/* Oops */
	return (FALSE);
}



/*
 * XXX XXX XXX Player Ghosts are such a hack, they have been completely
 * removed.
 *
 * An idea for reintroducing them is to create a small number of
 * "unique" monsters which will serve as the "player ghosts".
 * Each will have a place holder for the "name" of a deceased player,
 * which will be extracted from a "bone" file, or replaced with a
 * "default" name if a real name is not available.  Each ghost will
 * appear exactly once and will not induce a special feeling.
 *
 * Possible methods:
 *   (s) 1 Skeleton
 *   (z) 1 Zombie
 *   (M) 1 Mummy
 *   (G) 1 Polterguiest, 1 Spirit, 1 Ghost, 1 Shadow, 1 Phantom
 *   (W) 1 Wraith
 *   (V) 1 Vampire, 1 Vampire Lord
 *   (L) 1 Lich
 *
 * Possible change: Lose 1 ghost, Add "Master Lich"
 *
 * Possible change: Lose 2 ghosts, Add "Wraith", Add "Master Lich"
 *
 * Possible change: Lose 4 ghosts, lose 1 vampire lord
 *
 * Note that ghosts should never sleep, should be very attentive, should
 * have maximal hitpoints, drop only good (or great) items, should be
 * cold blooded, evil, undead, immune to poison, sleep, confusion, fear.
 *
 * Base monsters:
 *   Skeleton
 *   Zombie
 *   Mummy
 *   Poltergeist
 *   Spirit
 *   Ghost
 *   Vampire
 *   Wraith
 *   Vampire Lord
 *   Shadow
 *   Phantom
 *   Lich
 *
 * This routine will simply extract ghost names from files, and
 * attempt to allocate a player ghost somewhere in the dungeon,
 * note that normal allocation may also attempt to place ghosts,
 * so we must work with some form of default names.
 *
 * XXX XXX XXX
 */




/*
 * Attempt to allocate a random monster in the dungeon.
 *
 * Place the monster at least "dis" distance from the player.
 *
 * Use "slp" to choose the initial "sleep" status
 *
 * Use "monster_level" for the monster level
 */
bool alloc_monster(int dis, bool slp)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x;
	int	attempts_left = 10000;

	/* Find a legal, distant, unoccupied, space */
	while (--attempts_left)
	{
		/* Pick a location */
		y = rand_int(DUNGEON_HGT);
		x = rand_int(DUNGEON_WID);

		/* Require "naked" floor grid */
		if (!cave_can_occupy_bold(y, x)) continue;

		/* Accept far away grids */
		if (distance(y, x, py, px) > dis) break;
	}

	if (!attempts_left)
	{
		if (cheat_xtra || cheat_hear)
		{
			msg_print("Warning! Could not allocate a new monster.");
		}

		return FALSE;
	}

	/* Attempt to place the monster, allow groups */
	if (place_monster(y, x, slp, TRUE, FALSE)) return (TRUE);

	/* Nope */
	return (FALSE);
}




/*
 * Hack -- the "type" of the current "summon specific"
 */
static int summon_specific_type = 0;


/*
 * Hack -- help decide if a monster race is "okay" to summon
 */
static bool summon_specific_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	bool okay = FALSE;


	/* Hack -- no specific type specified */
	if (!summon_specific_type) return (TRUE);


	/* Check our requirements */
	switch (summon_specific_type)
	{
		case SUMMON_ANIMAL:
		{
			okay = ((r_ptr->flags3 & (RF3_ANIMAL)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_SPIDER:
		{
			okay = ((r_ptr->d_char == 'S') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_HOUND:
		{
			okay = (((r_ptr->d_char == 'C') || (r_ptr->d_char == 'Z')) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_HYDRA:
		{
			okay = ((r_ptr->d_char == 'M') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_ANGEL: /* (ape) */
		{
			okay = ((r_ptr->d_char == 'A') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_DEMON:
		{
			okay = ((r_ptr->flags3 & (RF3_DEMON)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_UNDEAD: 
		{
			okay = ((r_ptr->flags3 & (RF3_UNDEAD)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_DRAGON:
		{
			okay = ((r_ptr->flags3 & (RF3_DRAGON)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_SILVER:
		{
			okay = ((r_ptr->flags3 & (RF3_SILVER)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_KIN:
		{
			okay = ((r_ptr->d_char == summon_kin_type) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_HI_UNDEAD:
		{
			okay = ((r_ptr->d_char == 'L') ||
			        (r_ptr->d_char == 'V') ||
			        (r_ptr->d_char == 'W'));
			break;
		}

		case SUMMON_HI_DRAGON:
		{
			okay = (r_ptr->d_char == 'D');
			break;
		}

		case SUMMON_HI_DEMON:
		{
			okay = ((r_ptr->d_char == 'U') ||
			        (r_ptr->d_char == '&'));
			break;
		}

		/* require correct color becuase there are now other unique 'W's */
		case SUMMON_WRAITH:
		{
			okay = ((r_ptr->d_char == 'W') &&
					(r_ptr->d_attr == TERM_RED) &&
			        (r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

		case SUMMON_UNIQUE:
		{
			okay = (r_ptr->flags1 & (RF1_UNIQUE)) ? TRUE : FALSE;
			break;
		}
	}

	/* Result */
	return (okay);
}


/*
 * Place a monster (of the specified "type") near the given
 * location.  Return TRUE if a monster was actually summoned.
 *
 * We will attempt to place the monster up to 10 times before giving up.
 *
 * Note: SUMMON_UNIQUE and SUMMON_WRAITH (XXX) will summon Uniques
 * Note: SUMMON_HI_UNDEAD and SUMMON_HI_DRAGON may summon Uniques
 * Note: None of the other summon codes will ever summon Uniques.
 *
 * This function has been changed.  We now take the "monster level"
 * of the summoning monster as a parameter, and use that, along with
 * the current dungeon level, to help determine the level of the
 * desired monster.  Note that this is an upper bound, and also
 * tends to "prefer" monsters of that level.  Currently, we use
 * the average of the dungeon and monster levels, and then add
 * five to allow slight increases in monster power.
 *
 * Note that we use the new "monster allocation table" creation code
 * to restrict the "get_mon_num()" function to the set of "legal"
 * monsters, making this function much faster and more reliable.
 *
 * Note that this function may not succeed, though this is very rare.
 */
bool summon_specific(int y1, int x1, int lev, int type)
{
	int i, x, y, r_idx, tries = 0;
	monster_race *r_ptr;

	/* Look for a location */
	for (i = 0; i < 22; ++i)
	{
		/* Pick a distance */
		int d = (i / 15) + 1;

		/* Pick a location */
		scatter(&y, &x, y1, x1, d, 0);

		/* Only check a circular area */
		/* (be sure to use the same criteria as summon_possible() ) */
		if (distance(y1, x1, y, x) > 2) continue;

		/* Require empty floor grid */
		if (!cave_can_occupy_bold(y, x)) continue;

		/* Require line of sight */
		if (!los(y1, x1, y, x))

		/* Hack -- no summon on glyph of warding */
		if (cave_feat[y][x] == FEAT_GLYPH) continue;

		/* Okay */
		break;
	}

	/* Failure */
	if (i == 22) return (FALSE);


	/* Save the "summon" type */
	summon_specific_type = type;


	/* Require "okay" monsters */
	get_mon_num_hook = summon_specific_okay;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* keeps trying until it gets a valid r_idx */
	while (tries < 90)
	{
		tries++;

		/* Pick a monster, using the level calculation */
		r_idx = get_mon_num((p_ptr->depth + lev) / 2 + 5, FALSE);

		/* Handle failure */
		if (!r_idx) continue;

		/* don't create WATER_ONLY monsters in a space without water */
		/* (check this here to allow for 2nd tries to get_mon_num) */
		r_ptr = &r_info[r_idx];
		if ((r_ptr->flags7 & (RF7_WATER_ONLY)) &&
			(!(cave_feat[y][x] == FEAT_WATER)))
		{
			/* try again */
			continue;
		}

		/* otherwise accept */
		break;
	}

	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Handle failure */
	if (!r_idx) return (FALSE);

	/* Attempt to place the monster (awake, allow groups) */
	if (!place_monster_aux(y, x, r_idx, FALSE, TRUE)) return (FALSE);

	/* Success */
	return (TRUE);
}





/*
 * Let the given monster attempt to reproduce.
 *
 * Note that "reproduction" REQUIRES empty space.
 */
bool multiply_monster(int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int i, y, x;

	bool result = FALSE;

	/* Try up to 18 times */
	for (i = 0; i < 18; i++)
	{
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, m_ptr->fy, m_ptr->fx, d, 0);

		/* pass_door monsters (silver moss) can multiply into a door space */
		if ((((cave_feat[y][x] >= FEAT_DOOR_HEAD) &&
		          (cave_feat[y][x] <= FEAT_DOOR_TAIL)) ||
		          (cave_feat[y][x] == FEAT_SECRET) ||
				  (cave_feat[y][x] == FEAT_RUBBLE)) &&
  				  (r_ptr->flags2 & (RF2_PASS_DOOR))) /* okay */;

		/* Require an "empty" floor grid */
		else if (!cave_can_occupy_bold(y, x)) continue;

		/* Create a new monster (awake, no groups) */
		result = place_monster_aux(y, x, m_ptr->r_idx, FALSE, FALSE);

		/* Done */
		break;
	}

	/* Result */
	return (result);
}





/*
 * Dump a message describing a monster's reaction to damage
 *
 * Technically should attempt to treat "Beholder"'s as jelly's
 */
void message_pain(int m_idx, int dam)
{
	long oldhp, newhp, tmp;
	int percentage;

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	char m_name[80];


	/* Get the monster name */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Notice non-damage */
	if (dam == 0)
	{
		msg_format("%^s is unharmed.", m_name);
		return;
	}

	/* Note -- subtle fix -CFT */
	newhp = (long)(m_ptr->hp);
	oldhp = newhp + (long)(dam);
	tmp = (newhp * 100L) / oldhp;
	percentage = (int)(tmp);


	/* Jelly's, Mold's, Vortex's, Quthl's */
	if (strchr("jmvQJwe,$", r_ptr->d_char))
	{
		if (percentage > 95)
			msg_format("%^s barely notices.", m_name);
		else if (percentage > 75)
			msg_format("%^s flinches.", m_name);
		else if (percentage > 50)
			msg_format("%^s squelches.", m_name);
		else if (percentage > 35)
			msg_format("%^s quivers in pain.", m_name);
		else if (percentage > 20)
			msg_format("%^s writhes about.", m_name);
		else if (percentage > 10)
			msg_format("%^s writhes in agony.", m_name);
		else
			msg_format("%^s jerks limply.", m_name);
	}

	/* Dogs and Hounds */
	else if (strchr("CZ", r_ptr->d_char))
	{
		if (percentage > 95)
			msg_format("%^s shrugs off the attack.", m_name);
		else if (percentage > 75)
			msg_format("%^s snarls with pain.", m_name);
		else if (percentage > 50)
			msg_format("%^s yelps in pain.", m_name);
		else if (percentage > 35)
			msg_format("%^s howls in pain.", m_name);
		else if (percentage > 20)
			msg_format("%^s howls in agony.", m_name);
		else if (percentage > 10)
			msg_format("%^s writhes in agony.", m_name);
		else
			msg_format("%^s yelps feebly.", m_name);
	}
    
   	/* monsters which don't feel pain (ignores) (undead, golems, and stupid trees) */
	else if ((strchr("szgW", r_ptr->d_char)) || 
		((strchr("E", r_ptr->d_char)) && (r_ptr->flags2 & (RF2_STUPID))))
	{
		if (percentage > 95)
			msg_format("%^s ignores the attack.", m_name);
		else if (percentage > 75)
			msg_format("%^s is slightly damaged.", m_name);
		else if (percentage > 50)
			msg_format("%^s is damaged.", m_name);
		else if (percentage > 35)
			msg_format("%^s is severely damaged.", m_name);
		else if (percentage > 20)
			msg_format("%^s has deep gashes but doesn't notice.", m_name);
		else if (percentage > 10)
			msg_format("%^s ignores the loss of its limbs.", m_name);
		else
			msg_format("%^s is literally falling apart.", m_name);
	}

	/* One type of monsters (ignore,squeal,shriek) */
	else if (strchr("FIMRSXabclqrt", r_ptr->d_char))
	{
		if (percentage > 95)
			msg_format("%^s ignores the attack.", m_name);
		else if (percentage > 75)
			msg_format("%^s grunts with pain.", m_name);
		else if (percentage > 50)
			msg_format("%^s squeals in pain.", m_name);
		else if (percentage > 35)
			msg_format("%^s shrieks in pain.", m_name);
		else if (percentage > 20)
			msg_format("%^s shrieks in agony.", m_name);
		else if (percentage > 10)
			msg_format("%^s writhes in agony.", m_name);
		else
			msg_format("%^s cries out feebly.", m_name);
	}

	/* Another type of monsters (shrug,cry,scream) */
	else
	{
		if (percentage > 95)
			msg_format("%^s shrugs off the attack.", m_name);
		else if (percentage > 75)
			msg_format("%^s grunts with pain.", m_name);
		else if (percentage > 50)
			msg_format("%^s cries out in pain.", m_name);
		else if (percentage > 35)
			msg_format("%^s screams in pain.", m_name);
		else if (percentage > 20)
			msg_format("%^s screams in agony.", m_name);
		else if (percentage > 10)
			msg_format("%^s writhes in agony.", m_name);
		else
			msg_format("%^s cries out feebly.", m_name);
	}
}



/*
 * Learn about an "observed" resistance.
 */
void update_smart_learn(int m_idx, int what)
{

#ifdef DRS_SMART_OPTIONS

	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];


	/* Not allowed to learn */
	if (!adult_ai_learn) return;

	/* Too stupid to learn anything */
	if (r_ptr->flags2 & (RF2_STUPID)) return;

	/* Not intelligent, only learn sometimes */
	if (!(r_ptr->flags2 & (RF2_SMART)) && (rand_int(100) < 50)) return;


	/* XXX XXX XXX */

	/* Analyze the knowledge */
	switch (what)
	{
		case DRS_FREE:
		{
			if (p_ptr->free_act) m_ptr->smart |= (SM_IMM_FREE);
			break;
		}

		case DRS_MANA:
		{
			if (!p_ptr->msp) m_ptr->smart |= (SM_IMM_MANA);
			break;
		}

		case DRS_RES_ACID:
		{
			if (p_ptr->resist_acid) m_ptr->smart |= (SM_RES_ACID);
			if (p_ptr->timed[TMD_OPP_ACID]) m_ptr->smart |= (SM_OPP_ACID);
			if (p_ptr->immune_acid) m_ptr->smart |= (SM_IMM_ACID);
			break;
		}

		case DRS_RES_ELEC:
		{
			if (p_ptr->resist_elec) m_ptr->smart |= (SM_RES_ELEC);
			if (p_ptr->timed[TMD_OPP_ELEC]) m_ptr->smart |= (SM_OPP_ELEC);
			if (p_ptr->immune_elec) m_ptr->smart |= (SM_IMM_ELEC);
			break;
		}

		case DRS_RES_FIRE:
		{
			if (p_ptr->resist_fire) m_ptr->smart |= (SM_RES_FIRE);
			if (p_ptr->timed[TMD_OPP_FIRE]) m_ptr->smart |= (SM_OPP_FIRE);
			if (p_ptr->immune_fire) m_ptr->smart |= (SM_IMM_FIRE);
			break;
		}

		case DRS_RES_COLD:
		{
			if (p_ptr->resist_cold) m_ptr->smart |= (SM_RES_COLD);
			if (p_ptr->timed[TMD_OPP_COLD]) m_ptr->smart |= (SM_OPP_COLD);
			if (p_ptr->immune_cold) m_ptr->smart |= (SM_IMM_COLD);
			break;
		}

		case DRS_RES_POIS:
		{
			if (p_ptr->resist_pois) m_ptr->smart |= (SM_RES_POIS);
			if (p_ptr->timed[TMD_OPP_POIS]) m_ptr->smart |= (SM_OPP_POIS);
			break;
		}

		case DRS_RES_FEAR:
		{
			if (p_ptr->resist_fear) m_ptr->smart |= (SM_RES_FEAR);
			break;
		}

		case DRS_RES_CHARM:
		{
		    /* if (p_ptr->resist_charm) m_ptr->smart |= (SM_RES_CHARM); */
			break;
		}
		
		case DRS_RES_LITE:
		{
			if (p_ptr->resist_lite) m_ptr->smart |= (SM_RES_LITE);
			break;
		}

		case DRS_RES_DARK:
		{
			if (p_ptr->resist_dark) m_ptr->smart |= (SM_RES_DARK);
			break;
		}

		case DRS_RES_BLIND:
		{
			if (p_ptr->resist_blind) m_ptr->smart |= (SM_RES_BLIND);
			break;
		}

		case DRS_RES_CONFU:
		{
			if (p_ptr->resist_confu) m_ptr->smart |= (SM_RES_CONFU);
			break;
		}

		case DRS_RES_SOUND:
		{
			if (p_ptr->resist_sound) m_ptr->smart |= (SM_RES_SOUND);
			break;
		}

		case DRS_RES_SHARD:
		{
			if (p_ptr->resist_shard) m_ptr->smart |= (SM_RES_SHARD);
			break;
		}

		case DRS_RES_NEXUS:
		{
			if (p_ptr->resist_nexus) m_ptr->smart |= (SM_RES_NEXUS);
			break;
		}

		case DRS_RES_NETHR:
		{
			if (p_ptr->resist_nethr) m_ptr->smart |= (SM_RES_NETHR);
			break;
		}

		case DRS_RES_CHAOS:
		{
			if (p_ptr->resist_chaos) m_ptr->smart |= (SM_RES_CHAOS);
			break;
		}

		case DRS_RES_DISEN:
		{
			if (p_ptr->resist_disen) m_ptr->smart |= (SM_RES_DISEN);
			break;
		}
	}

#endif /* DRS_SMART_OPTIONS */

}


