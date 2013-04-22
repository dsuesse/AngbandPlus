/* File: cmd1.c */

/* Purpose: Movement commands (part 1) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#define MAX_VAMPIRIC_DRAIN 100


/*
 * Determine if the player "hits" a monster (normal combat).
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_fire(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Never hit */
	if (chance <= 0) return (FALSE);

	/* Invisible monsters are harder to hit */
	if (!vis) chance = (chance + 1) / 2;

	/* Power competes against armor */
	if (rand_int(chance) < (ac * 3 / 4)) return (FALSE);

	/* Assume hit */
	return (TRUE);
}



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_norm(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Wimpy attack never hits */
	if (chance <= 0) return (FALSE);

	/* Penalize invisible targets */
	if (!vis) chance = (chance + 1) / 2;

	/* Power must defeat armor */
	if (rand_int(chance) < (ac * 3 / 4)) return (FALSE);

	/* Assume hit */
	return (TRUE);
}



/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
s16b critical_shot(int weight, int plus, int dam)
{
	int i, k;

	/* Extract "shot" power */
	i = (weight + ((p_ptr->to_h + plus) * 4) + (p_ptr->lev * 2));
        i += 50 * p_ptr->xtra_crit;

	/* Critical hit */
	if (randint(5000) <= i)
	{
		k = weight + randint(500);

		if (k < 500)
		{
			msg_print("It was a good hit!");
			dam = 2 * dam + 5;
		}
		else if (k < 1000)
		{
			msg_print("It was a great hit!");
			dam = 2 * dam + 10;
		}
		else
		{
			msg_print("It was a superb hit!");
			dam = 3 * dam + 15;
		}
	}

	return (dam);
}



/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
s16b critical_norm(int weight, int plus, int dam)
{
	int i, k;

	/* Extract "blow" power */
	i = (weight + ((p_ptr->to_h + plus) * 5) + (p_ptr->lev * 3));
        i += 50 * p_ptr->xtra_crit;

        /* Force good strikes */
        if (p_ptr->tim_deadly)
        {
                set_tim_deadly(p_ptr->tim_deadly - 1);
                msg_print("It was a *GREAT* hit!");
                dam = 3 * dam + 20;
        }
	/* Chance */
        else if (randint(5000) <= i)
	{
		k = weight + randint(650);

		if (k < 400)
		{
			msg_print("It was a good hit!");
			dam = 2 * dam + 5;
		}
		else if (k < 700)
		{
			msg_print("It was a great hit!");
			dam = 2 * dam + 10;
		}
		else if (k < 900)
		{
			msg_print("It was a superb hit!");
			dam = 3 * dam + 15;
		}
		else if (k < 1300)
		{
			msg_print("It was a *GREAT* hit!");
			dam = 3 * dam + 20;
		}
		else
		{
			msg_print("It was a *SUPERB* hit!");
			dam = ((7 * dam) / 2) + 25;
		}
	}

	return (dam);
}



/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 *
 * Note that most brands and slays are x3, except Slay Animal (x2),
 * Slay Evil (x2), and Kill dragon (x5).
 */
s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr, s32b *special)
{
	int mult = 1;

        monster_race *r_ptr = race_inf(m_ptr);

        u32b f1, f2, f3, f4, f5, esp;

	/* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Some "weapons" and "ammo" do extra damage */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
                case TV_AXE:
		case TV_DIGGING:
		{
			/* Slay Animal */
			if ((f1 & (TR1_SLAY_ANIMAL)) &&
			    (r_ptr->flags3 & (RF3_ANIMAL)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_ANIMAL);
				}

				if (mult < 2) mult = 2;
			}

			/* Slay Evil */
			if ((f1 & (TR1_SLAY_EVIL)) &&
			    (r_ptr->flags3 & (RF3_EVIL)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_EVIL);
				}

				if (mult < 2) mult = 2;
			}

			/* Slay Undead */
			if ((f1 & (TR1_SLAY_UNDEAD)) &&
			    (r_ptr->flags3 & (RF3_UNDEAD)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_UNDEAD);
				}

				if (mult < 3) mult = 3;
			}

			/* Slay Demon */
			if ((f1 & (TR1_SLAY_DEMON)) &&
			    (r_ptr->flags3 & (RF3_DEMON)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_DEMON);
				}

				if (mult < 3) mult = 3;
			}

			/* Slay Orc */
			if ((f1 & (TR1_SLAY_ORC)) &&
			    (r_ptr->flags3 & (RF3_ORC)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_ORC);
				}

				if (mult < 3) mult = 3;
			}

			/* Slay Troll */
			if ((f1 & (TR1_SLAY_TROLL)) &&
			    (r_ptr->flags3 & (RF3_TROLL)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_TROLL);
				}



				if (mult < 3) mult = 3;
			}

			/* Slay Giant */
			if ((f1 & (TR1_SLAY_GIANT)) &&
			    (r_ptr->flags3 & (RF3_GIANT)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_GIANT);
				}

				if (mult < 3) mult = 3;
			}

			/* Slay Dragon  */
			if ((f1 & (TR1_SLAY_DRAGON)) &&
			    (r_ptr->flags3 & (RF3_DRAGON)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_DRAGON);
				}

				if (mult < 3) mult = 3;
			}

			/* Execute Dragon */
			if ((f1 & (TR1_KILL_DRAGON)) &&
			    (r_ptr->flags3 & (RF3_DRAGON)))
			{
				if (m_ptr->ml)
				{
					r_ptr->r_flags3 |= (RF3_DRAGON);
				}

				if (mult < 5) mult = 5;

				if ((o_ptr->name1 == ART_AEGLIN) &&
				    strstr(r_name + r_ptr->name, "Fafner"))
					mult *= 3;
			}

                        /* Execute Undead */
                        if ((f5 & (TR5_KILL_UNDEAD)) &&
                            (r_ptr->flags3 & (RF3_UNDEAD)))
			{
				if (m_ptr->ml)
				{
                                        r_ptr->r_flags3 |= (RF3_UNDEAD);
				}

				if (mult < 5) mult = 5;
			}

                        /* Execute Demon */
                        if ((f5 & (TR5_KILL_DEMON)) &&
                            (r_ptr->flags3 & (RF3_DEMON)))
			{
				if (m_ptr->ml)
				{
                                        r_ptr->r_flags3 |= (RF3_DEMON);
				}

				if (mult < 5) mult = 5;
			}


			/* Brand (Acid) */
			if (f1 & (TR1_BRAND_ACID))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_ACID))
				{
					if (m_ptr->ml)
					{
						r_ptr->r_flags3 |= (RF3_IM_ACID);
					}
				}

                                /* Notice susceptibility */
                                else if (r_ptr->flags9 & (RF9_SUSCEP_ACID))
				{
					if (m_ptr->ml)
					{
                                                r_ptr->r_flags9 |= (RF9_SUSCEP_ACID);
					}
                                        if (mult < 6) mult = 6;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Elec) */
			if (f1 & (TR1_BRAND_ELEC))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_ELEC))
				{
					if (m_ptr->ml)
					{
						r_ptr->r_flags3 |= (RF3_IM_ELEC);
					}
				}

                                /* Notice susceptibility */
                                else if (r_ptr->flags9 & (RF9_SUSCEP_ELEC))
				{
					if (m_ptr->ml)
					{
                                                r_ptr->r_flags9 |= (RF9_SUSCEP_ELEC);
					}
                                        if (mult < 6) mult = 6;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Fire) */
			if (f1 & (TR1_BRAND_FIRE))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_FIRE))
				{
					if (m_ptr->ml)
					{
						r_ptr->r_flags3 |= (RF3_IM_FIRE);
					}
				}

                                /* Notice susceptibility */
                                else if (r_ptr->flags3 & (RF3_SUSCEP_FIRE))
				{
					if (m_ptr->ml)
					{
                                                r_ptr->r_flags3 |= (RF3_SUSCEP_FIRE);
					}
                                        if (mult < 6) mult = 6;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Cold) */
			if (f1 & (TR1_BRAND_COLD))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_COLD))
				{
					if (m_ptr->ml)
					{
						r_ptr->r_flags3 |= (RF3_IM_COLD);
					}
				}

                                /* Notice susceptibility */
                                else if (r_ptr->flags3 & (RF3_SUSCEP_COLD))
				{
					if (m_ptr->ml)
					{
                                                r_ptr->r_flags3 |= (RF3_SUSCEP_COLD);
					}
                                        if (mult < 6) mult = 6;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

                        /* Brand (Poison) */
			if (f1 & (TR1_BRAND_POIS))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_POIS))
				{
					if (m_ptr->ml)
					{
						r_ptr->r_flags3 |= (RF3_IM_POIS);
					}
				}

                                /* Notice susceptibility */
                                else if (r_ptr->flags9 & (RF9_SUSCEP_POIS))
				{
					if (m_ptr->ml)
					{
                                                r_ptr->r_flags9 |= (RF9_SUSCEP_POIS);
					}
                                        if (mult < 6) mult = 6;
                                        if (magik(95)) *special |= SPEC_POIS;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
                                        if (magik(50)) *special |= SPEC_POIS;
				}
			}

                        /* Wounding */
                        if (f5 & (TR5_WOUNDING))
			{
				/* Notice immunity */
#if 0 // DGDGDGDG
                                if (r_ptr->flags3 & (RF3_NO_CUT))
				{
					if (m_ptr->ml)
					{
                                                r_ptr->r_flags3 |= (RF3_NO_CUT);
					}
				}

				/* Otherwise, take the damage */
				else
#endif
				{
                                        if (magik(50)) *special |= SPEC_CUT;
				}
			}
			break;
		}
	}


	/* Return the total damage */
	return (tdam * mult);
}


/*
 * Search for hidden things
 */
void search(void)
{
	int y, x, chance;

	s16b this_o_idx, next_o_idx = 0;

	cave_type *c_ptr;

	/* Start with base search ability */
	chance = p_ptr->skill_srh;

	/* Penalize various conditions */
	if (p_ptr->blind || no_lite()) chance = chance / 10;
	if (p_ptr->confused || p_ptr->image) chance = chance / 10;

	/* Search the nearby grids, which are always in bounds */
	for (y = (py - 1); y <= (py + 1); y++)
	{
		for (x = (px - 1); x <= (px + 1); x++)
		{
			/* Sometimes, notice things */
			if (rand_int(100) < chance)
			{
				/* Access the grid */
				c_ptr = &cave[y][x];

				/* Invisible trap */
				if ((c_ptr->t_idx != 0) &&
				    !(c_ptr->info & CAVE_TRDT))
				{
					/* Pick a trap */
					pick_trap(y, x);

					/* Message */
					msg_print("You have found a trap.");

					/* Disturb */
					disturb(0, 0);
				}

				/* Secret door */
				if (c_ptr->feat == FEAT_SECRET)
				{
					/* Message */
					msg_print("You have found a secret door.");

					/* Pick a door XXX XXX XXX */
					cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);
                                        cave[y][x].mimic = 0;
                                        lite_spot(y, x);

					/* Disturb */
					disturb(0, 0);
				}

				/* Scan all objects in the grid */
				for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
				{
					object_type *o_ptr;

					/* Acquire object */
					o_ptr = &o_list[this_o_idx];

					/* Acquire next object */
					next_o_idx = o_ptr->next_o_idx;

					/* Skip non-chests */
					if (o_ptr->tval != TV_CHEST) continue;

					/* Skip non-trapped chests */
					if (!o_ptr->pval) continue;

					/* Identify once */
					if (!object_known_p(o_ptr))
					{
						/* Message */
						msg_print("You have discovered a trap on the chest!");

						/* Know the trap */
						object_known(o_ptr);

						/* Notice it */
						disturb(0, 0);
					}
				}
			}
		}
	}
}




/*
 * Player "wants" to pick up an object or gold.
 * Note that we ONLY handle things that can be picked up.
 * See "move_player()" for handling of other things.
 */
void carry(int pickup)
{
        if(!p_ptr->disembodied)
        {
                py_pickup_floor(pickup);
        }
}


/*
 * Handle player hitting a real trap
 */
static void hit_trap(void)
{
	bool ident=FALSE;

	cave_type *c_ptr;

	/* Disturb the player */
	disturb(0, 0);

	/* Get the cave grid */
	c_ptr = &cave[py][px];
	if (c_ptr->t_idx != 0)
	{
		ident = player_activate_trap_type(py, px, NULL, -1);
		if (ident)
		{
			t_info[c_ptr->t_idx].ident = TRUE;
			msg_format("You identified the trap as %s.",
				   t_name + t_info[c_ptr->t_idx].name);
		}
	}
}


void touch_zap_player(monster_type *m_ptr)
{
	int aura_damage = 0;
        monster_race *r_ptr = race_inf(m_ptr);

	if (r_ptr->flags2 & (RF2_AURA_FIRE))
	{
		if (!(p_ptr->immune_fire))
		{
			char aura_dam[80];

                        aura_damage = damroll(1 + (m_ptr->level / 26), 1 + (m_ptr->level / 17));

			/* Hack -- Get the "died from" name */
			monster_desc(aura_dam, m_ptr, 0x88);

			msg_print("You are suddenly very hot!");

			if (p_ptr->oppose_fire) aura_damage = (aura_damage+2) / 3;
			if (p_ptr->resist_fire) aura_damage = (aura_damage+2) / 3;
                        if (p_ptr->sensible_fire) aura_damage = (aura_damage+2) * 2;

			take_hit(aura_damage, aura_dam);
			r_ptr->r_flags2 |= RF2_AURA_FIRE;
			handle_stuff();
		}
	}


	if (r_ptr->flags2 & (RF2_AURA_ELEC))
	{
		if (!(p_ptr->immune_elec))
		{
			char aura_dam[80];

                        aura_damage = damroll(1 + (m_ptr->level / 26), 1 + (m_ptr->level / 17));

			/* Hack -- Get the "died from" name */
			monster_desc(aura_dam, m_ptr, 0x88);

			if (p_ptr->oppose_elec) aura_damage = (aura_damage+2) / 3;
			if (p_ptr->resist_elec) aura_damage = (aura_damage+2) / 3;

			msg_print("You get zapped!");
			take_hit(aura_damage, aura_dam);
			r_ptr->r_flags2 |= RF2_AURA_ELEC;
			handle_stuff();
		}
	}
}


static void natural_attack(s16b m_idx, int attack, bool *fear, bool *mdeath)
{
	int             k, bonus, chance;
	int             n_weight = 0;
	monster_type    *m_ptr = &m_list[m_idx];
	char            m_name[80];

	int             dss, ddd;

	char            *atk_desc;

	switch (attack)
	{
	case MUT2_SCOR_TAIL:
		dss = 3;
		ddd = 7;
		n_weight = 5;
		atk_desc = "tail";
		break;
	case MUT2_HORNS:
		dss = 2;
		ddd = 6;
		n_weight = 15;
		atk_desc = "horns";
		break;
	case MUT2_BEAK:
		dss = 2;
		ddd = 4;
		n_weight = 5;
		atk_desc = "beak";
		break;
	case MUT2_TRUNK:
		dss = 1;
		ddd = 4;
		n_weight = 35;
		atk_desc = "trunk";
		break;
	case MUT2_TENTACLES:
		dss = 2;
		ddd = 5;
		n_weight = 5;
		atk_desc = "tentacles";
		break;
	default:
		dss = ddd = n_weight = 1;
		atk_desc = "undefined body part";
	}

	/* Extract monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);


	/* Calculate the "attack quality" */
	bonus = p_ptr->to_h;
	chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));

	/* Test for hit */
        if (test_hit_norm(chance, m_ptr->ac, m_ptr->ml))
	{
		/* Sound */
		sound(SOUND_HIT);

		msg_format("You hit %s with your %s.", m_name, atk_desc);

		k = damroll(ddd, dss);
		k = critical_norm(n_weight, p_ptr->to_h, k);

		/* Apply the player damage bonuses */
		k += p_ptr->to_d;

		/* No negative damage */
		if (k < 0) k = 0;

		/* Complex message */
		if (wizard)
		{
			msg_format("You do %d (out of %d) damage.", k, m_ptr->hp);
		}

                switch (is_friend(m_ptr))
		{
                        case 1:
                                msg_format("%^s gets angry!", m_name);
                                change_side(m_ptr);
                                break;
                        case 0:
                                msg_format("%^s gets angry!", m_name);
                                m_ptr->status = MSTATUS_NEUTRAL_M;
                                break;
                }

		/* Damage, check for fear and mdeath */
		switch (attack)
		{
		case MUT2_SCOR_TAIL:
			project(0, 0, m_ptr->fy, m_ptr->fx, k, GF_POIS, PROJECT_KILL);
			break;
		case MUT2_HORNS:
			*mdeath = mon_take_hit(m_idx, k, fear, NULL);
			break;
		case MUT2_BEAK:
			*mdeath = mon_take_hit(m_idx, k, fear, NULL);
			break;
		case MUT2_TRUNK:
			*mdeath = mon_take_hit(m_idx, k, fear, NULL);
			break;
		case MUT2_TENTACLES:
			project(0, 0, m_ptr->fy, m_ptr->fx, k, GF_HELL_FIRE, PROJECT_KILL);
			break;
		default:
			*mdeath = mon_take_hit(m_idx, k, fear, NULL);
		}

		touch_zap_player(m_ptr);
	}
	/* Player misses */
	else
	{
		/* Sound */
		sound(SOUND_MISS);

		/* Message */
		msg_format("You miss %s.", m_name);
	}
}

/*
 * Carried monster can attack too.
 * Based on monst_attack_monst.
 */
static void carried_monster_attack(s16b m_idx, bool *fear, bool *mdeath, int x, int y)
{
        monster_type    *t_ptr = &m_list[m_idx];
        monster_race    *r_ptr;
        monster_race    *tr_ptr = race_inf(t_ptr);
        cave_type       *c_ptr;
	int             ap_cnt;
	int             ac, rlev,pt;
        char            t_name[80];
        char            temp[80];
	bool            blinked = FALSE, touched = FALSE;
	byte            y_saver = t_ptr->fy;
	byte            x_saver = t_ptr->fx;
        object_type     *o_ptr;


        /* Get the carried monster */
        o_ptr = &inventory[INVEN_CARRY];
        if(!o_ptr->k_idx) return;

        c_ptr = &cave[y][x];

        r_ptr = &r_info[o_ptr->pval];

	/* Not allowed to attack */
        if (r_ptr->flags1 & RF1_NEVER_BLOW) return;

	/* Total armor */
        ac = t_ptr->ac;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* Get the monster name (or "it") */
	monster_desc(t_name, t_ptr, 0);

	/* Assume no blink */
	blinked = FALSE;

        if (!t_ptr->ml)
	{
		msg_print("You hear noise.");
	}

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

		/* Stop attacking if the target dies! */
		if (t_ptr->fx != x_saver || t_ptr->fy != y_saver)
			break;

		/* Hack -- no more attacks */
		if (!method) break;

		if (blinked) /* Stop! */
		{
			/* break; */
		}

		/* Extract visibility (before blink) */
                visible = TRUE;

#ifdef MONSTER_LITE
                /* Extract visibility from carrying lite */
                if (r_ptr->flags9 & RF9_HAS_LITE) visible = TRUE;
#endif /* MONSTER_LITE */

		/* Extract the attack "power" */
		switch (effect)
		{
		case RBE_HURT:          power = 60; break;
		case RBE_POISON:        power =  5; break;
		case RBE_UN_BONUS:      power = 20; break;
		case RBE_UN_POWER:      power = 15; break;
		case RBE_EAT_GOLD:      power =  5; break;
		case RBE_EAT_ITEM:      power =  5; break;
		case RBE_EAT_FOOD:      power =  5; break;
		case RBE_EAT_LITE:      power =  5; break;
		case RBE_ACID:          power =  0; break;
		case RBE_ELEC:          power = 10; break;
		case RBE_FIRE:          power = 10; break;
		case RBE_COLD:          power = 10; break;
		case RBE_BLIND:         power =  2; break;
		case RBE_CONFUSE:       power = 10; break;
		case RBE_TERRIFY:       power = 10; break;
		case RBE_PARALYZE:      power =  2; break;
		case RBE_LOSE_STR:      power =  0; break;
		case RBE_LOSE_DEX:      power =  0; break;
		case RBE_LOSE_CON:      power =  0; break;
		case RBE_LOSE_INT:      power =  0; break;
		case RBE_LOSE_WIS:      power =  0; break;
		case RBE_LOSE_CHR:      power =  0; break;
		case RBE_LOSE_ALL:      power =  2; break;
		case RBE_SHATTER:       power = 60; break;
		case RBE_EXP_10:        power =  5; break;
		case RBE_EXP_20:        power =  5; break;
		case RBE_EXP_40:        power =  5; break;
		case RBE_EXP_80:        power =  5; break;
		case RBE_DISEASE:       power =  5; break;
		case RBE_TIME:          power =  5; break;
                case RBE_SANITY:        power = 60; break;
                case RBE_HALLU:         power = 10; break;
                case RBE_PARASITE:      power =  5; break;
		}


		/* Monster hits*/
		if (!effect || check_hit2(power, rlev,ac))
		{
			/* Always disturbing */
			disturb(1, 0);

			/* Describe the attack method */
			switch (method)
			{
			case RBM_HIT:
				{
					act = "hits %s.";
					touched = TRUE;
					break;
				}

			case RBM_TOUCH:
				{
					act = "touches %s.";
					touched = TRUE;
					break;
				}

			case RBM_PUNCH:
				{
					act = "punches %s.";
					touched = TRUE;
					break;
				}

			case RBM_KICK:
				{
					act = "kicks %s.";
					touched = TRUE;
					break;
				}

			case RBM_CLAW:
				{
					act = "claws %s.";
					touched = TRUE;
					break;
				}

			case RBM_BITE:
				{
					act = "bites %s.";
					touched = TRUE;
					break;
				}

			case RBM_STING:
				{
					act = "stings %s.";
					touched = TRUE;
					break;
				}

			case RBM_XXX1:
				{
					act = "XXX1's %s.";
					break;
				}

			case RBM_BUTT:
				{
					act = "butts %s.";
					touched = TRUE;
					break;
				}

			case RBM_CRUSH:
				{
					act = "crushes %s.";
					touched = TRUE;
					break;
				}

			case RBM_ENGULF:
				{
					act = "engulfs %s.";
					touched = TRUE;
					break;
				}

			case RBM_CHARGE:
				{
					act = "charges %s.";
					touched = TRUE;
					break;
				}

			case RBM_CRAWL:
				{
					act = "crawls on %s.";
					touched = TRUE;
					break;
				}

			case RBM_DROOL:
				{
					act = "drools on %s.";
					touched = FALSE;
					break;
				}

			case RBM_SPIT:
				{
					act = "spits on %s.";
					touched = FALSE;
					break;
				}

			case RBM_GAZE:
				{
					act = "gazes at %s.";
					touched = FALSE;
					break;
				}

			case RBM_WAIL:
				{
					act = "wails at %s.";
					touched = FALSE;
					break;
				}

			case RBM_SPORE:
				{
					act = "releases spores at %s.";
					touched = FALSE;
					break;
				}

			case RBM_XXX4:
				{
					act = "projects XXX4's at %s.";
					touched = FALSE;
					break;
				}

			case RBM_BEG:
				{
					act = "begs %s for money.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_INSULT:
				{
					act = "insults %s.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_MOAN:
				{
					act = "moans at %s.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_SHOW:
				{
					act = "sings to %s.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}
			}

			/* Message */
			if (act)
			{
				strfmt(temp,act,t_name);
                                if (t_ptr->ml)
                                        msg_format("Your monster %s", temp);

			}

			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage */
			damage = damroll(d_dice, d_side);

			pt = GF_MISSILE;

			/* Apply appropriate damage */
			switch (effect)
			{
			case 0:
				{
					damage = 0;
					pt  = 0;
					break;
				}

			case RBE_HURT:
                        case RBE_SANITY:
				{
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);
					break;
				}

			case RBE_POISON:
			case RBE_DISEASE:
				{
					pt = GF_POIS;
					break;
				}

			case RBE_UN_BONUS:
			case RBE_UN_POWER:
				{
					pt = GF_DISENCHANT;
					break;
				}

			case RBE_EAT_FOOD:
			case RBE_EAT_LITE:
				{
					pt = damage = 0;
					break;
				}

			case RBE_EAT_ITEM:
			case RBE_EAT_GOLD:
				{
					pt = damage = 0;
					if (randint(2)==1) blinked = TRUE;
					break;
				}

			case RBE_ACID:
				{
					pt = GF_ACID;
					break;
				}

			case RBE_ELEC:
				{
					pt = GF_ELEC;
					break;
				}

			case RBE_FIRE:
				{
					pt = GF_FIRE;
					break;
				}

			case RBE_COLD:
				{
					pt = GF_COLD;
					break;
				}

			case RBE_BLIND:
				{
					break;
				}

			case RBE_CONFUSE:
                        case RBE_HALLU:
				{
					pt = GF_CONFUSION;
					break;
				}

			case RBE_TERRIFY:
				{
					pt = GF_TURN_ALL;
					break;
				}

			case RBE_PARALYZE:
				{
					pt = GF_OLD_SLEEP; /* sort of close... */
					break;
				}

			case RBE_LOSE_STR:
			case RBE_LOSE_INT:
			case RBE_LOSE_WIS:
			case RBE_LOSE_DEX:
			case RBE_LOSE_CON:
			case RBE_LOSE_CHR:
			case RBE_LOSE_ALL:
                        case RBE_PARASITE:
				{
					break;
				}
			case RBE_SHATTER:
				{
					if (damage > 23)
					{
						/* Prevent destruction of quest levels and town */
						if (!is_quest(dun_level) && dun_level)
                                                        earthquake(py, px, 8);
					}
					break;
				}
			case RBE_EXP_10:
			case RBE_EXP_20:
			case RBE_EXP_40:
			case RBE_EXP_80:
				{
					pt = GF_NETHER;
					break;
				}
			case RBE_TIME:
				{
					pt = GF_TIME;
					break;
				}
			default:
				{
					pt = 0;
					break;
				}
			}

			if (pt)
			{
                                /* Do damage if not exploding */
                                project(0, 0, t_ptr->fy, t_ptr->fx,
                                        (pt == GF_OLD_SLEEP ? r_ptr->level : damage), pt, PROJECT_KILL | PROJECT_STOP);

				if (touched)
				{
					/* Aura fire */
					if ((tr_ptr->flags2 & RF2_AURA_FIRE) &&
						!(r_ptr->flags3 & RF3_IM_FIRE))
					{
                                                if (t_ptr->ml)
						{
							blinked = FALSE;
                                                        msg_format("You are suddenly very hot!");
							if(t_ptr->ml)
								tr_ptr->r_flags2 |= RF2_AURA_FIRE;
						}
                                                project(m_idx, 0, py, px,
                                                        damroll (1 + ((t_ptr->level) / 26),
                                                        1 + ((t_ptr->level) / 17)),
							GF_FIRE, PROJECT_KILL | PROJECT_STOP);
					}

					/* Aura elec */
					if ((tr_ptr->flags2 & (RF2_AURA_ELEC)) && !(r_ptr->flags3 & (RF3_IM_ELEC)))
					{
                                                if (t_ptr->ml)
						{
							blinked = FALSE;
                                                        msg_format("You get zapped!");
							if(t_ptr->ml)
								tr_ptr->r_flags2 |= RF2_AURA_ELEC;
						}
                                                project(m_idx, 0, py, px,
                                                        damroll (1 + ((t_ptr->level) / 26),
                                                        1 + ((t_ptr->level) / 17)),
							GF_ELEC, PROJECT_KILL | PROJECT_STOP);
					}

				}
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
			case RBM_CHARGE:
				{
						/* Disturbing */
						disturb(1, 0);

						/* Message */
                                                msg_format("Your monster misses %s.", t_name);
					break;
				}
			}
		}


		/* Analyze "visible" monsters only */
		if (visible)
		{
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (r_ptr->r_blows[ap_cnt] > 10))
			{
				/* Count attacks of this type */
				if (r_ptr->r_blows[ap_cnt] < MAX_UCHAR)
				{
					r_ptr->r_blows[ap_cnt]++;
				}
			}
		}
	}

	/* Blink away */
	if (blinked)
	{
                msg_print("You and your monster flee laughing!");

                teleport_player(MAX_SIGHT * 2 + 5);
	}
}

/*
 * Carried monster can attack too.
 * Based on monst_attack_monst.
 */
static void incarnate_monster_attack(s16b m_idx, bool *fear, bool *mdeath, int x, int y)
{
        monster_type    *t_ptr = &m_list[m_idx];
        monster_race    *r_ptr;
        monster_race    *tr_ptr = race_inf(t_ptr);
        cave_type       *c_ptr;
	int             ap_cnt;
	int             ac, rlev,pt;
        char            t_name[80];
        char            temp[80];
	bool            blinked = FALSE, touched = FALSE;
	byte            y_saver = t_ptr->fy;
	byte            x_saver = t_ptr->fx;


        if(!p_ptr->body_monster) return;

        c_ptr = &cave[y][x];

        r_ptr = race_info_idx(p_ptr->body_monster, 0);

	/* Not allowed to attack */
        if (r_ptr->flags1 & RF1_NEVER_BLOW) return;

	/* Total armor */
        ac = t_ptr->ac;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* Get the monster name (or "it") */
	monster_desc(t_name, t_ptr, 0);

	/* Assume no blink */
	blinked = FALSE;

        if (!t_ptr->ml)
	{
		msg_print("You hear noise.");
	}

	/* Scan through all four blows */
        for (ap_cnt = 0; ap_cnt < (p_ptr->num_blow > 4)?4:p_ptr->num_blow; ap_cnt++)
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

		/* Stop attacking if the target dies! */
		if (t_ptr->fx != x_saver || t_ptr->fy != y_saver)
			break;

		/* Hack -- no more attacks */
		if (!method) break;

		if (blinked) /* Stop! */
		{
			/* break; */
		}

		/* Extract visibility (before blink) */
                visible = TRUE;

#ifdef MONSTER_LITE
                /* Extract visibility from carrying lite */
                if (r_ptr->flags9 & RF9_HAS_LITE) visible = TRUE;
#endif /* MONSTER_LITE */

		/* Extract the attack "power" */
		switch (effect)
		{
		case RBE_HURT:          power = 60; break;
		case RBE_POISON:        power =  5; break;
		case RBE_UN_BONUS:      power = 20; break;
		case RBE_UN_POWER:      power = 15; break;
		case RBE_EAT_GOLD:      power =  5; break;
		case RBE_EAT_ITEM:      power =  5; break;
		case RBE_EAT_FOOD:      power =  5; break;
		case RBE_EAT_LITE:      power =  5; break;
		case RBE_ACID:          power =  0; break;
		case RBE_ELEC:          power = 10; break;
		case RBE_FIRE:          power = 10; break;
		case RBE_COLD:          power = 10; break;
		case RBE_BLIND:         power =  2; break;
		case RBE_CONFUSE:       power = 10; break;
		case RBE_TERRIFY:       power = 10; break;
		case RBE_PARALYZE:      power =  2; break;
		case RBE_LOSE_STR:      power =  0; break;
		case RBE_LOSE_DEX:      power =  0; break;
		case RBE_LOSE_CON:      power =  0; break;
		case RBE_LOSE_INT:      power =  0; break;
		case RBE_LOSE_WIS:      power =  0; break;
		case RBE_LOSE_CHR:      power =  0; break;
		case RBE_LOSE_ALL:      power =  2; break;
		case RBE_SHATTER:       power = 60; break;
		case RBE_EXP_10:        power =  5; break;
		case RBE_EXP_20:        power =  5; break;
		case RBE_EXP_40:        power =  5; break;
		case RBE_EXP_80:        power =  5; break;
		case RBE_DISEASE:       power =  5; break;
		case RBE_TIME:          power =  5; break;
                case RBE_SANITY:        power = 60; break;
                case RBE_HALLU:         power = 10; break;
                case RBE_PARASITE:      power =  5; break;
		}


		/* Monster hits*/
		if (!effect || check_hit2(power, rlev,ac))
		{
			/* Always disturbing */
			disturb(1, 0);

			/* Describe the attack method */
			switch (method)
			{
			case RBM_HIT:
				{
                                        act = "hit %s.";
					touched = TRUE;
					break;
				}

			case RBM_TOUCH:
				{
                                        act = "touch %s.";
					touched = TRUE;
					break;
				}

			case RBM_PUNCH:
				{
                                        act = "punch %s.";
					touched = TRUE;
					break;
				}

			case RBM_KICK:
				{
                                        act = "kick %s.";
					touched = TRUE;
					break;
				}

			case RBM_CLAW:
				{
                                        act = "claw %s.";
					touched = TRUE;
					break;
				}

			case RBM_BITE:
				{
                                        act = "bite %s.";
					touched = TRUE;
					break;
				}

			case RBM_STING:
				{
                                        act = "sting %s.";
					touched = TRUE;
					break;
				}

			case RBM_XXX1:
				{
					act = "XXX1's %s.";
					break;
				}

			case RBM_BUTT:
				{
                                        act = "butt %s.";
					touched = TRUE;
					break;
				}

			case RBM_CRUSH:
				{
                                        act = "crush %s.";
					touched = TRUE;
					break;
				}

			case RBM_ENGULF:
				{
                                        act = "engulf %s.";
					touched = TRUE;
					break;
				}

			case RBM_CHARGE:
				{
                                        act = "charge %s.";
					touched = TRUE;
					break;
				}

			case RBM_CRAWL:
				{
                                        act = "crawl on %s.";
					touched = TRUE;
					break;
				}

			case RBM_DROOL:
				{
                                        act = "drool on %s.";
					touched = FALSE;
					break;
				}

			case RBM_SPIT:
				{
                                        act = "spit on %s.";
					touched = FALSE;
					break;
				}

			case RBM_GAZE:
				{
                                        act = "gaze at %s.";
					touched = FALSE;
					break;
				}

			case RBM_WAIL:
				{
                                        act = "wail at %s.";
					touched = FALSE;
					break;
				}

			case RBM_SPORE:
				{
                                        act = "release spores at %s.";
					touched = FALSE;
					break;
				}

			case RBM_XXX4:
				{
                                        act = "project XXX4's at %s.";
					touched = FALSE;
					break;
				}

			case RBM_BEG:
				{
                                        act = "beg %s for money.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_INSULT:
				{
                                        act = "insult %s.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_MOAN:
				{
                                        act = "moan at %s.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_SHOW:
				{
                                        act = "sing to %s.";
					touched = FALSE;
					t_ptr->csleep = 0;
					break;
				}
			}

			/* Message */
			if (act)
			{
				strfmt(temp,act,t_name);
                                if (t_ptr->ml)
                                        msg_format("You %s", temp);

			}

			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage */
                        damage = damroll(d_dice, d_side) + p_ptr->to_d;

			pt = GF_MISSILE;

			/* Apply appropriate damage */
			switch (effect)
			{
			case 0:
				{
					damage = 0;
					pt  = 0;
					break;
				}

			case RBE_HURT:
                        case RBE_SANITY:
				{
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);
					break;
				}

			case RBE_POISON:
			case RBE_DISEASE:
				{
					pt = GF_POIS;
					break;
				}

			case RBE_UN_BONUS:
			case RBE_UN_POWER:
				{
					pt = GF_DISENCHANT;
					break;
				}

			case RBE_EAT_FOOD:
			case RBE_EAT_LITE:
				{
					pt = damage = 0;
					break;
				}

			case RBE_EAT_ITEM:
			case RBE_EAT_GOLD:
				{
					pt = damage = 0;
					if (randint(2)==1) blinked = TRUE;
					break;
				}

			case RBE_ACID:
				{
					pt = GF_ACID;
					break;
				}

			case RBE_ELEC:
				{
					pt = GF_ELEC;
					break;
				}

			case RBE_FIRE:
				{
					pt = GF_FIRE;
					break;
				}

			case RBE_COLD:
				{
					pt = GF_COLD;
					break;
				}

			case RBE_BLIND:
				{
					break;
				}

                        case RBE_HALLU:
			case RBE_CONFUSE:
				{
					pt = GF_CONFUSION;
					break;
				}

			case RBE_TERRIFY:
				{
					pt = GF_TURN_ALL;
					break;
				}

			case RBE_PARALYZE:
				{
					pt = GF_OLD_SLEEP; /* sort of close... */
					break;
				}

			case RBE_LOSE_STR:
			case RBE_LOSE_INT:
			case RBE_LOSE_WIS:
			case RBE_LOSE_DEX:
			case RBE_LOSE_CON:
			case RBE_LOSE_CHR:
			case RBE_LOSE_ALL:
                        case RBE_PARASITE:
				{
					break;
				}
			case RBE_SHATTER:
				{
					if (damage > 23)
					{
						/* Prevent destruction of quest levels and town */
						if (!is_quest(dun_level) && dun_level)
                                                        earthquake(py, px, 8);
					}
					break;
				}
			case RBE_EXP_10:
			case RBE_EXP_20:
			case RBE_EXP_40:
			case RBE_EXP_80:
				{
					pt = GF_NETHER;
					break;
				}
			case RBE_TIME:
				{
					pt = GF_TIME;
					break;
				}
			default:
				{
					pt = 0;
					break;
				}
			}

			if (pt)
			{
                                /* Do damage if not exploding */
                                project(0, 0, t_ptr->fy, t_ptr->fx,
                                        (pt == GF_OLD_SLEEP ? p_ptr->lev * 2 : damage), pt, PROJECT_KILL | PROJECT_STOP);

				if (touched)
				{
					/* Aura fire */
					if ((tr_ptr->flags2 & RF2_AURA_FIRE) &&
						!(r_ptr->flags3 & RF3_IM_FIRE))
					{
                                                if (t_ptr->ml)
						{
							blinked = FALSE;
                                                        msg_format("You are suddenly very hot!");
							if(t_ptr->ml)
								tr_ptr->r_flags2 |= RF2_AURA_FIRE;
						}
                                                project(m_idx, 0, py, px,
                                                        damroll (1 + ((t_ptr->level) / 26),
                                                        1 + ((t_ptr->level) / 17)),
							GF_FIRE, PROJECT_KILL | PROJECT_STOP);
					}

					/* Aura elec */
					if ((tr_ptr->flags2 & (RF2_AURA_ELEC)) && !(r_ptr->flags3 & (RF3_IM_ELEC)))
					{
                                                if (t_ptr->ml)
						{
							blinked = FALSE;
                                                        msg_format("You get zapped!");
							if(t_ptr->ml)
								tr_ptr->r_flags2 |= RF2_AURA_ELEC;
						}
                                                project(m_idx, 0, py, px,
                                                        damroll (1 + ((t_ptr->level) / 26),
                                                        1 + ((t_ptr->level) / 17)),
							GF_ELEC, PROJECT_KILL | PROJECT_STOP);
					}

				}
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
			case RBM_CHARGE:
				{
						/* Disturbing */
						disturb(1, 0);

						/* Message */
                                                msg_format("You miss %s.", t_name);
					break;
				}
			}
		}


		/* Analyze "visible" monsters only */
		if (visible)
		{
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (r_ptr->r_blows[ap_cnt] > 10))
			{
				/* Count attacks of this type */
				if (r_ptr->r_blows[ap_cnt] < MAX_UCHAR)
				{
					r_ptr->r_blows[ap_cnt]++;
				}
			}
		}
	}

	/* Blink away */
	if (blinked)
	{
                msg_print("You flee laughing!");

                teleport_player(MAX_SIGHT * 2 + 5);
	}
}

/*
 * Fetch an attack description from dam_*.txt files.
 */

static void flavored_attack(int percent, char* output) {
  if (percent < 5) {
    if (!flavored_attacks) strcpy(output, "You scratch %s.");
    else get_rnd_line("dam_none.txt", output);

  } else if (percent < 30) {
    if (!flavored_attacks) strcpy(output, "You hit %s.");
    else get_rnd_line("dam_med.txt", output);

  } else if (percent < 60) {
    if (!flavored_attacks) strcpy(output, "You wound %s.");
    else get_rnd_line("dam_lots.txt", output);

  } else if (percent < 95) {
    if (!flavored_attacks) strcpy(output, "You cripple %s.");
    else get_rnd_line("dam_huge.txt", output);

  } else {
    if (!flavored_attacks) strcpy(output, "You demolish %s.");
    else get_rnd_line("dam_xxx.txt", output);

  }

}

/* Monk bare handed attacks */
void py_attack_monk(int *k, monster_type *m_ptr)
{
        int special_effect = 0, stun_effect = 0, times = 0;
        martial_arts * ma_ptr = &ma_blows[0], * old_ptr = &ma_blows[0];
        int resist_stun = 0;
        monster_race *r_ptr = race_inf(m_ptr);
        char m_name[80];

	/* Extract monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

        if (r_ptr->flags1 & RF1_UNIQUE) resist_stun += 88;
        if (r_ptr->flags3 & RF3_NO_CONF) resist_stun += 44;
        if (r_ptr->flags3 & RF3_NO_SLEEP) resist_stun += 44;
        if ((r_ptr->flags3 & RF3_UNDEAD) || (r_ptr->flags3 & RF3_NONLIVING)) resist_stun += 88;

        /* Attempt 'times' */
        for (times = 0; times < (p_ptr->lev<7?1:p_ptr->lev/7); times++)
        {
                do
                {
                        ma_ptr = &ma_blows[(randint(MAX_MA))-1];
                }

                while ((ma_ptr->min_level > p_ptr->lev) || (randint(p_ptr->lev) < ma_ptr->chance));

                /* keep the highest level attack available we found */
                if ((ma_ptr->min_level > old_ptr->min_level) && !(p_ptr->stun || p_ptr->confused))
                {
                        old_ptr = ma_ptr;

                        if (wizard && cheat_xtra)
                        {
                                msg_print("Attack re-selected.");
                        }
                }
                else
                {
                        ma_ptr = old_ptr;
                }
        }

        *k = damroll(ma_ptr->dd, ma_ptr->ds);

        if (ma_ptr->effect == MA_KNEE)
        {
                if (r_ptr->flags1 & RF1_MALE)
                {
                        msg_format("You hit %s in the groin with your knee!", m_name);
                        sound(SOUND_PAIN);
                        special_effect = MA_KNEE;
                }
                else
                        msg_format(ma_ptr->desc, m_name);
        }
        else if (ma_ptr->effect == MA_SLOW)
        {
                if (!((r_ptr->flags1 & RF1_NEVER_MOVE) || strchr("UjmeEv$,DdsbBFIJQSXclnw!=?", r_ptr->d_char)))
                {
                        msg_format("You kick %s in the ankle.", m_name);
                        special_effect = MA_SLOW;
                }
                else msg_format(ma_ptr->desc, m_name);
        }
        else
        {
                if (ma_ptr->effect)
                {
                        stun_effect = (ma_ptr->effect/2) + randint(ma_ptr->effect/2);
                }

                msg_format(ma_ptr->desc, m_name);
        }

        *k = critical_norm(p_ptr->lev * (randint(10)), ma_ptr->min_level, *k);

        if ((special_effect == MA_KNEE) && ((*k + p_ptr->to_d) < m_ptr->hp))
        {
                msg_format("%^s moans in agony!", m_name);
                stun_effect = 7 + randint(13);
                resist_stun /= 3;
        }
        else if ((special_effect == MA_SLOW) && ((*k + p_ptr->to_d) < m_ptr->hp))
        {
                if (!(r_ptr->flags1 & RF1_UNIQUE) &&
                        (randint(p_ptr->lev) > m_ptr->level) &&
                        m_ptr->mspeed > 60)
                {
                        msg_format("%^s starts limping slower.", m_name);
                        m_ptr->mspeed -= 10;
                }
        }

        if (stun_effect && ((*k + p_ptr->to_d) < m_ptr->hp))
        {
                if (p_ptr->lev > randint(m_ptr->level + resist_stun + 10))
                {
                        if (m_ptr->stunned)
                                msg_format("%^s is still stunned.", m_name);
                        else
                                msg_format("%^s is stunned.", m_name);

                        m_ptr->stunned += (stun_effect);
                }
        }
}

/* Apply nazgul effects */
void do_nazgul(int *k, int *num, int num_blow, int weap, monster_race *r_ptr, object_type *o_ptr)
{
        u32b f1, f2, f3, f4, f5, esp;

        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

        /* Mega Hack -- Hitting Nazgul is REALY dangerous(ideas from Akhronath) */
        if (r_ptr->flags7 & RF7_NAZGUL)
        {
                if ((!o_ptr->name2) && (!artifact_p(o_ptr)))
                {
                        msg_print("Your weapon *DISINTEGRATES*!");
                        *k = 0;
                        inven_item_increase(INVEN_WIELD + weap, -1);
                        inven_item_optimize(INVEN_WIELD + weap);

                        /* To stop attacking */
                        *num = num_blow;
                }
                else if (o_ptr->name2)
                {
                        if (!(f1 & TR1_SLAY_EVIL) && !(f1 & TR1_SLAY_UNDEAD))
                        {
                                msg_print("The Ringwraith is IMPERVIOUS to the mundane weapon.");
                                *k = 0;
                        }

                        /* 25% chance of getting destroyed */
                        if (magik(25))
                        {
                                msg_print("Your weapon is destroyed !");
                                inven_item_increase(INVEN_WIELD + weap, -1);
                                inven_item_optimize(INVEN_WIELD + weap);

                                /* To stop attacking */
                                *num = num_blow;
                        }
                }
                else if (artifact_p(o_ptr))
                {
                        if (!(f1 & TR1_SLAY_EVIL) && !(f1 & TR1_SLAY_UNDEAD))
                        {
                                msg_print("The Ringwraith is IMPERVIOUS to the mundane weapon.");
                                *k = 0;
                        }

                        apply_disenchant(INVEN_WIELD + weap);

                        /* 1/1000 chance of getting destroyed */
                        if (!rand_int(1000))
                        {
                                msg_print("Your weapon is destroyed !");
                                inven_item_increase(INVEN_WIELD + weap, -1);
                                inven_item_optimize(INVEN_WIELD + weap);

                                /* To stop attacking */
                                *num = num_blow;
                        }
                }

                /* If any damage is done, then 25% chance of getting the Black Breath */
                if (*k)
                {
                        if (magik(25))
                        {
                                msg_print("Your foe calls upon your soul!");
                                msg_print("You feel the Black Breath slowly draining you of life...");
                                p_ptr->black_breath = TRUE;
                        }
                }
        }
}

/*
 * Apply the special effects of an attack
 */
void attack_special(monster_type *m_ptr, byte special, int dam)
{
	char m_name[80];
	
        monster_race *r_ptr = race_inf(m_ptr);

	/* Extract monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Special - Cut monster */
        if (special & SPEC_CUT)
	{
#if 0 // DGDGDG
		/* Cut the monster */
		if (r_ptr->flags3 & (RF3_NO_CUT))
		{
			if (m_ptr->ml)
			{
				l_ptr->r_flags3 |= (RF3_NO_CUT);
			}
		}
                else
#endif
                if (rand_int(100) >= r_ptr->level)
		{
			/* Already partially poisoned */
                        if (m_ptr->bleeding) msg_format("%^s is bleeding more strongly.", m_name);
			/* Was not poisoned */
                        else msg_format("%^s is bleeding.", m_name);

			m_ptr->bleeding += dam*2;
		}
	}

	/* Special - Poison monster */
	if (special & SPEC_POIS) 
	{
		/* Poison the monster */
                if (r_ptr->flags3 & (RF3_IM_POIS))
                {
                        if (m_ptr->ml)
                        {
                                r_ptr->r_flags3 |= (RF3_IM_POIS);
                        }
                }
                /* Notice susceptibility */
                else if (r_ptr->flags9 & (RF9_SUSCEP_POIS))
                {
                        if (m_ptr->ml)
                        {
                                r_ptr->r_flags9 |= (RF9_SUSCEP_POIS);
                        }
			/* Already partially poisoned */
                        if (m_ptr->poisoned) msg_format("%^s is more poisoned.", m_name);
			/* Was not poisoned */
                        else msg_format("%^s is poisoned.", m_name);

                        m_ptr->poisoned += dam * 2;
                }
		else if (rand_int(100) >= r_ptr->level)
		{
			/* Already partially poisoned */
                        if (m_ptr->poisoned) msg_format("%^s is more poisoned.", m_name);
			/* Was not poisoned */
                        else msg_format("%^s is poisoned.", m_name);

			m_ptr->poisoned += dam;
		}
	}
}

/*
 * Player attacks a (poor, defenseless) creature        -RAK-
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
void py_attack(int y, int x, int max_blow)
{
	int		num = 0, k, bonus, chance;
        s32b            special = 0;

	cave_type       *c_ptr = &cave[y][x];

	monster_type    *m_ptr = &m_list[c_ptr->m_idx];
        monster_race    *r_ptr = race_inf(m_ptr);

	object_type     *o_ptr;

	char            m_name[80];


	bool            fear = FALSE;
	bool            mdeath = FALSE;

	bool            backstab = FALSE;
	bool            vorpal_cut = FALSE;
	int             chaos_effect = 0;
	bool            stab_fleeing = FALSE;
	bool            do_quake = FALSE;
	bool            drain_msg = TRUE;
	int             drain_result = 0, drain_heal = 0;
	int             drain_left = MAX_VAMPIRIC_DRAIN;
        u32b            f1, f2, f3, f4, f5, esp; /* A massive hack -- life-draining weapons */
	bool            no_extra = FALSE;
        int             weap;


	/* Disturb the player */
	disturb(0, 0);

        if (r_info[p_ptr->body_monster].flags1 & RF1_NEVER_BLOW)
        {
                msg_print("You cannot attack in this form!");
                return;
        }

	if (p_ptr->pclass == CLASS_ROGUE)
	{
		if ((m_ptr->csleep) && (m_ptr->ml))
		{
			/* Can't backstab creatures that we can't see, right? */
			backstab = TRUE;
		}
		else if ((m_ptr->monfear) && (m_ptr->ml))
		{
			stab_fleeing = TRUE;
		}
	}

	/* Disturb the monster */
	m_ptr->csleep = 0;


	/* Extract monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Auto-Recall if possible and visible */
        if (m_ptr->ml) monster_race_track(m_ptr->r_idx, m_ptr->ego);

	/* Track a new monster */
	if (m_ptr->ml) health_track(c_ptr->m_idx);

	/* Stop if friendly */
        if ((is_friend(m_ptr) >= 0) &&
	    ! (p_ptr->stun || p_ptr->confused || p_ptr->image ||
	    ((p_ptr->muta2 & MUT2_BERS_RAGE) && p_ptr->shero) ||
	    !(m_ptr->ml)))
	{
		if (!(inventory[INVEN_WIELD].art_name))
		{
			msg_format("You stop to avoid hitting %s.", m_name);
			return;
		}

		if (!(streq(quark_str(inventory[INVEN_WIELD].art_name), "'Stormbringer'")))
		{
			msg_format("You stop to avoid hitting %s.", m_name);
			return;
		}

		msg_format("Your black blade greedily attacks %s!", m_name);
	}


	/* Handle player fear */
	if (p_ptr->afraid)
	{
		/* Message */
		if (m_ptr->ml)
			msg_format("You are too afraid to attack %s!", m_name);
		else
			msg_format ("There is something scary in your way!");

		/* Done */
		return;
	}

        /* Monsters that can dont use weapons, us etheir natural attacks */
        if ((p_ptr->body_monster) && (!r_info[p_ptr->body_monster].body_parts[BODY_WEAPON]))
        {
                incarnate_monster_attack(c_ptr->m_idx, &fear, &mdeath, y, x);
        }
        /* Otherwise use your weapon(s) */
        else
        {

        /* Attack with ALL the weapons !!!!! -- ooh that's gonna hurt YOU */
        for (weap = 0; weap < r_info[p_ptr->body_monster].body_parts[BODY_WEAPON]; weap++)
        {
        /* Monster is already dead ? oh :( */
        if (mdeath) break;

        /* Reset the blows counter */
        num = 0;

	/* Access the weapon */
        o_ptr = &inventory[INVEN_WIELD + weap];

	/* Calculate the "attack quality" */
	bonus = p_ptr->to_h + o_ptr->to_h;
	chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));

        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

        if(!(f4 & TR4_NEVER_BLOW))
        {
        int num_blow = p_ptr->num_blow;

        /* Restrict to max_blow(if max_blow >= 0) */
        if((max_blow >= 0) && (num_blow > max_blow)) num_blow = max_blow;

	/* Attack once for each legal blow */
        while (num++ < num_blow)
	{
		/* Test for hit */
                if (test_hit_norm(chance, m_ptr->ac, m_ptr->ml))
		{
			/* Sound */
			sound(SOUND_HIT);

			/* Hack -- bare hands do one damage */
			k = 1;

			/* Select a chaotic effect (50% chance) */
			if ((f1 & TR1_CHAOTIC) && (randint(2)==1))
			{
				if (randint(5) < 3)
				{
					/* Vampiric (20%) */
					chaos_effect = 1;
				}
                                else if (randint(250) == 1)
				{
					/* Quake (0.12%) */
					chaos_effect = 2;
				}
				else if (randint(10) != 1)
				{
					/* Confusion (26.892%) */
					chaos_effect = 3;
				}
				else if (randint(2) == 1)
				{
					/* Teleport away (1.494%) */
					chaos_effect = 4;
				}
				else
				{
					/* Polymorph (1.494%) */
					chaos_effect = 5;
				}
			}

			/* Vampiric drain */
			if ((f1 & TR1_VAMPIRIC) || (chaos_effect == 1))
			{
				if (!((r_ptr->flags3 & RF3_UNDEAD) || (r_ptr->flags3 & RF3_NONLIVING)))
					drain_result = m_ptr->hp;
				else
					drain_result = 0;
			}

                        if (f1 & TR1_VORPAL && (randint(6) == 1))
				vorpal_cut = TRUE;
			else vorpal_cut = FALSE;

			if (p_ptr->pclass == CLASS_MONK && monk_empty_hands())
			{
                                py_attack_monk(&k, m_ptr);
			}

			/* Handle normal weapon */
			else if (o_ptr->k_idx)
			{
				k = damroll(o_ptr->dd, o_ptr->ds);
                                k = tot_dam_aux(o_ptr, k, m_ptr, &special);

				if (backstab)
				{
					k *= (3 + (p_ptr->lev / 40));
				}
				else if (stab_fleeing)
				{
					k = ((3 * k) / 2);
				}

				if ((p_ptr->impact && ((k > 50) || randint(7)==1)) ||
				    (chaos_effect == 2))
				{
					do_quake = TRUE;
				}

				k = critical_norm(o_ptr->weight, o_ptr->to_h, k);

				if (vorpal_cut)
				{
					int step_k = k;

                                        msg_format("Your weapon cuts deep into %s!", m_name);
					do
					{
						k += step_k;
					}
                                        while (randint(4) == 1);
				}

				k += o_ptr->to_d;

                                do_nazgul(&k, &num, num_blow, weap, r_ptr, o_ptr);
			}

                        /* May it clone the monster ? */
                        if ((f4 & TR4_CLONE) && magik(30))
			{
                                msg_format("Oh no ! Your weapon clones %^s!", m_name);
                                multiply_monster(c_ptr->m_idx, FALSE, TRUE);
			}

			/* Apply the player damage bonuses */
			k += p_ptr->to_d;

                        /* Penalty for could-2H when having a shield */
                        if ((f4 & TR4_COULD2H) && inventory[INVEN_ARM].k_idx) k /= 2;

			/* No negative damage */
			if (k < 0) k = 0;

			/* Message */
			if (!(backstab || stab_fleeing))
			{
				if (!(p_ptr->pclass == CLASS_MONK && monk_empty_hands()))
                                {
                                        if (strchr("vwjmelX,.*", r_ptr->d_char)) {
                                          msg_format("You hit %s.", m_name);
                                        } else {
                                          char buff[255];

                                          flavored_attack((100*k)/m_ptr->maxhp, buff);
                                          msg_format(buff, m_name);
                                        }
                                }
			}
			else if (backstab)
                        {
                                char buf[80];

                                monster_race_desc(buf, m_ptr->r_idx, m_ptr->ego);

                                backstab = FALSE;

				msg_format("You cruelly stab the helpless, sleeping %s!",
                                    buf);
                        }
                        else
                        {
                                char buf[80];

                                monster_race_desc(buf, m_ptr->r_idx, m_ptr->ego);

				msg_format("You backstab the fleeing %s!",
                                    buf);
                        }

			/* Complex message */
			if (wizard)
			{
				msg_format("You do %d (out of %d) damage.", k, m_ptr->hp);
			}

                        if (special) attack_special(m_ptr, special, k);

			/* Damage, check for fear and death */
			if (mon_take_hit(c_ptr->m_idx, k, &fear, NULL))
			{
                                /* Hack -- High-level warriors can spread their attacks out
                                 * among weaker foes.
                                 */
                                if ((p_ptr->pclass == CLASS_WARRIOR) &&
                                        (p_ptr->lev > 34) && (num < num_blow) &&
                                        (energy_use))
                                {
                                        energy_use = energy_use * num / num_blow;
                                }
				mdeath = TRUE;
				break;
			}

                        switch (is_friend(m_ptr))
                        {
                                case 1:
                                        msg_format("%^s gets angry!", m_name);
                                        change_side(m_ptr);
                                        break;
                                case 0:
                                        msg_format("%^s gets angry!", m_name);
                                        m_ptr->status = MSTATUS_NEUTRAL_M;
                                        break;
                        }

			touch_zap_player(m_ptr);

			/* Are we draining it?  A little note: If the monster is
			dead, the drain does not work... */

			if (drain_result)
			{
				drain_result -= m_ptr->hp;  /* Calculate the difference */

				if (drain_result > 0) /* Did we really hurt it? */
				{
					drain_heal = damroll(4,(drain_result / 6));

					if (cheat_xtra)
					{
						msg_format("Draining left: %d", drain_left);
					}

					if (drain_left)
					{
						if (drain_heal < drain_left)
						{
							drain_left -= drain_heal;
						}
						else
						{
							drain_heal = drain_left;
							drain_left = 0;
						}

						if (drain_msg)
						{
							msg_format("Your weapon drains life from %s!", m_name);
							drain_msg = FALSE;
						}

						hp_player(drain_heal);
						/* We get to keep some of it! */
					}
				}
			}

			/* Confusion attack */
			if ((p_ptr->confusing) || (chaos_effect == 3))
			{
				/* Cancel glowing hands */
				if (p_ptr->confusing)
				{
					p_ptr->confusing = FALSE;
					msg_print("Your hands stop glowing.");
				}

				/* Confuse the monster */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (m_ptr->ml)
					{
						r_ptr->r_flags3 |= (RF3_NO_CONF);
					}

					msg_format("%^s is unaffected.", m_name);
				}
                                else if (rand_int(100) < m_ptr->level)
				{
					msg_format("%^s is unaffected.", m_name);
				}
				else
				{
					msg_format("%^s appears confused.", m_name);
					m_ptr->confused += 10 + rand_int(p_ptr->lev) / 5;
				}
			}

			else if (chaos_effect == 4)
			{
				msg_format("%^s disappears!", m_name);
				teleport_away(c_ptr->m_idx, 50);
                                num = num_blow + 1; /* Can't hit it anymore! */
				no_extra = TRUE;
			}

			else if ((chaos_effect == 5) && cave_floor_bold(y,x)
                            && (randint(90) > m_ptr->level))
			{
				if (!((r_ptr->flags1 & RF1_UNIQUE) ||
				      (r_ptr->flags4 & RF4_BR_CHAO) ||
				      (r_ptr->flags1 & RF1_QUESTOR)))
				{
					int tmp = poly_r_idx(m_ptr->r_idx);

					/* Pick a "new" monster race */

					/* Handle polymorph */
					if (tmp != m_ptr->r_idx)
					{
						msg_format("%^s changes!", m_name);

						/* "Kill" the "old" monster */
						delete_monster_idx(c_ptr->m_idx);

						/* Create a new monster (no groups) */
                                                (void)place_monster_aux(y, x, tmp, FALSE, FALSE, MSTATUS_ENEMY);

						/* XXX XXX XXX Hack -- Assume success */

						/* Hack -- Get new monster */
						m_ptr = &m_list[c_ptr->m_idx];

						/* Oops, we need a different name... */
						monster_desc(m_name, m_ptr, 0);

						/* Hack -- Get new race */
                                                r_ptr = race_inf(m_ptr);

						fear = FALSE;

					}
				}
				else
					msg_format("%^s is unaffected.", m_name);
			}
		}

		/* Player misses */
		else
		{
			/* Sound */
			sound(SOUND_MISS);

			backstab = FALSE; /* Clumsy! */

			/* Message */
			msg_format("You miss %s.", m_name);
		}
	}
        }
        else
        {
                msg_print("You can't attack with that weapon.");
        }
        }
        }

	/* Mutations which yield extra 'natural' attacks */
	if (!no_extra)
	{
		if (p_ptr->muta2 & MUT2_HORNS && !mdeath)
			natural_attack(c_ptr->m_idx, MUT2_HORNS, &fear, &mdeath);
		if (p_ptr->muta2 & MUT2_BEAK && !mdeath)
			natural_attack(c_ptr->m_idx, MUT2_BEAK, &fear, &mdeath);
		if (p_ptr->muta2 & MUT2_SCOR_TAIL && !mdeath)
			natural_attack(c_ptr->m_idx, MUT2_SCOR_TAIL, &fear, &mdeath);
		if (p_ptr->muta2 & MUT2_TRUNK && !mdeath)
			natural_attack(c_ptr->m_idx, MUT2_TRUNK, &fear, &mdeath);
		if (p_ptr->muta2 & MUT2_TENTACLES && !mdeath)
			natural_attack(c_ptr->m_idx, MUT2_TENTACLES, &fear, &mdeath);
	}

        /* Carried monster can attack too */
        if (m_list[c_ptr->m_idx].hp)
                carried_monster_attack(c_ptr->m_idx,&fear,&mdeath,y,x);

	/* Hack -- delay fear messages */
	if (fear && m_ptr->ml)
	{
		/* Sound */
		sound(SOUND_FLEE);

		/* Message */
		msg_format("%^s flees in terror!", m_name);
	}

	/* Mega-Hack -- apply earthquake brand */
	if (do_quake)
	{
		/* Prevent destruction of quest levels and town */
		if (!is_quest(dun_level) && dun_level)
			earthquake(py, px, 10);
	}
}



static bool pattern_tile(int y, int x)
{
	return ((cave[y][x].feat <= FEAT_PATTERN_XTRA2) &&
	    (cave[y][x].feat >= FEAT_PATTERN_START));
}


static bool pattern_seq(int c_y, int c_x, int n_y, int n_x)
{
	if (!(pattern_tile(c_y, c_x)) && !(pattern_tile(n_y, n_x)))
		return TRUE;

	if (cave[n_y][n_x].feat == FEAT_PATTERN_START)
	{
		if ((!(pattern_tile(c_y, c_x))) &&
		    !(p_ptr->confused || p_ptr->stun || p_ptr->image))
		{
                        if (get_check("If you start walking the Straight Road, you must walk the whole way. Ok? "))
				return TRUE;
			else
				return FALSE;
		}
		else
			return TRUE;
	}
	else if ((cave[n_y][n_x].feat == FEAT_PATTERN_OLD) ||
	    (cave[n_y][n_x].feat == FEAT_PATTERN_END) ||
	    (cave[n_y][n_x].feat == FEAT_PATTERN_XTRA2))
	{
		if (pattern_tile(c_y, c_x))
		{
			return TRUE;
		}
		else
		{
                        msg_print("You must start walking the Straight Road from the startpoint.");
			return FALSE;
		}
	}
	else if ((cave[n_y][n_x].feat == FEAT_PATTERN_XTRA1)||
	    (cave[c_y][c_x].feat == FEAT_PATTERN_XTRA1))
	{
		return TRUE;
	}
	else if (cave[c_y][c_x].feat == FEAT_PATTERN_START)
	{
		if (pattern_tile(n_y, n_x))
			return TRUE;
		else
		{
                        msg_print("You must walk the Straight Road in correct order.");
			return FALSE;
		}
	}
	else if ((cave[c_y][c_x].feat == FEAT_PATTERN_OLD) ||
	    (cave[c_y][c_x].feat == FEAT_PATTERN_END) ||
	    (cave[c_y][c_x].feat == FEAT_PATTERN_XTRA2))
	{
		if (!pattern_tile(n_y, n_x))
		{
                        msg_print("You may not step off from the Straight Road.");
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		if (!pattern_tile(c_y, c_x))
		{
                        msg_print("You must start walking the Straight Road from the startpoint.");
			return FALSE;
		}
		else
		{
			byte ok_move = FEAT_PATTERN_START;
			switch (cave[c_y][c_x].feat)
			{
				case FEAT_PATTERN_1:
					ok_move = FEAT_PATTERN_2;
					break;
				case FEAT_PATTERN_2:
					ok_move = FEAT_PATTERN_3;
					break;
				case FEAT_PATTERN_3:
					ok_move = FEAT_PATTERN_4;
					break;
				case FEAT_PATTERN_4:
					ok_move = FEAT_PATTERN_1;
					break;
				default:
					if (wizard)
                                                msg_format("Funny Straight Road walking, %d.", cave[c_y][c_x]);
					return TRUE; /* Goof-up */
			}

			if ((cave[n_y][n_x].feat == ok_move) ||
			    (cave[n_y][n_x].feat == cave[c_y][c_x].feat))
				return TRUE;
			else
			{
				if (!pattern_tile(n_y, n_x))
                                        msg_print("You may not step off from the Straight Road.");
				else
                                        msg_print("You must walk the Straight Road in correct order.");

				return FALSE;
			}
		}
	}
}



bool player_can_enter(byte feature)
{
	bool pass_wall;
        bool only_wall = FALSE;

	/* Player can not walk through "walls" unless in Shadow Form */
        if (p_ptr->wraith_form)
		pass_wall = TRUE;
	else
		pass_wall = FALSE;

        /* Wall mimicry force the player to stay in walls */
        if ((p_ptr->pclass == CLASS_MIMIC) && (p_ptr->class_extra6 & CLASS_WALL))
        {
                only_wall = TRUE;
        }

	switch (feature)
	{
		case FEAT_DEEP_WATER:
                        if (p_ptr->wild_mode)
			{
				int wt = (adj_str_wgt[p_ptr->stat_ind[A_STR]] * 100) / 2;

                                if ((calc_total_weight() < wt) || (p_ptr->ffall))
					return (TRUE);
				else
					return (FALSE);
			}
                        else
                                return (TRUE);

		case FEAT_SHAL_LAVA:
                        if (p_ptr->wild_mode)
			{
                                if ((p_ptr->resist_fire) ||
					(p_ptr->immune_fire) ||
					(p_ptr->oppose_fire) ||
                                        (p_ptr->ffall))
					return (TRUE);
				else if (p_ptr->pclass == CLASS_DAEMONOLOGIST)
					return (TRUE);
				else
					return (FALSE);
			}
                        else
                                return (TRUE);

		case FEAT_DEEP_LAVA:
                        if (p_ptr->wild_mode)
			{
                                if ((p_ptr->resist_fire) ||
					(p_ptr->immune_fire) ||
					(p_ptr->oppose_fire) ||
                                        (p_ptr->ffall))
					return (TRUE);
				else if (p_ptr->pclass == CLASS_DAEMONOLOGIST)
					return (TRUE);
				else
					return (FALSE);
			}
                        else
                                return (TRUE);

		case FEAT_TREES:
			{
                                if ((p_ptr->fly) || (p_ptr->prace == RACE_ENT) ||
                                    (p_ptr->mimic_form == MIMIC_ENT) ||
                                    (p_ptr->pclass == CLASS_RANGER) ||
                                    (p_ptr->prace == RACE_WOOD_ELF) ||
                                    (p_ptr->pclass == CLASS_DRUID))
					return (TRUE);
				else
					return (FALSE);
			}

                default:
			{
                                if ((p_ptr->climb) && (f_info[feature].flags1 & FF1_CAN_CLIMB))
					return (TRUE);
                                if ((p_ptr->fly) && ((f_info[feature].flags1 & FF1_CAN_FLY) || (f_info[feature].flags1 & FF1_CAN_LEVITATE)))
					return (TRUE);
                                else if (only_wall && (f_info[feature].flags1 & FF1_FLOOR))
                                        return (FALSE);
                                else if ((p_ptr->ffall) && (f_info[feature].flags1 & FF1_CAN_LEVITATE))
					return (TRUE);
                                else if ((pass_wall || only_wall) && (f_info[feature].flags1 & FF1_CAN_PASS))
					return (TRUE);
                                else if (f_info[feature].flags1 & FF1_NO_WALK)
                                        return (FALSE);
                                else if ((f_info[feature].flags1 & FF1_WEB) && (!(r_info[p_ptr->body_monster].flags7 & RF7_SPIDER)))
                                        return (FALSE);
			}
	}

	return (TRUE);
}

/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should (probably) always induce energy expenditure.
 *
 * Note that moving will *always* take a turn, and will *always* hit
 * any monster which might be in the destination grid.  Previously,
 * moving into walls was "free" and did NOT hit invisible monsters.
 */
void move_player_aux(int dir, int do_pickup, int run)
{
        int y, x, tmp;

	cave_type *c_ptr = &cave[py][px];
	monster_type *m_ptr;
        monster_race *r_ptr = &r_info[p_ptr->body_monster], *mr_ptr;

	char m_name[80];

	bool stormbringer = FALSE;

	bool oktomove = TRUE;

        /* Hack - random movement */
        if (p_ptr->disembodied)
                tmp = dir;
        else if ((r_ptr->flags1 & RF1_RAND_25) && (r_ptr->flags1 & RF1_RAND_50))
        {
                if (randint(100) < 75)
                        tmp = randint(9);
                else
                        tmp = dir;
        }
        else if (r_ptr->flags1 & RF1_RAND_50)
        {
                if (randint(100) < 50)
                        tmp = randint(9);
                else
                        tmp = dir;
        }
        else if (r_ptr->flags1 & RF1_RAND_25)
        {
                if (randint(100) < 25)
                        tmp = randint(9);
                else
                        tmp = dir;
        }
        else
        {
                tmp = dir;
        }

        if ((c_ptr->feat == FEAT_ICE) && (!p_ptr->ffall && !p_ptr->fly))
	{
                if (magik(70 - p_ptr->lev))
                {
			tmp = randint(9);
                        msg_print("You slip on the icy floor.");
                }
		else
			tmp = dir;
	}

        /* Find the result of moving */
        y = py + ddy[tmp];
        x = px + ddx[tmp];

	/* Examine the destination */
	c_ptr = &cave[y][x];

        /* Change oldpx and oldpy to place the player well when going back to big mode */
        if (p_ptr->wild_mode)
        {
                if(ddy[tmp] > 0)  p_ptr->oldpy = 1;
                if(ddy[tmp] < 0)  p_ptr->oldpy = MAX_HGT - 2;
                if(ddy[tmp] == 0) p_ptr->oldpy = MAX_HGT / 2;
                if(ddx[tmp] > 0)  p_ptr->oldpx = 1;
                if(ddx[tmp] < 0)  p_ptr->oldpx = MAX_WID - 2;
                if(ddx[tmp] == 0) p_ptr->oldpx = MAX_WID / 2;
        }

	/* Exit the area */
        if ((!dun_level) && (!p_ptr->wild_mode) && (!is_quest(dun_level)) &&
		((x == 0) || (x == cur_wid-1) ||
		 (y == 0) || (y == cur_hgt-1)))
	{
		/* Can the player enter the grid? */
		if (player_can_enter(c_ptr->mimic))
		{
			/* Hack: move to new area */
			if ((y == 0) && (x == 0))
			{
				p_ptr->wilderness_y--;
				p_ptr->wilderness_x--;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = cur_wid - 2;
                                ambush_flag = FALSE;
			}

			else if ((y == 0) && (x == MAX_WID-1))
			{
				p_ptr->wilderness_y--;
				p_ptr->wilderness_x++;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = 1;
                                ambush_flag = FALSE;
			}

			else if ((y == MAX_HGT-1) && (x == 0))
			{
				p_ptr->wilderness_y++;
				p_ptr->wilderness_x--;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = cur_wid - 2;
                                ambush_flag = FALSE;
			}

			else if ((y == MAX_HGT-1) && (x == MAX_WID-1))
			{
				p_ptr->wilderness_y++;
				p_ptr->wilderness_x++;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = 1;
                                ambush_flag = FALSE;
			}

			else if (y == 0)
			{
				p_ptr->wilderness_y--;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = x;
                                ambush_flag = FALSE;
			}

			else if (y == cur_hgt-1)
			{
				p_ptr->wilderness_y++;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = x;
                                ambush_flag = FALSE;
			}

			else if (x == 0)
			{
				p_ptr->wilderness_x--;
				p_ptr->oldpx = cur_wid - 2;
				p_ptr->oldpy = y;
                                ambush_flag = FALSE;
			}

			else if (x == cur_wid-1)
			{
				p_ptr->wilderness_x++;
				p_ptr->oldpx = 1;
				p_ptr->oldpy = y;
                                ambush_flag = FALSE;
			}

			p_ptr->leaving = TRUE;

                        return;
		}
	}

        /* Some hooks */
        if (process_hooks(HOOK_MOVE, (y << 8) + x)) return;

	/* Get the monster */
	m_ptr = &m_list[c_ptr->m_idx];
        mr_ptr = race_inf(m_ptr);

	if (inventory[INVEN_WIELD].art_name)
	{
		if (streq(quark_str(inventory[INVEN_WIELD].art_name), "'Stormbringer'"))
			stormbringer = TRUE;
	}

	/* Hack -- attack monsters */
        if (c_ptr->m_idx && (m_ptr->ml || player_can_enter(c_ptr->feat)))
	{

		/* Attack -- only if we can see it OR it is not in a wall */
                if ((is_friend(m_ptr) > 0) &&
		    !(p_ptr->confused || p_ptr->image || !(m_ptr->ml) || p_ptr->stun ||
		    ((p_ptr->muta2 & MUT2_BERS_RAGE) && p_ptr->shero)) &&
		    (pattern_seq(py, px, y, x)) &&
                    ((player_can_enter(cave[y][x].feat))))
		{
			m_ptr->csleep = 0;

			/* Extract monster name (or "it") */
			monster_desc(m_name, m_ptr, 0);

			/* Auto-Recall if possible and visible */
                        if (m_ptr->ml) monster_race_track(m_ptr->r_idx, m_ptr->ego);

			/* Track a new monster */
			if (m_ptr->ml) health_track(c_ptr->m_idx);

			/* displace? */
			if (stormbringer && (randint(1000)>666))
			{
                                py_attack(y,x,-1);
			}
			else if (cave_floor_bold(py, px) ||
                            (mr_ptr->flags2 & RF2_PASS_WALL))
			{
				msg_format("You push past %s.", m_name);
				m_ptr->fy = py;
				m_ptr->fx = px;
				cave[py][px].m_idx = c_ptr->m_idx;
				c_ptr->m_idx = 0;
				update_mon(cave[py][px].m_idx, TRUE);
			}
			else
			{
				msg_format("%^s is in your way!", m_name);
				energy_use = 0;
				oktomove = FALSE;
			}

			/* now continue on to 'movement' */
		}
		else
		{
                        py_attack(y, x,-1);
			oktomove = FALSE;
		}
	}

	else if ((c_ptr->feat == FEAT_DARK_PIT) && !p_ptr->ffall)
	{
		msg_print("You can't cross the chasm.");
		running = 0;
		oktomove = FALSE;
	}

#ifdef ALLOW_EASY_DISARM /* TNB */

	/* Disarm a visible trap */
	else if ((do_pickup != easy_disarm) &&
		(c_ptr->t_idx != 0) && (c_ptr->info & CAVE_TRDT))
	{
                (void) do_cmd_disarm_aux(y, x, tmp);
		return;
	}

#endif /* ALLOW_EASY_DISARM -- TNB */

        /* Player can't enter ? soo bad for him/her ... */
        else if (!player_can_enter(c_ptr->feat))
	{
		oktomove = FALSE;

		/* Disturb the player */
		disturb(0, 0);

                if (p_ptr->prob_travel)
                {
                        if (passwall(tmp, TRUE)) return;
                }

		/* Notice things in the dark */
		if ((!(c_ptr->info & (CAVE_MARK))) &&
		    (p_ptr->blind || !(c_ptr->info & (CAVE_LITE))))
		{
			/* Rubble */
			if (c_ptr->feat == FEAT_RUBBLE)
			{
				msg_print("You feel some rubble blocking your way.");
				c_ptr->info |= (CAVE_MARK);
				lite_spot(y, x);
			}

			/* Closed door */
			else if (c_ptr->feat < FEAT_SECRET)
			{
				msg_print("You feel a closed door blocking your way.");
				c_ptr->info |= (CAVE_MARK);
				lite_spot(y, x);
			}

			/* Wall (or secret door) */
			else
			{
                                int feat;

                                if (c_ptr->mimic) feat = c_ptr->mimic;
                                else feat = f_info[c_ptr->feat].mimic;

                                msg_format("You feel %s.", f_text + f_info[feat].text);
                                c_ptr->info |= (CAVE_MARK);
				lite_spot(y, x);
			}
		}

		/* Notice things */
		else
		{
			/* Rubble */
			if (c_ptr->feat == FEAT_RUBBLE)
			{
                                if (!easy_tunnel)
                                {
                                        msg_print("There is rubble blocking your way.");

                                        if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
                                                energy_use = 0;
                                        /*
                                         * Well, it makes sense that you lose time bumping into
                                         * a wall _if_ you are confused, stunned or blind; but
                                         * typing mistakes should not cost you a turn...
                                         */
                                }
                                else
                                {
                                        do_cmd_tunnel_aux(y, x, dir);
                                        return;
                                }
			}
			/* Closed doors */
			else if (c_ptr->feat < FEAT_SECRET)
			{
#ifdef ALLOW_EASY_OPEN

				if (easy_open)
				{
					if (easy_open_door(y, x)) return;
				}
				else

#endif /* ALLOW_EASY_OPEN */

				{
					msg_print("There is a closed door blocking your way.");

					if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
						energy_use = 0;
				}
			}

			/* Wall (or secret door) */
                        else
			{
                                if (!easy_tunnel)
                                {
                                        int feat;

                                        if (c_ptr->mimic) feat = c_ptr->mimic;
                                        else feat = f_info[c_ptr->feat].mimic;

                                        msg_format("There is %s.", f_text + f_info[feat].text);

                                        if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
                                                energy_use = 0;
                                }
                                else
                                {
                                        do_cmd_tunnel_aux(y, x, dir);
                                        return;
                                }
			}
		}

		/* Sound */
		sound(SOUND_HITWALL);
	}

	/* Normal movement */
	if (!pattern_seq(py, px, y, x))
	{
		if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
		{
			energy_use = 0;
		}

		disturb(0,0); /* To avoid a loop with running */

		oktomove = FALSE;
	}

	/* Normal movement */
	if (oktomove)
	{
                int oy, ox;
                int feat;

		/* Save old location */
		oy = py;
		ox = px;

		/* Move the player */
		py = y;
		px = x;

                if (cave[py][px].mimic) feat = cave[py][px].mimic;
                else feat = cave[py][px].feat;

		/* Redraw new spot */
		lite_spot(py, px);

		/* Redraw old spot */
		lite_spot(oy, ox);

		/* Sound */
		/* sound(SOUND_WALK); */

		/* Check for new panel (redraw map) */
		verify_panel();

                /* For get everything when requested hehe I'm *NASTY* */
                if (dun_level && (d_info[dungeon_type].flags1 & DF1_FORGET))
                {
                        wiz_dark();
                }

		/* Update stuff */
                p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

		/* Update the monsters */
		p_ptr->update |= (PU_DISTANCE);

		/* Window stuff */
                if (!run) p_ptr->window |= (PW_OVERHEAD);

                /* Mega-hack some feature descs */
                if (f_info[cave[py][px].feat].text > 1)
                {
                        msg_print(f_text + f_info[feat].text);
                }

		/* Spontaneous Searching */
		if ((p_ptr->skill_fos >= 50) ||
		    (0 == rand_int(50 - p_ptr->skill_fos)))
		{
			search();
		}

		/* Continuous Searching */
		if (p_ptr->searching)
		{
			search();
		}

		/* Handle "objects" */
#ifdef ALLOW_EASY_DISARM /* TNB */

		carry(do_pickup != always_pickup);

#else /* ALLOW_EASY_DISARM -- TNB */

		carry(do_pickup);

#endif /* ALLOW_EASY_DISARM -- TNB */

		/* Handle "store doors" */
                if (c_ptr->feat == FEAT_SHOP)
		{
			/* Disturb */
			disturb(0, 0);

			/* Hack -- Enter store */
			command_new = '_';
		}

		/* Handle quest areas -KMW- */
		else if (cave[y][x].feat == FEAT_QUEST_ENTER)
		{
			/* Disturb */
			disturb(0, 0);

			/* Hack -- Enter quest level */
			command_new = '[';
		}

		else if (cave[y][x].feat == FEAT_QUEST_EXIT)
		{
			leaving_quest = p_ptr->inside_quest;

			p_ptr->inside_quest = cave[y][x].special;
			dun_level = 0;
			p_ptr->oldpx = 0;
			p_ptr->oldpy = 0;
			p_ptr->leaving = TRUE;
		}

                else if (cave[y][x].feat >= FEAT_ALTAR_HEAD &&
                         cave[y][x].feat <= FEAT_ALTAR_TAIL)
                         {
                            cptr name = f_name + f_info[cave[y][x].feat].name;
                            cptr pref = (is_a_vowel(name[0])) ? "an" : "a";
                            msg_format("You see %s %s.", pref, name);
                         }

		/* Discover invisible traps */
		else if ((c_ptr->t_idx != 0) &&
			 !(f_info[cave[y][x].feat].flags1 & FF1_DOOR))
		{
			/* Disturb */
			disturb(0, 0);

			if (!(c_ptr->info & CAVE_TRDT))
			{
				/* Message */
				msg_print("You found a trap!");

				/* Pick a trap */
				pick_trap(py, px);
			}

			/* Hit the trap */
			hit_trap();
		}

                /* Execute the inscription */
                else if (c_ptr->inscription)
                {
                        msg_format("There is an inscription here: %s", inscription_info[c_ptr->inscription].text);
                        if(inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_WALK)
                        {
                                execute_inscription(c_ptr->inscription, py, px);
                        }
                }
	}

        /* Update wilderness knowledge */
        if (p_ptr->wild_mode)
        {
                if (wizard) msg_format("y:%d, x:%d", py, px);

                /* Update the known wilderness */
                reveal_wilderness_around_player(py, px, 0, WILDERNESS_SEE_RADIUS);
        }
}

void move_player(int dir, int do_pickup)
{
        move_player_aux(dir, do_pickup, 0);
}


/*
 * Hack -- Check for a "known wall" (see below)
 */
static int see_wall(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are not known walls */
	if (!in_bounds2(y, x)) return (FALSE);

	/* Non-wall grids are not known walls */
	if (cave[y][x].feat < FEAT_SECRET) return (FALSE);

        if ((cave[y][x].feat == FEAT_DEEP_WATER) ||
           ((cave[y][x].feat >= FEAT_SHAL_WATER) &&
            (cave[y][x].feat <= FEAT_GRASS))) return (FALSE);

        if (cave[y][x].feat == FEAT_SHOP) return (FALSE);

        if ((f_info[cave[y][x].feat].flags1 & FF1_CAN_RUN)) return (FALSE);

	/* Must be known to the player */
	if (!(cave[y][x].info & (CAVE_MARK))) return (FALSE);

	/* Default */
	return (TRUE);
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are unknown */
	if (!in_bounds2(y, x)) return (TRUE);

	/* Memorized grids are always known */
	if (cave[y][x].info & (CAVE_MARK)) return (FALSE);

	/* Non-floor grids are unknown */
	if (!cave_floor_bold(y, x)) return (TRUE);

	/* Viewable door/wall grids are known */
	if (player_can_see_bold(y, x)) return (FALSE);

	/* Default */
	return (TRUE);
}





/*
 * The running algorithm:                       -CJS-
 *
 * In the diagrams below, the player has just arrived in the
 * grid marked as '@', and he has just come from a grid marked
 * as 'o', and he is about to enter the grid marked as 'x'.
 *
 * Of course, if the "requested" move was impossible, then you
 * will of course be blocked, and will stop.
 *
 * Overview: You keep moving until something interesting happens.
 * If you are in an enclosed space, you follow corners. This is
 * the usual corridor scheme. If you are in an open space, you go
 * straight, but stop before entering enclosed space. This is
 * analogous to reaching doorways. If you have enclosed space on
 * one side only (that is, running along side a wall) stop if
 * your wall opens out, or your open space closes in. Either case
 * corresponds to a doorway.
 *
 * What happens depends on what you can really SEE. (i.e. if you
 * have no light, then running along a dark corridor is JUST like
 * running in a dark room.) The algorithm works equally well in
 * corridors, rooms, mine tailings, earthquake rubble, etc, etc.
 *
 * These conditions are kept in static memory:
 * find_openarea         You are in the open on at least one
 * side.
 * find_breakleft        You have a wall on the left, and will
 * stop if it opens
 * find_breakright       You have a wall on the right, and will
 * stop if it opens
 *
 * To initialize these conditions, we examine the grids adjacent
 * to the grid marked 'x', two on each side (marked 'L' and 'R').
 * If either one of the two grids on a given side is seen to be
 * closed, then that side is considered to be closed. If both
 * sides are closed, then it is an enclosed (corridor) run.
 *
 * LL           L
 * @x          LxR
 * RR          @R
 *
 * Looking at more than just the immediate squares is
 * significant. Consider the following case. A run along the
 * corridor will stop just before entering the center point,
 * because a choice is clearly established. Running in any of
 * three available directions will be defined as a corridor run.
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * #.#
 * ##.##
 * .@x..
 * ##.##
 * #.#
 *
 * Likewise, a run along a wall, and then into a doorway (two
 * runs) will work correctly. A single run rightwards from @ will
 * stop at 1. Another run right and down will enter the corridor
 * and make the corner, stopping at the 2.
 *
 * #@x    1
 * ########### ######
 * 2        #
 * #############
 * #
 *
 * After any move, the function area_affect is called to
 * determine the new surroundings, and the direction of
 * subsequent moves. It examines the current player location
 * (at which the runner has just arrived) and the previous
 * direction (from which the runner is considered to have come).
 *
 * Moving one square in some direction places you adjacent to
 * three or five new squares (for straight and diagonal moves
 * respectively) to which you were not previously adjacent,
 * marked as '!' in the diagrams below.
 *
 * ...!   ...
 * .o@!   .o.!
 * ...!   ..@!
 * !!!
 *
 * You STOP if any of the new squares are interesting in any way:
 * for example, if they contain visible monsters or treasure.
 *
 * You STOP if any of the newly adjacent squares seem to be open,
 * and you are also looking for a break on that side. (that is,
 * find_openarea AND find_break).
 *
 * You STOP if any of the newly adjacent squares do NOT seem to be
 * open and you are in an open area, and that side was previously
 * entirely open.
 *
 * Corners: If you are not in the open (i.e. you are in a corridor)
 * and there is only one way to go in the new squares, then turn in
 * that direction. If there are more than two new ways to go, STOP.
 * If there are two ways to go, and those ways are separated by a
 * square which does not seem to be open, then STOP.
 *
 * Otherwise, we have a potential corner. There are two new open
 * squares, which are also adjacent. One of the new squares is
 * diagonally located, the other is straight on (as in the diagram).
 * We consider two more squares further out (marked below as ?).
 *
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid, and "check_dir" to the grid marked 's'.
 *
 * .s
 * @x?
 * #?
 *
 * If they are both seen to be closed, then it is seen that no
 * benefit is gained from moving straight. It is a known corner.
 * To cut the corner, go diagonally, otherwise go straight, but
 * pretend you stepped diagonally into that next location for a
 * full view next time. Conversely, if one of the ? squares is
 * not seen to be closed, then there is a potential choice. We check
 * to see whether it is a potential corner or an intersection/room entrance.
 * If the square two spaces straight ahead, and the space marked with 's'
 * are both blank, then it is a potential corner and enter if find_examine
 * is set, otherwise must stop because it is not a corner.
 */




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static byte cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static byte chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };

/*
 * The direction we are running
 */
static byte find_current;

/*
 * The direction we came from
 */
static byte find_prevdir;

/*
 * We are looking for open area
 */
static bool find_openarea;

/*
 * We are looking for a break
 */
static bool find_breakright;
static bool find_breakleft;



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. ???
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                  p
 */
static void run_init(int dir)
{
	int             row, col, deepleft, deepright;
	int             i, shortleft, shortright;


	/* Save the direction */
	find_current = dir;

	/* Assume running straight */
	find_prevdir = dir;

	/* Assume looking for open area */
	find_openarea = TRUE;

	/* Assume not looking for breaks */
	find_breakright = find_breakleft = FALSE;

	/* Assume no nearby walls */
	deepleft = deepright = FALSE;
	shortright = shortleft = FALSE;

	/* Find the destination grid */
	row = py + ddy[dir];
	col = px + ddx[dir];

	/* Extract cycle index */
	i = chome[dir];

	/* Check for walls */
	if (see_wall(cycle[i+1], py, px))
	{
		find_breakleft = TRUE;
		shortleft = TRUE;
	}
	else if (see_wall(cycle[i+1], row, col))
	{
		find_breakleft = TRUE;
		deepleft = TRUE;
	}

	/* Check for walls */
	if (see_wall(cycle[i-1], py, px))
	{
		find_breakright = TRUE;
		shortright = TRUE;
	}
	else if (see_wall(cycle[i-1], row, col))
	{
		find_breakright = TRUE;
		deepright = TRUE;
	}

	/* Looking for a break */
	if (find_breakleft && find_breakright)
	{
		/* Not looking for open area */
		find_openarea = FALSE;

		/* Hack -- allow angled corridor entry */
		if (dir & 0x01)
		{
			if (deepleft && !deepright)
			{
				find_prevdir = cycle[i - 1];
			}
			else if (deepright && !deepleft)
			{
				find_prevdir = cycle[i + 1];
			}
		}

		/* Hack -- allow blunt corridor entry */
		else if (see_wall(cycle[i], row, col))
		{
			if (shortleft && !shortright)
			{
				find_prevdir = cycle[i - 2];
			}
			else if (shortright && !shortleft)
			{
				find_prevdir = cycle[i + 2];
			}
		}
	}
}


/*
 * Update the current "run" path
 *
 * Return TRUE if the running should be stopped
 */
static bool run_test(void)
{
	int         prev_dir, new_dir, check_dir = 0;
	int         row, col;
	int         i, max, inv;
	int         option = 0, option2 = 0;
	cave_type   *c_ptr;

	/* Where we came from */
	prev_dir = find_prevdir;


	/* Range of newly adjacent grids */
	max = (prev_dir & 0x01) + 1;


	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++)
	{
		s16b this_o_idx, next_o_idx = 0;


		/* New direction */
		new_dir = cycle[chome[prev_dir] + i];

		/* New location */
		row = py + ddy[new_dir];
		col = px + ddx[new_dir];

		/* Access grid */
		c_ptr = &cave[row][col];


		/* Visible monsters abort running */
		if (c_ptr->m_idx)
		{
			monster_type *m_ptr = &m_list[c_ptr->m_idx];

			/* Visible monster */
			if (m_ptr->ml) return (TRUE);
		}

		/* Visible objects abort running */
		for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;

			/* Acquire object */
			o_ptr = &o_list[this_o_idx];

			/* Acquire next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Visible object */
			if (o_ptr->marked) return (TRUE);
		}


		/* Assume unknown */
		inv = TRUE;

		/* Check memorized grids */
		if (c_ptr->info & (CAVE_MARK))
		{
			bool notice = TRUE;

			/* Examine the terrain */
			switch (c_ptr->feat)
			{
				/* Floors */
				case FEAT_FLOOR:

				/* Secret doors */
				case FEAT_SECRET:

				/* Normal veins */
				case FEAT_MAGMA:
				case FEAT_QUARTZ:
				case FEAT_SANDWALL:

				/* Hidden treasure */
				case FEAT_MAGMA_H:
				case FEAT_QUARTZ_H:
				case FEAT_SANDWALL_H:

				/* Walls */
				case FEAT_WALL_EXTRA:
				case FEAT_WALL_INNER:
				case FEAT_WALL_OUTER:
				case FEAT_WALL_SOLID:
				case FEAT_PERM_EXTRA:
				case FEAT_PERM_INNER:
				case FEAT_PERM_OUTER:
				case FEAT_PERM_SOLID:
				/* dirt, grass, trees, ... */
				case FEAT_SHAL_WATER:
				case FEAT_DIRT:
				case FEAT_GRASS:
				case FEAT_DARK_PIT:
				case FEAT_TREES:
				case FEAT_MOUNTAIN:
				case FEAT_SAND:
				case FEAT_DEAD_TREE:
				case FEAT_ASH:
				case FEAT_MUD:
				{
					/* Ignore */
					notice = FALSE;

					/* Done */
					break;
				}

				/* quest features */
				case FEAT_QUEST_ENTER:
				case FEAT_QUEST_EXIT:
				{
					/* Notice */
					notice = TRUE;

					/* Done */
					break;
				}

				case FEAT_DEEP_LAVA:
				case FEAT_SHAL_LAVA:
				{
					/* Ignore */
					if (p_ptr->invuln || p_ptr->immune_fire) notice = FALSE;
					if (p_ptr->pclass == CLASS_DAEMONOLOGIST) notice = FALSE;

					/* Done */
					break;
				}

				case FEAT_DEEP_WATER:
				case FEAT_ICE:
				{
					/* Ignore */
					if (p_ptr->ffall) notice = FALSE;

					/* Done */
					break;
				}

				/* Open doors */
				case FEAT_OPEN:
				case FEAT_BROKEN:
				{
					/* Option -- ignore */
					if (find_ignore_doors) notice = FALSE;

					/* Done */
					break;
				}

				/* Stairs */
				case FEAT_LESS:
				case FEAT_MORE:
				{
					/* Option -- ignore */
					if (find_ignore_stairs) notice = FALSE;

					/* Done */
					break;
				}
			}

                        if(f_info[c_ptr->feat].flags1 & FF1_DONT_NOTICE_RUNNING) notice = FALSE;

			/* Interesting feature */
			if (notice) return (TRUE);

			/* The grid is "visible" */
			inv = FALSE;
		}

		/* Analyze unknown grids and floors */
		if (inv || cave_floor_bold(row, col))
		{
			/* Looking for open area */
			if (find_openarea)
			{
				/* Nothing */
			}

			/* The first new direction. */
			else if (!option)
			{
				option = new_dir;
			}

			/* Three new directions. Stop running. */
			else if (option2)
			{
				return (TRUE);
			}

			/* Two non-adjacent new directions.  Stop running. */
			else if (option != cycle[chome[prev_dir] + i - 1])
			{
				return (TRUE);
			}

			/* Two new (adjacent) directions (case 1) */
			else if (new_dir & 0x01)
			{
				check_dir = cycle[chome[prev_dir] + i - 2];
				option2 = new_dir;
			}

			/* Two new (adjacent) directions (case 2) */
			else
			{
				check_dir = cycle[chome[prev_dir] + i + 1];
				option2 = option;
				option = new_dir;
			}
		}

		/* Obstacle, while looking for open area */
		else
		{
			if (find_openarea)
			{
				if (i < 0)
				{
					/* Break to the right */
					find_breakright = TRUE;
				}

				else if (i > 0)
				{
					/* Break to the left */
					find_breakleft = TRUE;
				}
			}
		}
	}


	/* Looking for open area */
	if (find_openarea)
	{
		/* Hack -- look again */
		for (i = -max; i < 0; i++)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Access grid */
			c_ptr = &cave[row][col];

			/* Unknown grid or non-wall XXX XXX XXX cave_floor_grid(c_ptr)) */
			if (!(c_ptr->info & (CAVE_MARK)) ||
			    ((c_ptr->feat < FEAT_SECRET) ||
                            (c_ptr->feat == FEAT_DEEP_WATER) ||
                            ((c_ptr->feat >= FEAT_SHAL_WATER) &&
				 (c_ptr->feat <= FEAT_GRASS))))

			{
				/* Looking to break right */
				if (find_breakright)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break left */
				if (find_breakleft)
				{
					return (TRUE);
				}
			}
		}

		/* Hack -- look again */
		for (i = max; i > 0; i--)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Access grid */
			c_ptr = &cave[row][col];

			/* Unknown grid or non-wall XXX XXX XXX cave_floor_grid(c_ptr)) */
			if (!(c_ptr->info & (CAVE_MARK)) ||
			    ((c_ptr->feat < FEAT_SECRET) ||
                            (c_ptr->feat == FEAT_DEEP_WATER) ||
                            ((c_ptr->feat >= FEAT_SHAL_WATER) &&
				 (c_ptr->feat <= FEAT_GRASS))))

			{
				/* Looking to break left */
				if (find_breakleft)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break right */
				if (find_breakright)
				{
					return (TRUE);
				}
			}
		}
	}


	/* Not looking for open area */
	else
	{
		/* No options */
		if (!option)
		{
			return (TRUE);
		}

		/* One option */
		else if (!option2)
		{
			/* Primary option */
			find_current = option;

			/* No other options */
			find_prevdir = option;
		}

		/* Two options, examining corners */
		else if (find_examine && !find_cut)
		{
			/* Primary option */
			find_current = option;

			/* Hack -- allow curving */
			find_prevdir = option2;
		}

		/* Two options, pick one */
		else
		{
			/* Get next location */
			row = py + ddy[option];
			col = px + ddx[option];

			/* Don't see that it is closed off. */
			/* This could be a potential corner or an intersection. */
			if (!see_wall(option, row, col) ||
			    !see_wall(check_dir, row, col))
			{
				/* Can not see anything ahead and in the direction we */
				/* are turning, assume that it is a potential corner. */
				if (find_examine &&
				    see_nothing(option, row, col) &&
				    see_nothing(option2, row, col))
				{
					find_current = option;
					find_prevdir = option2;
				}

				/* STOP: we are next to an intersection or a room */
				else
				{
					return (TRUE);
				}
			}

			/* This corner is seen to be enclosed; we cut the corner. */
			else if (find_cut)
			{
				find_current = option2;
				find_prevdir = option2;
			}

			/* This corner is seen to be enclosed, and we */
			/* deliberately go the long way. */
			else
			{
				find_current = option;
				find_prevdir = option2;
			}
		}
	}


	/* About to hit a known wall, stop */
	if (see_wall(find_current, py, px))
	{
		return (TRUE);
	}


	/* Failure */
	return (FALSE);
}



/*
 * Take one step along the current "run" path
 */
void run_step(int dir)
{
	/* Start running */
	if (dir)
	{
		/* Hack -- do not start silly run */
		if (see_wall(dir, py, px) &&
		   (cave[py+ddy[dir]][px+ddx[dir]].feat != FEAT_TREES))
		{
			/* Message */
			msg_print("You cannot run in that direction.");

			/* Disturb */
			disturb(0, 0);

			/* Done */
			return;
		}

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);

		/* Initialize */
		run_init(dir);
	}

	/* Keep running */
	else
	{
		/* Update run */
		if (run_test())
		{
			/* Disturb */
			disturb(0, 0);

			/* Done */
			return;
		}
	}

	/* Decrease the run counter */
	if (--running <= 0) return;

	/* Take time */
	energy_use = 100;


	/* Move the player, using the "pickup" flag */
#ifdef ALLOW_EASY_DISARM /* TNB */

        move_player_aux(find_current, FALSE, 1);

#else /* ALLOW_EASY_DISARM -- TNB */

        move_player_aux(find_current, always_pickup, 1);

#endif /* ALLOW_EASY_DISARM -- TNB */
}

/*
 * Take care of the various things that can happen when you step
 * into a space. (Objects, traps, and stores.)
 */
void step_effects(int y, int x, int do_pickup)
{
	/* Handle "objects" */
        py_pickup_floor(do_pickup);

	/* Handle "store doors" */
        if (cave[y][x].feat == FEAT_SHOP)
	{
		/* Disturb */
		disturb(0, 0);

		/* Hack -- Enter store */
                command_new = KTRL('V');
	}

	/* Discover/set off traps */
        else if (cave[y][x].t_idx != 0)
	{
		/* Disturb */
		disturb(0, 0);

		if (!(cave[y][x].info & CAVE_TRDT))
		{
			/* Message */
			msg_print("You found a trap!");

			/* Pick a trap */
			pick_trap(y, x);
		}

		/* Hit the trap */
                hit_trap();
	}
}

/*
 * Issue a pet command
 */
void do_cmd_pet(void)
{
	int             i = 0;
	int             num = 0;
	int             powers[36];
	char            power_desc[36][80];
	bool            flag, redraw;
	int             ask;
	char            choice;
	char            out_val[160];
	int             pets = 0, pet_ctr = 0;
	bool            all_pets = FALSE;
	monster_type    *m_ptr;


	for (num = 0; num < 36; num++)
	{
		powers[num] = 0;
		strcpy(power_desc[num], "");
	}

	num = 0;

	if (p_ptr->confused)
	{
		msg_print("You are too confused to command your pets");
		energy_use = 0;
		return;
	}

	/* Calculate pets */
	/* Process the monsters (backwards) */
	for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
	{
		/* Access the monster */
		m_ptr = &m_list[pet_ctr];

                if (m_ptr->status >= MSTATUS_FRIEND) pets++;
	}

	if (pets == 0)
	{
                msg_print("You have no pets/companions.");
		energy_use = 0;
		return;
	}
	else
	{
		strcpy(power_desc[num], "dismiss pets");
		powers[num++] = 1;
		strcpy(power_desc[num], "call pets");
		powers[num++] = 2;
		strcpy(power_desc[num], "follow me");
		powers[num++] = 6;
		strcpy(power_desc[num], "seek and destroy");
		powers[num++] = 3;
		if (p_ptr->pet_open_doors)
			strcpy(power_desc[num], "disallow open doors");
		else
			strcpy(power_desc[num], "allow open doors");
		powers[num++] = 4;
		if (p_ptr->pet_pickup_items)
			strcpy(power_desc[num], "disallow pickup items");
		else
			strcpy(power_desc[num], "allow pickup items");
		powers[num++] = 5;
                strcpy(power_desc[num], "give target to a friend");
                powers[num++] = 7;
                strcpy(power_desc[num], "give target to all friends");
                powers[num++] = 8;
                strcpy(power_desc[num], "friend forget target");
                powers[num++] = 9;
	}

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Build a prompt (accept all spells) */
	if (num <= 26)
	{
		/* Build a prompt (accept all spells) */
		strnfmt(out_val, 78, "(Command %c-%c, *=List, ESC=exit) Select a command: ",
			I2A(0), I2A(num - 1));
	}
	else
	{
		strnfmt(out_val, 78, "(Command %c-%c, *=List, ESC=exit) Select a command: ",
			I2A(0), '0' + num - 27);
	}

	/* Get a command from the user */
	while (!flag && get_com(out_val, &choice))
	{
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?'))
		{
			/* Show the list */
			if (!redraw)
			{
				byte y = 1, x = 0;
				int ctr = 0;
				char dummy[80];

				strcpy(dummy, "");

				/* Show list */
				redraw = TRUE;

				/* Save the screen */
                                character_icky = TRUE;
				Term_save();

				prt("", y++, x);

				while (ctr < num)
				{
					sprintf(dummy, "%c) %s", I2A(ctr), power_desc[ctr]);
					prt(dummy, y + ctr, x);
					ctr++;
				}

				if (ctr < 17)
				{
					prt("", y + ctr, x);
				}
				else
				{
					prt("", y + 17, x);
				}
			}

			/* Hide the list */
			else
			{
				/* Hide list */
				redraw = FALSE;

				/* Restore the screen */
				Term_load();
                                character_icky = FALSE;
			}

			/* Redo asking */
			continue;
		}

		if (choice == '\r' && num == 1)
		{
			choice = 'a';
		}

		if (isalpha(choice))
		{
			/* Note verify */
			ask = (isupper(choice));

			/* Lowercase */
			if (ask) choice = tolower(choice);

			/* Extract request */
			i = (islower(choice) ? A2I(choice) : -1);
		}
		else
		{
			ask = FALSE; /* Can't uppercase digits */

			i = choice - '0' + 26;
		}

		/* Totally Illegal */
		if ((i < 0) || (i >= num))
		{
			bell();
			continue;
		}

		/* Verify it */
		if (ask)
		{
			char tmp_val[160];

			/* Prompt */
			strnfmt(tmp_val, 78, "Use %s? ", power_desc[i]);

			/* Belay that order */
			if (!get_check(tmp_val)) continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	/* Restore the screen */
        if (redraw)
        {
                Term_load();
                character_icky = FALSE;
        }

	/* Abort if needed */
	if (!flag)
	{
		energy_use = 0;
		return;
	}

	switch (powers[i])
	{
                /* forget target */
                case 9:
		{
                        monster_type *m_ptr;
                        int ii, jj;

                        msg_print("Select the friendly monster:");
                        if (!tgt_pt(&ii,&jj)) return;

                        if (cave[jj][ii].m_idx)
                        {
                                m_ptr = &m_list[cave[jj][ii].m_idx];

                                if (m_ptr->status < MSTATUS_PET)
                                {
                                        msg_print("You cannot give orders to this monster.");
                                        return;
                                }

                                m_ptr->target = -1;
                        }
			break;
		}
                /* Give target to all */
                case 8:
		{
                        monster_type *m_ptr;
                        int ii, jj, i;


                        msg_print("Select the target monster:");
                        if (!tgt_pt(&ii,&jj)) return;

                        if (cave[jj][ii].m_idx)
                        {
                                msg_print("Target selected");

                                for (i = m_max - 1; i >= 1; i--)
                                {
                                        /* Access the monster */
                                        m_ptr = &m_list[i];

                                        if (!m_ptr->r_idx) continue;

                                        if (m_ptr->status < MSTATUS_PET) continue;

                                        m_ptr->target = cave[jj][ii].m_idx;
                                }
                        }
                        else
                        {
                                msg_print("This is not a correct target.");
                                return;
                        }
			break;
		}
		case 1: /* Dismiss pets */
		{
			int Dismissed = 0;

			if (get_check("Dismiss all pets? ")) all_pets = TRUE;

			/* Process the monsters (backwards) */
			for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
			{
                                monster_race *r_ptr;

				/* Access the monster */
				m_ptr = &m_list[pet_ctr];
                                r_ptr = &r_info[m_ptr->r_idx];

                                if ((!(r_ptr->flags7 & RF7_NO_DEATH)) && ((m_ptr->status == MSTATUS_PET) || (m_ptr->status == MSTATUS_FRIEND))) /* Get rid of it! */
				{
					bool delete_this = FALSE;

					if (all_pets)
						delete_this = TRUE;
					else
					{
						char friend_name[80], check_friend[80];
						monster_desc(friend_name, m_ptr, 0x80);
						sprintf(check_friend, "Dismiss %s? ", friend_name);

						if (get_check(check_friend))
							delete_this = TRUE;
					}

					if (delete_this)
					{
						delete_monster_idx(pet_ctr);
						Dismissed++;
					}
				}
			}

			msg_format("You have dismissed %d pet%s.", Dismissed,
				(Dismissed == 1 ? "" : "s"));
			break;
		}
		/* Call pets */
		case 2:
		{
			p_ptr->pet_follow_distance = 1;
			break;
		}
                /* "Seek and destroy" */
		case 3:
		{
			p_ptr->pet_follow_distance = 255;
			break;
		}
		/* flag - allow pets to open doors */
		case 4:
		{
			p_ptr->pet_open_doors = !p_ptr->pet_open_doors;
			break;
		}
		/* flag - allow pets to pickup items */
		case 5:
		{
			p_ptr->pet_pickup_items = !p_ptr->pet_pickup_items;

			/* Drop objects being carried by pets */
			if (!p_ptr->pet_pickup_items)
			{
				for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
				{
					/* Access the monster */
					m_ptr = &m_list[pet_ctr];

                                        if (m_ptr->status >= MSTATUS_PET)
					{
						monster_drop_carried_objects(m_ptr);
					}
				}
			}

			break;
		}
		/* "Follow Me" */
		case 6:
		{
			p_ptr->pet_follow_distance = 6;
			break;
		}
	}
}

/* Incarnate into a body */
bool do_cmd_integrate_body()
{
        cptr q, s;
        int item;
        object_type *o_ptr;

        if(!p_ptr->disembodied)
        {
                msg_print("You are already in a body");
                return FALSE;
        }

        /* Restrict choices to monsters */
        item_tester_tval = TV_CORPSE;

        /* Get an item */
        q = "Incarnate in which body? ";
        s = "You have no corpse to incarnate in.";
        if (!get_item(&item, q, s, (USE_FLOOR))) return FALSE;

        o_ptr = &o_list[0 - item];

        if(o_ptr->sval != SV_CORPSE_CORPSE)
        {
                msg_print("You must select a corpse");
                return FALSE;
        }

        p_ptr->body_monster = o_ptr->pval2;
        p_ptr->chp = o_ptr->pval3;

        floor_item_increase(0 - item, -1);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);

        msg_print("Your spirit is incarnated in your new body.");
        p_ptr->wraith_form = FALSE;
        p_ptr->disembodied = FALSE;
	do_cmd_redraw();

        return TRUE;
}

/* Leave a body */
void do_cmd_leave_body(bool drop_body)
{
        object_type *o_ptr, forge;
        monster_race *r_ptr = &r_info[p_ptr->body_monster];
        int i;

        if(p_ptr->disembodied)
        {
                msg_print("You are already disembodied.");
                return;
        }

        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
        {
                if (p_ptr->body_parts[i - INVEN_WIELD] && inventory[i].k_idx &&
                    cursed_p(&inventory[i]))
                {
                        msg_print("A cursed object is preventing you from leaving your body.");
                        return;
                }
        }

        if(drop_body)
        {
                o_ptr = &forge;
                object_prep(o_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_CORPSE));
                o_ptr->number = 1;
                o_ptr->pval = 0;
                o_ptr->pval2 = p_ptr->body_monster;
                o_ptr->pval3 = p_ptr->chp;
                o_ptr->weight = (r_ptr->weight + rand_int(r_ptr->weight) / 10) + 1;
                object_aware(o_ptr);
                object_known(o_ptr);
                o_ptr->ident |= IDENT_STOREB;

                /* Unique corpses are unique */
                if (r_ptr->flags1 & RF1_UNIQUE)
                {
                        o_ptr->name1 = 201;
                }

                drop_near(o_ptr, -1, py, px);
        }

        msg_print("Your spirit leaves your body.");
        p_ptr->disembodied = TRUE;
        p_ptr->body_monster = test_monster_name("Lost soul"); /* Lost soul(just for the picture) */
	do_cmd_redraw();
}

bool execute_inscription(byte i, byte y, byte x)
{
        cave_type *c_ptr = &cave[y][x];

        /* Check is the grid as enough mana to be actived */
        if(c_ptr->mana >= inscription_info[i].mana)
        {
                /* Reduce the grid mana -- note: it can't be restored */
                c_ptr->mana -= inscription_info[i].mana;

                switch(i)
                {
                        case INSCRIP_LIGHT:
                        {
                                msg_print("The inscription shines in a bright light !");
                                lite_room(y, x);
                                break;
                        }
                        case INSCRIP_DARK:
                        {
                                msg_print("The inscription is enveloped in a dark aura!");
                                unlite_room(y, x);
                                break;
                        }
                        case INSCRIP_STORM:
                        {
                                msg_print("The inscription releases a powerful storm !");
                                project(0, 3, y, x, damroll(10, 10), GF_ELEC, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
                                break;
                        }
                        case INSCRIP_PROTECTION:
                        {
                                return FALSE;
                                break;
                        }
                        case INSCRIP_DWARF_SUMMON:
                        {
                                int yy = y, xx = x;

                                scatter(&yy, &xx, y, x, 3, 0);
                                place_monster_one(yy, xx, test_monster_name("Dwarven Warrior"), 0, FALSE, MSTATUS_FRIEND);
                                break;
                        }
                        case INSCRIP_CHASM:
                        {
                monster_type *m_ptr;
                monster_race *r_ptr;
                cave_type *c_ptr;
                int     ii = x, ij = y;

                cave_set_feat(ij, ii, FEAT_DARK_PIT);
                msg_print("A chasm appears in the floor!");

                if(cave[ij][ii].m_idx)
                {
                        m_ptr = &m_list[cave[ij][ii].m_idx];
                        r_ptr = race_inf(m_ptr);

                        if(r_ptr->flags7 & RF7_CAN_FLY)
                        {
                                msg_print("The monster simply flies over the chasm.");
                        }
                        else
                        {
                                if(!(r_ptr->flags1 & RF1_UNIQUE))
                                {
                                        msg_print("The monster fall in the chasm !");
                                        delete_monster_idx(cave[ij][ii].m_idx);
                                }
                        }
                }

                if(cave[ij][ii].o_idx)
                {
                        s16b this_o_idx, next_o_idx = 0;

                        c_ptr = &cave[ij][ii];

                        /* Scan all objects in the grid */
                        for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
                        {
                                object_type *o_ptr;
                                bool plural = FALSE;

                                char o_name[80];

                                /* Acquire object */
                                o_ptr = &o_list[this_o_idx];

                                if(o_ptr->number > 1) plural = TRUE;

                                /* Acquire next object */
                                next_o_idx = o_ptr->next_o_idx;

                                /* Effect "observed" */
                                if (o_ptr->marked)
                                {
                                        object_desc(o_name, o_ptr, FALSE, 0);
                                }

                                /* Artifacts get to resist */
                                if (o_ptr->name1)
                                {
                                        /* Observe the resist */
                                        if (o_ptr->marked)
                                        {
                                                msg_format("The %s %s simply fly over the chasm!",
                                                           o_name, (plural ? "are" : "is"));
                                        }
                                }

                                /* Kill it */
                                else
                                {
                                        /* Delete the object */
                                        delete_object_idx(this_o_idx);

                                        /* Redraw */
                                        lite_spot(ij, ii);
                                }
                        }
               }
                                break;
                        }
                        case INSCRIP_BLACK_FIRE:
                        {
                                msg_print("The inscription releases a blast of hellfire !");
                                project(0, 3, y, x, 200, GF_HELL_FIRE, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
                                break;
                        }
                }
        }
        return TRUE;
}

/* Choose an inscription and engrave it */
void do_cmd_engrave()
{
        char buf[41] = "";
        byte i;

        sprintf(buf, "%s", inscription_info[cave[py][px].inscription].text);

        get_string("Engrave what ? ", buf, 40);

        for(i = 0; i < MAX_INSCRIPTIONS; i++)
        {
                if(!strcmp(inscription_info[i].text, buf))
                {
                        if(inscription_info[i].know)
                        {
                                /* Save the inscription */
                                cave[py][px].inscription = i;
                        }else
                                msg_print("You can't use this inscription for now.");
                }
        }

        /* Execute the inscription */
        if(inscription_info[cave[py][px].inscription].when & INSCRIP_EXEC_ENGRAVE)
        {
                execute_inscription(cave[py][px].inscription, py, px);
        }

        energy_use += 300;
}

/*
 * Let's do a spinning around attack:                   -- DG --
 *     aDb
 *     y@k
 *     ooT
 * Ah ... all of those will get hit.
 */
void do_spin()
{
        int i, j;

        msg_print("You start spinning around...");
        for(j = py - 1; j <= py + 1; j++)
                for(i = px - 1; i <= px + 1; i++)
                {
                        /* Avoid stupid bugs */
                        if(in_bounds(j, i) && cave[j][i].m_idx)
                                py_attack(j, i, 1);
                }
}
